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

#ifndef BASE_Threads_ReaderLock_H
#define BASE_Threads_ReaderLock_H

#include "../type_traits/Utils.h"

#include "LockPolicy.h"

namespace Base::Threads
{

/**
 * @brief Locks a classes of type "MutexHolder" for "reading".
 * The MutexHolder must:
 * 1. Define a MutexHolder::ReaderGate class that implements getStruct().
 * 2. Define a method that takes a ReaderLock as argument,
 *    and returns a ReaderGate instance.
 */
template<typename MutexHolder, auto LocalPointer = nullptr>
class ReaderLock
    : public SharedLock
{
public:
    using local_data_t = const MemberPointerTo_t<LocalPointer>;

    [[nodiscard]]
    ReaderLock(const MutexHolder& mutex_holder)
        : SharedLock(*mutex_holder.getMutexPair())
        , gate(mutex_holder.getReaderGate(this))
        , localData((&*gate)->*LocalPointer)
    {}

    const auto* operator->() const {return &(StripSmartPointer{localData}());}

private:
    typename MutexHolder::ReaderGate gate;
    local_data_t& localData;
};

template<typename MutexHolder>
class ReaderLock<MutexHolder, nullptr>
    : public SharedLock
{
public:
    [[nodiscard]]
    ReaderLock(const MutexHolder& mutex_holder)
        : SharedLock(*mutex_holder.getMutexPair())
        , gate(mutex_holder.getReaderGate(this))
    {}

    auto operator->() const {return &*gate;}

private:
    typename MutexHolder::ReaderGate gate;
};

} //namespace Base::Threads

#endif // BASE_Threads_ReaderLock_H
