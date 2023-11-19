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
{
    // Makes sure it is locked.
    assert(exclusiveLock);
    // Makes sure it was moved from thread: moveFromThread().
    assert(mutexHolder.activeThread == std::thread::id{});
}


template<typename MutexHolder>
bool WriterLock<MutexHolder>::isThreadObsolete() const
{
    return mutexHolder.activeThread != std::this_thread::get_id();
}

template<typename MutexHolder>
void WriterLock<MutexHolder, nullptr>::release()
{
    if(!sharedLock && !exclusiveLock)
    {
        throw ExceptionCannotReleaseUnlocked{};
    }
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
auto& WriterLock<MutexHolder>::operator->() const
{
    assert(!exclusiveLock || !sharedLock);
    assert(exclusiveLock || sharedLock);
    if(!exclusiveLock && ! sharedLock)
    {
        throw ExceptionNeedLock{};
    }
    // TODO: somehow, use reader_gate for sharedLock.
    return gate;
}

template<typename MutexHolder>
WriterLock<MutexHolder>::operator bool() const
{
    return !isThreadObsolete();
}

template<typename MutexHolder>
void WriterLock<MutexHolder>::markStart()
{
    ExclusiveLockFreeLock l{mutexHolder};
    mutexHolder.activeThread = std::this_thread::get_id();
}

template<typename MutexHolder>
void WriterLock<MutexHolder>::moveFromThread()
{
    if(!exclusiveLock)
    {
        assert(false);
        throw ExceptionNewThreadRequiresLock{};
    }
    assert(!sharedLock);

    // Prevent this thread from hijacking back the lock.
    // Makes bool(lock) evaluate to false;
    mutexHolder.activeThread = std::thread::id{};
    exclusiveLock->detachFromThread();
}

template<typename MutexHolder>
void WriterLock<MutexHolder>::resumeInNewThread()
{
    if(!exclusiveLock)
    {
        assert(false);
        throw ExceptionNewThreadRequiresLock{};
    }

    if(std::thread::id{} != mutexHolder.activeThread)
    {
        assert(false);
        throw ExceptionNewThreadRequiresMovedLock{};
    }

    // Prevent this thread from hijacking (continueWriting) it back.
    mutexHolder.activeThread = std::this_thread::get_id();
    exclusiveLock->attachToThread(true);
}

template<typename MutexHolder>
void WriterLock<MutexHolder>::releaseInNewThread()
{
    resumeInNewThread();
    release();
}

} //namespace Base::Threads
