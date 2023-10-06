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
    : public ThreadSafeContainer<std::map<Key,Val>>
{
public:
    auto find(const Key& key)
    {return LockedIterator(mutex, container.find(key));}

    auto find(const Key& key) const
    {return LockedIterator(mutex, container.find(key));}

private:
    using ThreadSafeContainer<std::map<Key,Val>>::mutex;
    using ThreadSafeContainer<std::map<Key,Val>>::container;
};

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
    : public ThreadSafeContainer<std::unordered_map<Key,Val>>
{
public:
    auto find(const Key& key)
    {return LockedIterator(mutex, container.find(key));}

    auto find(const Key& key) const
    {return LockedIterator(mutex, container.find(key));}

private:
    using ThreadSafeContainer<std::unordered_map<Key,Val>>::mutex;
    using ThreadSafeContainer<std::unordered_map<Key,Val>>::container;
    friend class ExclusiveLock;
};

} //namespace Base::Threads

#endif // BASE_Threads_ThreadSafeMap_H
