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

#ifndef BASE_Threads_ThreadSafeContainer_H
#define BASE_Threads_ThreadSafeContainer_H

#include <shared_mutex>

#include "LockedIterator.h"

namespace Base::Threads
{

class UniqueLock;

template<typename ContainerType>
class ThreadSafeContainer
{
public:
    using container_type = ContainerType;
    using container_iterator = typename container_type::iterator;
    using container_const_iterator = typename container_type::const_iterator;

    using iterator = LockedIterator<container_iterator>;
    using const_iterator = LockedIterator<container_const_iterator>;

    iterator begin();
    container_iterator end();
    const_iterator cbegin() const;
    container_const_iterator cend() const;

    void clear();
    size_t size() const;

protected:
    friend class UniqueLock;
    mutable std::shared_mutex mutex;
    container_type container;
};

} //namespace Base::Threads

#include "ThreadSafeContainer.inl"

#endif // BASE_Threads_ThreadSafeContainer_H
