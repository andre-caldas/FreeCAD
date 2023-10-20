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

#include <type_traits>
#include <shared_mutex>

#include "LockPolicy.h"

namespace Base::Threads
{

template<typename ItType>
class LockedIterator;

template<typename ContainerType>
class ThreadSafeContainer
{
public:
    using self_t = ThreadSafeContainer;
    typedef ContainerType unsafe_container_t;
    typedef typename unsafe_container_t::iterator container_iterator;
    typedef typename unsafe_container_t::const_iterator container_const_iterator;

    typedef LockedIterator<container_iterator> iterator;
    typedef LockedIterator<container_const_iterator> const_iterator;

    typedef std::shared_mutex mutex_type;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    size_t size() const;
    bool empty() const;
    void clear();

    struct ModifierGate
    {
        ModifierGate(self_t* self) : self(self) {}
        self_t* self;
        auto getMutexPtr() const {return &self->mutex;}
        void clear();
    };
    ModifierGate getModifierGate(const ExclusiveLockBase*)
    {assert(LockPolicy::isLocked(mutex));return ModifierGate{this};}

protected:
    mutable mutex_type mutex;
    ContainerType container;

public: // :-(
    mutex_type* getMutexPtr() const {return &mutex;}
};

} //namespace Base::Threads

#include "ThreadSafeContainer.inl"

#endif // BASE_Threads_ThreadSafeContainer_H
