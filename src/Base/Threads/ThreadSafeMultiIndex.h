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

#include "IteratorWrapper.h"
#include "ThreadSafeContainer.h"

namespace Base::Threads
{

template<typename Element, auto ...LocalPointers>
class MultiIndexContainer
{
public:
    using iterator = IteratorSecondPtrAsRef<typename std::map<double, Element*>::iterator>;
    using const_iterator = IteratorSecondPtrAsRef<typename std::map<double, Element*>::const_iterator>;

    auto begin();
    auto begin() const;
    auto cbegin() const;

    auto end();
    auto end() const;
    auto cend() const;

    bool empty() const;

    template<typename Key>
    auto equal_range(const Key& key) const;

    template<std::size_t I, typename Key>
    auto equal_range(const Key& key) const;

    template<typename Key>
    auto find(const Key& key);

    template<typename Key>
    auto find(const Key& key) const;

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const;

    template<typename Key>
    bool contains(const Key& key) const;

    template<typename ...Vn>
    auto emplace(Vn&& ...vn);

    auto erase(const Element& element);

    template<typename ItType>
    auto erase(ItType it);

    auto move_back(const Element& element);

    template<typename ItType>
    auto move_back(const ItType& it);


    using ElementInfo = MultiIndexElementInfo<Element, LocalPointers...>;

    using element_t = Element;
    template<typename X>
    static constexpr auto index_from_raw_v = ElementInfo::template index_from_raw_v<X>;
    template<std::size_t I>
    using raw_from_index_t = typename ElementInfo::template raw_from_index_t<I>;
    template<std::size_t I>
    using type_from_index_t = typename ElementInfo::template type_from_index_t<I>;

private:
    std::unordered_map<const element_t*, std::unique_ptr<element_t>> data;

    /// @brief Incremented on every insertion.
    std::atomic_int counter;

    /**
     * @brief Items oredered (in principle) by insetion order.
     * @attention We use double instead of an integer so it becomes easy
     * to insert an element between two existing elements.
     */
    std::map<double, element_t*> ordered_data;

    /**
     * @brief Reverse of ordered_data.
     * It is used to get the key for "ordered_data".
     */
    std::map<const element_t*, double> ordered_data_reverse;


    /**
     * @brief The Element struct show us what indexes can be used to search the list.
     * Each tuple entry is an std::map<raw_key, Element&>.
     */
    typename ElementInfo::indexes_tuple_t indexes;

    template<std::size_t ...In>
    void insertIndexes(const element_t& element, const std::index_sequence<In...>&);

    template<std::size_t I>
    void insertIndex(const element_t& element);


    template<std::size_t ...In>
    void eraseIndexes(const element_t& element, const std::index_sequence<In...>&);

    template<std::size_t I>
    void eraseIndex(const element_t& element);
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
    using self_t = ThreadSafeMultiIndex;
    using parent_t = ThreadSafeContainer<MultiIndexContainer<Record, LocalPointers...>>;

    using element_t = Record;

    template<typename Key>
    auto find(const Key& key)
    {return LockedIterator(mutex, container.equal_range(key));}

    template<typename Key>
    auto find(const Key& key) const
    {return LockedIterator(mutex, container.equal_range(key));}

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains(key);}

    template<typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains(key);}

    struct ModifierGate
        : parent_t::ModifierGate
    {
        ModifierGate(self_t* self) : parent_t::ModifierGate(self), self(self) {}
        self_t* self;

        template<typename ...Vn>
        auto emplace(Vn&& ...vn)
        {return self->container.template emplace(std::forward<Vn>(vn)...);}

        auto erase(const element_t& element)
        {return self->container.template erase(element);}

        template<typename ItType>
        auto erase(ItType& iterator)
        {return self->container.template erase(iterator.getIterator());}

        auto move_back(const element_t& element)
        {return self->container.template move_back(element);}

        template<typename ItType>
        auto move_back(const ItType& iterator)
        {return self->container.template move_back(iterator.getIterator());}
    };
    ModifierGate getModifierGate(const ExclusiveLockBase*)
    {assert(LockPolicy::isLockedExclusively(mutex));return ModifierGate{this};}

protected:
    using parent_t::mutex;
    using parent_t::container;
};

} //namespace ::Threads

#include "ThreadSafeMultiIndex.inl"

#endif // BASE_Threads_ThreadSafeMultiIndex_H
