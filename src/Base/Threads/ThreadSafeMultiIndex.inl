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

#include "IteratorWrapper.h"
#include "ThreadSafeContainer.h"

#include "ThreadSafeMultiIndex.h"

namespace Base::Threads
{

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::begin()
{
    return iterator(ordered_data.begin());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::begin() const
{
    return iterator(ordered_data.begin());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::cbegin() const
{
    return iterator(ordered_data.cbegin());
}


template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::end()
{
    return iterator(ordered_data.end());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::end() const
{
    return iterator(ordered_data.end());
}

template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::cend() const
{
    return iterator(ordered_data.cend());
}



template<typename Element, auto ...LocalPointers>
bool MultiIndexContainer<Element, LocalPointers...>::empty() const
{
    return ordered_data.empty();
}

template<typename Element, auto ...LocalPointers>
template<typename Key>
auto MultiIndexContainer<Element, LocalPointers...>::equal_range(const Key& key) const
{
    return equal_range<index_from_raw_v<Key>>(key);
}

template<typename Element, auto ...LocalPointers>
template<std::size_t I, typename Key>
auto MultiIndexContainer<Element, LocalPointers...>::equal_range(const Key& key) const
{
    const_iterator end(ordered_data.end());

    auto& map = std::template get<I>(indexes);
    auto it_ptr = map.find(ReduceToRaw<Key>::reduce(key));
    if(it_ptr == map.end())
    {
        return std::make_pair(end, std::move(end));
    }

    auto it_id = ordered_data_reverse.find(it_ptr->second);
    assert(it_id != ordered_data_reverse.end());
    if(it_id == ordered_data_reverse.end())
    {
        return std::make_pair(end, std::move(end));
    }

    auto it_final = ordered_data.find(it_id->second);
    assert(it_final != ordered_data.end());

    return std::make_pair(const_iterator{std::move(it_final)}, std::move(end));
}


template<typename Element, auto ...LocalPointers>
template<typename Key>
auto MultiIndexContainer<Element, LocalPointers...>::find(const Key& key)
{
    return EndAwareIterator(equal_range(key));
}

template<typename Element, auto ...LocalPointers>
template<typename Key>
auto MultiIndexContainer<Element, LocalPointers...>::find(const Key& key) const
{
    return EndAwareIterator(equal_range(key));
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
    auto unique_ptr = std::make_unique<Element>(std::forward<Vn>(vn)...);
    Element* inserted_element = unique_ptr.get();
    auto res1 = data.emplace(unique_ptr.get(), std::move(unique_ptr));
    assert(res1.second);

    double count = ++counter;
    auto [it, success] = ordered_data.emplace(count, inserted_element);
    assert(success);

    assert(ordered_data.size() == data.size());
    ordered_data_reverse.emplace(inserted_element, count);
    assert(ordered_data_reverse.size() == data.size());

    insertIndexes(*inserted_element, std::make_index_sequence<sizeof...(LocalPointers)>{});
    return std::pair(iterator(std::move(it)), true);
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

    eraseIndexes(element, std::make_index_sequence<sizeof...(LocalPointers)>{});

    double key = pos->second;
    ordered_data_reverse.erase(&element);
    assert(ordered_data_reverse.size() == data.size() - 1);
    ordered_data.erase(key);
    assert(ordered_data.size() == data.size() - 1);
    data.erase(&element);
    assert(data.size() == ordered_data.size());
    return 1;
}

template<typename Element, auto ...LocalPointers>
template<typename ItType>
auto MultiIndexContainer<Element, LocalPointers...>::erase(ItType it)
{
    erase(*it);
    return ++it; // Attention: assuming it != end!
}


template<typename Element, auto ...LocalPointers>
auto MultiIndexContainer<Element, LocalPointers...>::move_back(const Element& element)
{
    double old_count = ordered_data_reverse.at(&element);
    double new_count = ++counter;
    ordered_data_reverse.at(&element) = new_count;

    auto nh = ordered_data.extract(old_count);
    nh.key() = new_count;
    ordered_data.insert(std::move(nh));
    return new_count;
}


template<typename Element, auto ...LocalPointers>
template<typename ItType>
auto MultiIndexContainer<Element, LocalPointers...>::move_back(const ItType& it)
{
    return move_back(*it);
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
    (void)_;
}

template<typename Element, auto ...LocalPointers>
template<std::size_t I>
void MultiIndexContainer<Element, LocalPointers...>::insertIndex(const element_t& element)
{
    auto& map = std::template get<I>(indexes);
    auto& value = element.*(ElementInfo::template pointer_v<I>);
    auto raw_value = ReduceToRaw<decltype(value)>::reduce(value);
    map.emplace(raw_value, &element);
    assert(map.size() == data.size());
}

template<typename Element, auto ...LocalPointers>
template<std::size_t ...In>
void MultiIndexContainer<Element, LocalPointers...>::eraseIndexes(const element_t& element, const std::index_sequence<In...>&)
{
    // If you know a better way... please, tell me! :-)
    int _[] = {(eraseIndex<In>(element),0)...};
    (void)_;
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
