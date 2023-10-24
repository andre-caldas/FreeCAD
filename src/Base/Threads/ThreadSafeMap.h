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

#ifndef BASE_Threads_ThreadSafeMap_H
#define BASE_Threads_ThreadSafeMap_H

#include <map>
#include <unordered_map>

#include "ThreadSafeContainer.h"

namespace Base::Threads
{

/**
 * @brief This wraps an std::map or std::unordered_map and provides a shared_mutex
 * to make the map readable by many only when the map structure
 * is not being modified.
 * The map elements are not protected, just the map structure.
 * To protect each element, use std::atomic.
 *
 * Iterators use a shared_lock.
 * Methods that change the map structure use a lock.
 */
template<typename MapType, typename Key, typename Val>
class ThreadSafeMapCommon
    : public ThreadSafeContainer<MapType>
{
public:
    using self_t = ThreadSafeMapCommon;
    using parent_t = ThreadSafeContainer<MapType>;
    using typename parent_t::iterator;
    using typename parent_t::const_iterator;

    auto find(const Key& key)
    {return LockedIterator(mutex, container.find(key), container.end());}

    auto find(const Key& key) const
    {return LockedIterator(mutex, container.find(key), container.end());}

    size_t count(const Key& key) const
    {SharedLock l(mutex); return container.count(key);}

    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.count(key);}

    auto& at(const Key& key)
    {SharedLock l(mutex); return container.at(key);}

    auto& at(const Key& key) const
    {SharedLock l(mutex); return container.at(key);}

    struct ModifierGate
        : parent_t::ModifierGate
    {
        ModifierGate(self_t* self) : parent_t::ModifierGate(self), self(self) {}
        self_t* self;

        auto erase(const Key& key)
        {return self->container.erase(key);}

        auto erase(const iterator& it)
        {return self->container.erase(it.getIterator());}

        auto erase(const const_iterator& it)
        {return self->container.erase(it.getIterator());}

        auto& operator[](const Key& key)
        {return self->container[key];}
    };
    ModifierGate getModifierGate(const ExclusiveLockBase*)
    {assert(LockPolicy::isLockedExclusively(mutex));return ModifierGate{this};}

protected:
    using parent_t::mutex;
    using parent_t::container;
};

/**
 * @brief This wraps an std::map and provides a shared_mutex
 * to make the map readable by many only when the map structure
 * is not being modified.
 * The map elements are not protected, just the map structure.
 * To protect each element, use std::atomic.
 *
 * Iterators use a shared_lock.
 * Methods that change the map structure use a lock.
 */
template<typename Key, typename Val>
class ThreadSafeMap
    : public ThreadSafeMapCommon<std::map<Key,Val>, Key, Val>
{};

/**
 * @brief This wraps an std::map and provides a shared_mutex
 * to make the map readable by many only when the map structure
 * is not being modified.
 * The map elements are not protected, just the map structure.
 * To protect each element, use std::atomic.
 *
 * Iterators use SharedLock.
 * Methods that change the map structure use a lock.
 */
template<typename Key, typename Val>
class ThreadSafeUnorderedMap
    : public ThreadSafeMapCommon<std::unordered_map<Key,Val>, Key, Val>
{};

} //namespace Base::Threads

#endif // BASE_Threads_ThreadSafeMap_H
