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

namespace Base::Threads
{

class ExclusiveLock;

template<typename ItType>
class LockedIterator;

template<typename ContainerType>
class ThreadSafeContainer
{
public:
    typedef ContainerType container_type;
    typedef typename container_type::iterator container_iterator;
    typedef typename container_type::const_iterator container_const_iterator;

    typedef LockedIterator<container_iterator> iterator;
    typedef LockedIterator<container_const_iterator> const_iterator;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    container_iterator end() noexcept {return container.end();}
    container_const_iterator end() const {return container.end();}
    container_const_iterator cend() const {return container.cend();}

    void clear();
    size_t size() const;

protected:
    mutable std::shared_mutex mutex;
    container_type container;
    friend class ExclusiveLock;
};

} //namespace Base::Threads

#include "ThreadSafeContainer.inl"

#endif // BASE_Threads_ThreadSafeContainer_H
