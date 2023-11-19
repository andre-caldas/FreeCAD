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

#include "type_traits/MultiIndexRecordInfo.h"

#include "IteratorWrapper.h"
#include "ThreadSafeContainer.h"

#include "ThreadSafeMultiIndex.h"
#include <iostream>

namespace Base::Threads
{

template<typename Record, auto ...LocalPointers>
template<std::size_t ...In>
bool MultiIndexContainer<Record, LocalPointers...>::axert(const std::index_sequence<In...>&) const
{
    auto size = ordered_data.size();
    assert(data.size() == size);
    assert(ordered_data_reverse.size() == size);
    // If you know a better way... please, tell me! :-)
    int _[] = {(assert(std::template get<In>(indexes).size() == size),0)...};
    (void)_;
    return true;
}

template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::begin()
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return iterator(ordered_data.begin());
}

template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::begin() const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return cbegin();
}

template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::cbegin() const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return const_iterator(ordered_data.cbegin());
}


template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::end()
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return iterator(ordered_data.end());
}

template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::end() const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return cend();
}

template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::cend() const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return const_iterator(ordered_data.cend());
}



template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::size() const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return ordered_data.size();
}

template<typename Record, auto ...LocalPointers>
bool MultiIndexContainer<Record, LocalPointers...>::empty() const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return ordered_data.empty();
}

template<typename Record, auto ...LocalPointers>
void MultiIndexContainer<Record, LocalPointers...>::clear()
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    ordered_data_reverse.clear();
    ordered_data.clear();
    data.clear();
    clearIndexes(std::make_index_sequence<sizeof...(LocalPointers)>{});
}

template<typename Record, auto ...LocalPointers>
template<typename Key>
auto MultiIndexContainer<Record, LocalPointers...>::find(const Key& key)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return find<index_from_raw_v<Key>>(key);
}

template<typename Record, auto ...LocalPointers>
template<std::size_t I, typename Key>
auto MultiIndexContainer<Record, LocalPointers...>::find(const Key& key)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return _find<iterator, I>(key);
}

template<typename Record, auto ...LocalPointers>
template<typename Key>
auto MultiIndexContainer<Record, LocalPointers...>::find(const Key& key) const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return find<index_from_raw_v<Key>>(key);
}

template<typename Record, auto ...LocalPointers>
template<std::size_t I, typename Key>
auto MultiIndexContainer<Record, LocalPointers...>::find(const Key& key) const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto self = const_cast<self_t*>(this);
    return self->template _find<const_iterator, I>(key);
}

template<typename Record, auto ...LocalPointers>
template<typename ItType, std::size_t I, typename Key>
auto MultiIndexContainer<Record, LocalPointers...>::_find(const Key& key)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto& map = std::template get<I>(indexes);
    auto it_ptr = map.find(ReduceToRaw<Key>::reduce(key));
    if(it_ptr == map.end())
    {
        return ItType(ordered_data.end());
    }

    auto it_id = ordered_data_reverse.find(it_ptr->second);
    assert(it_id != ordered_data_reverse.end());
    if(it_id == ordered_data_reverse.end())
    {
        return ItType{ordered_data.end()};
    }

    auto it_final = ordered_data.find(it_id->second);
    assert(it_final != ordered_data.end());

    return ItType{std::move(it_final)};
}

template<typename Record, auto ...LocalPointers>
template<std::size_t I, typename Key>
bool MultiIndexContainer<Record, LocalPointers...>::contains(const Key& key) const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto it = find<I>(key);
    return it != end();
}

template<typename Record, auto ...LocalPointers>
template<typename Key>
bool MultiIndexContainer<Record, LocalPointers...>::contains(const Key& key) const
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto it = find(key);
    return it != end();
}

template<typename Record, auto ...LocalPointers>
template<typename ...Vn>
auto MultiIndexContainer<Record, LocalPointers...>::emplace_back(Vn&& ...vn)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto unique_ptr = std::make_unique<Record>(std::forward<Vn>(vn)...);
    Record* inserted_record = unique_ptr.get();
    auto res1 = data.emplace(unique_ptr.get(), std::move(unique_ptr));
    assert(res1.second);

    long count = ++counter;
    auto [it, success] = ordered_data.emplace(count, inserted_record);
    assert(success);

    assert(ordered_data.size() == data.size());
    ordered_data_reverse.emplace(inserted_record, count);
    assert(ordered_data_reverse.size() == data.size());

    insertIndexes(*inserted_record, std::make_index_sequence<sizeof...(LocalPointers)>{});
    return std::pair(iterator(std::move(it)), true);
}


template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::erase(const Record& record)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto nh = extract(record);
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return (bool)nh;
}

template<typename Record, auto ...LocalPointers>
template<typename ItType, std::enable_if_t<IsIterator<ItType>::value, bool>>
auto MultiIndexContainer<Record, LocalPointers...>::erase(const ItType& it)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    ItType next = it;
    ++next; // Attention: assuming it != end!
    erase(*it);
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    return next;
}

template<typename Record, auto ...LocalPointers>
template<typename Key, std::enable_if_t<!IsIterator<Key>::value, int>>
auto MultiIndexContainer<Record, LocalPointers...>::erase(const Key& key)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    std::size_t count = 0;
    for(auto it = find(key); it != end(); it = find(key))
    {
        assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

        ++count;
        erase(*it);
        assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    }
    return count;
}


template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::extract(const Record& record)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto pos = ordered_data_reverse.find(&record);
    assert(pos != ordered_data_reverse.end());
    if(pos == ordered_data_reverse.end())
    {
        return typename decltype(data)::node_type{};
    }

    deleteIndexes(record, std::make_index_sequence<sizeof...(LocalPointers)>{});

    long key = pos->second;
    ordered_data_reverse.erase(&record);
    assert(ordered_data_reverse.size() == data.size() - 1);
    ordered_data.erase(key);
    assert(ordered_data.size() == data.size() - 1);
    auto nh = data.extract(&record);
    assert(data.size() == ordered_data.size());
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    return nh;
}

template<typename Record, auto ...LocalPointers>
template<typename ItType, std::enable_if_t<IsIterator<ItType>::value, bool>>
auto MultiIndexContainer<Record, LocalPointers...>::extract(const ItType& it)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return extract(*it);
}

template<typename Record, auto ...LocalPointers>
template<typename Key, std::enable_if_t<!IsIterator<Key>::value, int>>
auto MultiIndexContainer<Record, LocalPointers...>::extract(const Key& key)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    auto it = find(key);
    if(it == end())
    {
        return node_type{};
    }
    return extract(*it);
}


template<typename Record, auto ...LocalPointers>
auto MultiIndexContainer<Record, LocalPointers...>::move_back(const Record& record)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    long old_count = ordered_data_reverse.at(&record);
    long new_count = ++counter;
    ordered_data_reverse.at(&record) = new_count;

    auto nh = ordered_data.extract(old_count);
    nh.key() = new_count;
    ordered_data.insert(std::move(nh));
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));

    return new_count;
}


template<typename Record, auto ...LocalPointers>
template<typename ItType>
auto MultiIndexContainer<Record, LocalPointers...>::move_back(const ItType& it)
{
    assert(axert(std::make_index_sequence<sizeof...(LocalPointers)>{}));
    return move_back(*it);
}



/*
 * Private methods.
 */
template<typename Record, auto ...LocalPointers>
template<std::size_t ...In>
void MultiIndexContainer<Record, LocalPointers...>::insertIndexes(const Record& record, const std::index_sequence<In...>&)
{
    // If you know a better way... please, tell me! :-)
    int _[] = {(insertIndex<In>(record),0)...};
    (void)_;
}

template<typename Record, auto ...LocalPointers>
template<std::size_t I>
void MultiIndexContainer<Record, LocalPointers...>::insertIndex(const Record& record)
{
    auto& map = std::template get<I>(indexes);
    assert(map.size() == data.size() - 1);
    auto& value = record.*(RecordInfo::template pointer_v<I>);
    auto raw_value = ReduceToRaw<decltype(value)>::reduce(value);
    map.emplace(raw_value, &record);
    assert(map.size() == data.size());
}


template<typename Record, auto ...LocalPointers>
template<std::size_t ...In>
void MultiIndexContainer<Record, LocalPointers...>::deleteIndexes(const Record& record, const std::index_sequence<In...>&)
{
    // If you know a better way... please, tell me! :-)
    int _[] = {(deleteIndex<In>(record),0)...};
    (void)_;
}

template<typename Record, auto ...LocalPointers>
template<std::size_t I>
void MultiIndexContainer<Record, LocalPointers...>::deleteIndex(const Record& record)
{
    auto& map = std::template get<I>(indexes);
    auto& value = record.*(RecordInfo::template pointer_v<I>);
    auto raw_value = ReduceToRaw<decltype(value)>::reduce(value);
    auto [begin,end] = map.equal_range(raw_value);
    for(auto it = begin; it != end; ++it)
    {
        if(it->second == &record)
        {
            map.extract(it);
            assert(map.size() == data.size()-1);
            return;
        }
    }
    assert(false);
}


template<typename Record, auto ...LocalPointers>
template<std::size_t ...In>
void MultiIndexContainer<Record, LocalPointers...>::clearIndexes(const std::index_sequence<In...>&)
{
    // If you know a better way... please, tell me! :-)
    int _[] = {(clearIndex<In>(),0)...};
    (void)_;
}

template<typename Record, auto ...LocalPointers>
template<std::size_t I>
void MultiIndexContainer<Record, LocalPointers...>::clearIndex()
{
    auto& map = std::template get<I>(indexes);
    map.clear();
    assert(map.size() == data.size());
}

} //namespace ::Threads
