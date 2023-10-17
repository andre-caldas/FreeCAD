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

#ifndef BASE_Threads_ThreadSafeMultiIndex_H
#define BASE_Threads_ThreadSafeMultiIndex_H

#include "type_traits/record_info.h"

#include "PairSecondIterator.h"
#include "ThreadSafeContainer.h"

namespace Base::Threads
{

template<typename Element, auto ...LocalPointers>
class MultiIndexContainer
{
public:
    using ElementInfo = MultiIndexElementInfo<Element, LocalPointers...>;

    using element_t = Element;
    template<typename X>
    static constexpr auto index_from_raw_v = ElementInfo::template index_from_raw_v<X>;
    template<std::size_t I>
    using raw_from_index_t = typename ElementInfo::template raw_from_index_t<I>;
    template<std::size_t I>
    using type_from_index_t = typename ElementInfo::template type_from_index_t<I>;

    using iterator = PairSecondIterator<typename std::map<double, element_t&>::iterator>;
    using const_iterator = PairSecondIterator<typename std::map<double, element_t&>::const_iterator>;

    auto begin() {return PairSecondIterator(ordered_data.begin());}
    auto begin() const {return PairSecondIterator(ordered_data.begin());}
    auto cbegin() const {return PairSecondIterator(ordered_data.cbegin());}

    auto end() {return PairSecondIterator(ordered_data.end());}
    auto end() const {return PairSecondIterator(ordered_data.end());}
    auto cend() const {return PairSecondIterator(ordered_data.cend());}

    template<typename X>
    auto equal_range(const X& key)
    {
        auto& map = std::template get<index_from_raw_v<X>>(indexes);
        auto it = map.find(ReduceToRaw<X>::reduce(key));
        return std::pair(PairSecondIterator(std::move(it)), PairSecondIterator(map.end()));
    }

    template<std::size_t I, typename X>
    auto equal_range(const X& key) const
    {
        auto& map = std::template get<I>(indexes);
        auto it = map.find(ReduceToRaw<X>::reduce(key));
        return std::pair(PairSecondIterator(std::move(it)), PairSecondIterator(map.end()));
    }

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const
    {auto range = equal_range<I>(key); return range.first != range.second;}

    template<typename Key>
    bool contains(const Key& key) const
    {auto range = equal_range(key); return range.first != range.second;}

    template<typename ...Vn>
    auto emplace(Vn&& ...vn)
    {
        auto emplace_pair = data.emplace(std::forward<Vn>(vn)...);
        assert(emplace_pair.second);
        const Element& inserted_element = *(emplace_pair.first);
        ordered_data.emplace(++counter, inserted_element);
        assert(ordered_data.size() == data.size());

        insertIndexes(inserted_element);
        return emplace_pair;
    }

private:
    // TODO: use lambda in template (C++20).
    struct elementHash
    {
        std::size_t operator()(const element_t& element)
        {return (std::size_t)&element;}
    };

    /**
     * @brief Holds each data set.
     */
    std::unordered_set<element_t, elementHash> data;

    /// @brief Incremented on every insertion.
    std::atomic_int counter;

    /**
     * @brief Items oredered by insetion order.
     * @attention We use double instead of an integer so it becomes easy
     * to insert an element between two existing elements.
     */
    std::map<double, element_t&> ordered_data;

    /**
     * @brief The Element struct show us what indexes can be used to search the list.
     * Each tuple entry is an std::map<raw_key, Element&>.
     */
    typename ElementInfo::indexes_tuple_t indexes;

    void insertIndexes(const element_t& element)
    {
        for(auto i = 0; i < element.tuple_size; ++i)
        {
            insertIndex<i>(element);
        }
    }

    template<std::size_t I>
    void insertIndex(const element_t& element)
    {
        auto& map = std::template get<I>(indexes);
        auto& value = element.*(ElementInfo::template pointer_v<I>);
        auto raw_value = ReduceToRaw<decltype(value)>::reduce(value);
        map.emplace(raw_value, element);
        assert(map.size() == data.size());
    }
};



/**
 * @brief Implements a thread safe container that:
 * 0. Manages a tuple of objects.
 * 1. Keeps the order of insertion.
 * 2. Can look up any object with efficenty.
 *
 * Possible uses:
 * 1. An std::map that keeps track the order of insertion.
 * 2. An std::map that can be searched in both directions.
 */
template<typename Record, auto ...LocalPointers>
class ThreadSafeMultiIndex
    : public ThreadSafeContainer<MultiIndexContainer<Record, LocalPointers...>>
{
public:
    using base_class_t = ThreadSafeContainer<MultiIndexContainer<Record, LocalPointers...>>;

    template<typename X>
    auto find(const X& key)
    {return LockedIterator(mutex, container.equal_range(key));}

    template<typename X>
    auto find(const X& key) const
    {return LockedIterator(mutex, container.equal_range(key));}

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains(key);}

    template<typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains(key);}

    template<typename ...Vn>
    auto emplace(Vn&& ...vn)
    {ExclusiveLock l(mutex); return container.template emplace(vn...);}


private:
    using base_class_t::mutex;
    using base_class_t::container;
};

} //namespace ::Threads

#endif // BASE_Threads_ThreadSafeMultiIndex_H
