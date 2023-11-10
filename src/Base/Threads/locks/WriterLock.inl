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
#include "WriterLock.h"

namespace Base::Threads
{

template<typename MutexHolder>
WriterLock<MutexHolder>::WriterLock(MutexHolder& mutex_holder, bool try_to_resume)
    : exclusiveLock(std::make_unique<ExclusiveLock<MutexHolder>>(mutex_holder))
    , mutexHolder(mutex_holder)
    , gate(mutex_holder.getWriterGate(exclusiveLock.get()))
{
    assert(!sharedLock && exclusiveLock);

    if(try_to_resume) {
        if(isThreadObsolete()) {
            release();
        }
    }
    else {
        markStart();
    }
}

template<typename MutexHolder>
WriterLock<MutexHolder>::WriterLock(WriterLock<MutexHolder>&& other)
    : exclusiveLock(std::move(other.exclusiveLock))
    , sharedLock(std::move(other.sharedLock))
    , mutexHolder(other.mutexHolder)
    , gate(other.gate)
{}


template<typename MutexHolder>
bool WriterLock<MutexHolder>::isThreadObsolete() const
{
    return mutexHolder.activeThread != std::this_thread::get_id();
}

template<typename MutexHolder>
void WriterLock<MutexHolder, nullptr>::release()
{
    assert(!sharedLock || !exclusiveLock);
    assert(sharedLock || exclusiveLock);
    exclusiveLock.reset();
    sharedLock.reset();
}

template<typename MutexHolder>
bool WriterLock<MutexHolder>::resume()
{
    assert(!sharedLock && !exclusiveLock);
    exclusiveLock = std::make_unique<ExclusiveLock<MutexHolder>>(mutexHolder);
    if(isThreadObsolete())
    {
        release();
        return false;
    }
    return true;
}

template<typename MutexHolder>
bool WriterLock<MutexHolder>::resumeReading()
{
    assert(!sharedLock && !exclusiveLock);
    sharedLock = std::make_unique<SharedLock>(*mutexHolder.getMutexPair());
    if(isThreadObsolete())
    {
        release();
        return false;
    }
    return true;
}

template<typename MutexHolder>
auto WriterLock<MutexHolder>::operator->() const
{
    assert(!exclusiveLock || !sharedLock);
    assert(exclusiveLock || sharedLock);
    if(!exclusiveLock && ! sharedLock)
    {
        throw ExceptionNeedLock{};
    }
    return &*gate;
}

template<typename MutexHolder>
WriterLock<MutexHolder>::operator bool() const
{
    return !isThreadObsolete();
}

template<typename MutexHolder>
void WriterLock<MutexHolder>::registerNewThread()
{
    if(!LockPolicy::isLockedExclusively(mutexHolder.getMutexPair()))
    {
        assert(false);
        throw ExceptionNewThreadRequiresLock{};
    }
    mutexHolder.activeThread = std::this_thread::get_id();
}

template<typename MutexHolder>
void WriterLock<MutexHolder>::markStart()
{
    ExclusiveLock l{mutexHolder};
    mutexHolder.activeThread = std::this_thread::get_id();
}

} //namespace Base::Threads
