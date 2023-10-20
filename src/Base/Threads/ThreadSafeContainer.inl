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

#include <mutex>

#include "LockPolicy.h"
#include "LockedIterator.h"
#include "ThreadSafeContainer.h"

namespace Base::Threads {

/*
 * Iterator: begin().
 */
template<typename ContainerType>
typename ThreadSafeContainer<ContainerType>::iterator
ThreadSafeContainer<ContainerType>::begin()
{
    return iterator(mutex, container.begin(), container.end());
}

template<typename ContainerType>
typename ThreadSafeContainer<ContainerType>::const_iterator
ThreadSafeContainer<ContainerType>::begin() const
{
    return cbegin();
}

template<typename ContainerType>
typename ThreadSafeContainer<ContainerType>::const_iterator
ThreadSafeContainer<ContainerType>::cbegin() const
{
    return const_iterator(mutex, container.cbegin(), container.cend());
}


/*
 * Iterator: end().
 */
template<typename ContainerType>
typename ThreadSafeContainer<ContainerType>::iterator
ThreadSafeContainer<ContainerType>::end()
{
    return iterator::MakeEndIterator(container.end());
}

template<typename ContainerType>
typename ThreadSafeContainer<ContainerType>::const_iterator
ThreadSafeContainer<ContainerType>::end() const
{
    return cend();
}

template<typename ContainerType>
typename ThreadSafeContainer<ContainerType>::const_iterator
ThreadSafeContainer<ContainerType>::cend() const
{
    return const_iterator::MakeEndIterator(container.cend());
}

template<typename ContainerType>
size_t ThreadSafeContainer<ContainerType>::size() const
{
    SharedLock lock(mutex);
    return container.size();
}

template<typename ContainerType>
bool ThreadSafeContainer<ContainerType>::empty() const
{
    SharedLock lock(mutex);
    return container.empty();
}


template<typename ContainerType>
void ThreadSafeContainer<ContainerType>::clear()
{
    ExclusiveLock lock(mutex);
    container.clear();
}

} // namespace
