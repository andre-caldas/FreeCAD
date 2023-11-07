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

#include "MultiIndexContainer.h"

#include "ThreadSafeContainer.h"

namespace Base::Threads
{

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
    using iterator = typename parent_t::iterator;
    using const_iterator = typename parent_t::const_iterator;

    using element_t = Record;

    using parent_t::ThreadSafeContainer;

    template<typename Key>
    auto find(const Key& key)
    {return iterator(mutex, container.find(key));}

    template<typename Key>
    auto find(const Key& key) const
    {return const_iterator(mutex, container.find(key));}

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains<>(key);}

    template<typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains<>(key);}

    struct WriterGate
        : parent_t::WriterGate
    {
        WriterGate(self_t* self) : parent_t::WriterGate(self), self(self) {}
        self_t* self;

        template<typename ...Vn>
        auto emplace(Vn&& ...vn)
        {return self->container.template emplace<>(std::forward<Vn>(vn)...);}

        auto erase(const element_t& element)
        {return self->container.template erase<>(element);}

        template<typename ItType>
        auto erase(ItType& iterator)
        {return self->container.template erase<>(iterator);}

        auto extract(const element_t& element)
        {return self->container.template extract<>(element);}

        template<typename ItType>
        auto extract(ItType& iterator)
        {return self->container.template extract<>(iterator);}

        auto move_back(const element_t& element)
        {return self->container.template move_back<>(element);}

        template<typename ItType>
        auto move_back(const ItType& iterator)
        {return self->container.template move_back<>(iterator);}
    };
    WriterGate getWriterGate(const ExclusiveLockBase*)
    {assert(LockPolicy::isLockedExclusively(mutex));return WriterGate{this};}

protected:
    using parent_t::mutex;
    using parent_t::container;
};

} //namespace ::Threads

#endif // BASE_Threads_ThreadSafeMultiIndex_H
