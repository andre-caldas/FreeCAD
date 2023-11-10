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

#include <memory>

#include "../Exception.h"
#include "ReaderLock.h"

namespace Base::Threads
{

template<typename MutexHolder>
ReaderLock<MutexHolder>::ReaderLock(const MutexHolder& mutex_holder)
    : mutexPair(*mutex_holder.getMutexPair())
    , sharedLock(std::make_unique<SharedLock>(mutexPair))
    , gate(mutex_holder.getReaderGate(sharedLock.get()))
{}


template<typename MutexHolder>
void ReaderLock<MutexHolder, nullptr>::release()
{
    assert(sharedLock);
    sharedLock.reset();
}

template<typename MutexHolder>
void ReaderLock<MutexHolder>::resume()
{
    assert(!sharedLock);
    sharedLock = std::make_unique<SharedLock>(mutexPair);
}

template<typename MutexHolder>
auto ReaderLock<MutexHolder>::operator->() const
{
    assert(sharedLock);
    if(!sharedLock) {
        throw ExceptionNeedLock{};
    }
    return &*gate;
}

} //namespace Base::Threads
