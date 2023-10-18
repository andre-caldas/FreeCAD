// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#include "type_traits/record_info.h"

#include "PairSecondIterator.h"
#include "ThreadSafeContainer.h"

#include "ThreadSafeMultiIndex.h"

namespace Base::Threads
{

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::begin()
{
    return PairSecondIterator(ordered_data.begin());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::begin() const
{
    return PairSecondIterator(ordered_data.begin());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::cbegin() const
{
    return PairSecondIterator(ordered_data.cbegin());
}


template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::end()
{
    return PairSecondIterator(ordered_data.end());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::end() const
{
    return PairSecondIterator(ordered_data.end());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::cend() const
{
    return PairSecondIterator(ordered_data.cend());
}



template<typename Element, auto ...LocalPointers>
template<typename X>
auto MultiIndexContainer<Element, LocalPointers...>::equal_range(const X& key)
{
    auto& map = std::template get<index_from_raw_v<X>>(indexes);
    auto it = map.find(ReduceToRaw<X>::reduce(key));
    return std::pair(PairSecondIterator(std::move(it)), PairSecondIterator(map.end()));
}

template<typename Element, auto ...LocalPointers>
template<std::size_t I, typename X>
auto MultiIndexContainer<Element, LocalPointers...>::equal_range(const X& key) const
{
    auto& map = std::template get<I>(indexes);
    auto it = map.find(ReduceToRaw<X>::reduce(key));
    return std::pair(PairSecondIterator(std::move(it)), PairSecondIterator(map.end()));
}

template<typename Element, auto ...LocalPointers>
template<std::size_t I, typename Key>
bool MultiIndexContainer<Element, LocalPointers...>::contains(const Key& key) const
{
    auto range = equal_range<I>(key);
    return range.first != range.second;
}

template<typename Element, auto ...LocalPointers>
template<typename Key>
bool MultiIndexContainer<Element, LocalPointers...>::contains(const Key& key) const
{
    auto range = equal_range(key);
    return range.first != range.second;
}

template<typename Element, auto ...LocalPointers>
template<typename ...Vn>
auto MultiIndexContainer<Element, LocalPointers...>::emplace(Vn&& ...vn)
{
    auto emplace_pair = data.emplace(std::forward<Vn>(vn)...);
    auto [it, success] = emplace_pair;
    assert(success);
    const Element& inserted_element = *it;

    double count = ++counter;
    ordered_data.emplace(count, inserted_element);
    assert(ordered_data.size() == data.size());
    ordered_data_reverse.emplace(&inserted_element, count);
    assert(ordered_data_reverse.size() == data.size());

    insertIndexes(inserted_element, std::make_index_sequence<sizeof...(LocalPointers)>{});
    return emplace_pair;
}


template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::erase(const Element& element)
{
    auto pos = ordered_data_reverse.find(&element);
    assert(pos != ordered_data_reverse.end());
    if(pos == ordered_data_reverse.end())
    {
        return 0;
    }
    double count = pos->second;
    ordered_data.erase(count);
    assert(ordered_data.size() == data.size()-1);
    ordered_data_reverse.erase(&element);
    assert(ordered_data_reverse.size() == data.size()-1);

    eraseIndexes(element, std::make_index_sequence<sizeof...(LocalPointers)>{});

    data.erase(element);
    return 1;
}

template<typename Element, auto ...LocalPointers>
template<typename ItType>
auto MultiIndexContainer<Element, LocalPointers...>::erase(ItType it)
{
    assert(it != this->cend());
    if(it == this->cend())
    {
        return it;
    }
    erase(*it);
    return ++it;
}



/*
 * Private methods.
 */
template<typename Element, auto ...LocalPointers>
template<std::size_t ...In>
void MultiIndexContainer<Element, LocalPointers...>::insertIndexes(const element_t& element, const std::index_sequence<In...>&)
{
    // If you know a better way... please, tell me! :-)
    int _[] = {(insertIndex<In>(element),0)...};
}

template<typename Element, auto ...LocalPointers>
template<std::size_t I>
void MultiIndexContainer<Element, LocalPointers...>::insertIndex(const element_t& element)
{
    auto& map = std::template get<I>(indexes);
    auto& value = element.*(ElementInfo::template pointer_v<I>);
    auto raw_value = ReduceToRaw<decltype(value)>::reduce(value);
    map.emplace(raw_value, element);
    assert(map.size() == data.size());
}

template<typename Element, auto ...LocalPointers>
template<std::size_t ...In>
void MultiIndexContainer<Element, LocalPointers...>::eraseIndexes(const element_t& element, const std::index_sequence<In...>&)
{
    // If you know a better way... please, tell me! :-)
    int _[] = {(eraseIndex<In>(element),0)...};
}

template<typename Element, auto ...LocalPointers>
template<std::size_t I>
void MultiIndexContainer<Element, LocalPointers...>::eraseIndex(const element_t& element)
{
    auto& map = std::template get<I>(indexes);
    auto& value = element.*(ElementInfo::template pointer_v<I>);
    auto raw_value = ReduceToRaw<decltype(value)>::reduce(value);
    map.erase(raw_value);
    assert(map.size() == data.size()-1);
}

} //namespace ::Threads
