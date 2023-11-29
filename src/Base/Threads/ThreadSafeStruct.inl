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

#include "ThreadSafeStruct.h"

namespace Base::Threads
{

template<typename Struct>
template<typename... Args>
ThreadSafeStruct<Struct>::ThreadSafeStruct(Args&&... args)
    : theStruct(args...)
{}

template<typename Struct>
ThreadSafeStruct<Struct>::~ThreadSafeStruct()
{
    /*
     * This is very subtle...
     * Usually, a shared_ptr that holds the structure that holds
     * this ThreadSafeStruct is passed to the dedicatedThread.
     * This means that:
     *
     * 1. We cannot join() on destruction,
     * because we might be joining from the thread we want to join to!
     *
     * 2. We do not need to join, because the all the lambdas have certainly
     * finished processing.
     */
    if (dedicatedThread.joinable()) {
        dedicatedThread.detach();
    }
}

template<typename Struct>
auto ThreadSafeStruct<Struct>::lockForReading() const
{
    return r_lock_t {*this};
}

template<typename Struct>
auto ThreadSafeStruct<Struct>::continueReading()
{
    w_lock_t lock {*this, true};
    lock.release();
    lock.resumeReading();
    return lock;
}

template<typename Struct>
template<auto LocalPointer>
auto ThreadSafeStruct<Struct>::lockPointerForReading() const
{
    return r_lock_local_pointer_t<LocalPointer> {*this};
}

template<typename Struct>
auto ThreadSafeStruct<Struct>::startWriting()
{
    return w_lock_t {*this};
}

template<typename Struct>
auto ThreadSafeStruct<Struct>::continueWriting()
{
    return w_lock_t {*this, true};
}

template<typename Struct>
template<auto LocalPointer>
auto ThreadSafeStruct<Struct>::lockPointerForWriting()
{
    return w_lock_local_pointer_t<LocalPointer> {*this};
}

template<typename Struct>
ThreadSafeStruct<Struct>::ReaderGate::ReaderGate(const self_t* self)
    : self(self)
{}

template<typename Struct>
const Struct& ThreadSafeStruct<Struct>::ReaderGate::operator*() const
{
    return self->theStruct;
}

template<typename Struct>
const Struct* ThreadSafeStruct<Struct>::ReaderGate::operator->() const
{
    return &self->theStruct;
}

template<typename Struct>
const typename ThreadSafeStruct<Struct>::ReaderGate&
ThreadSafeStruct<Struct>::getReaderGate(const SharedLock*) const
{
    assert(LockPolicy::isLocked(mutex));
    return reader_gate;
}

template<typename Struct>
ThreadSafeStruct<Struct>::WriterGate::WriterGate(self_t* self)
    : self(self)
{}


template<typename Struct>
Struct& ThreadSafeStruct<Struct>::WriterGate::operator*() const
{
    return self->theStruct;
}

template<typename Struct>
Struct* ThreadSafeStruct<Struct>::WriterGate::operator->() const
{
    return &self->theStruct;
}

template<typename Struct>
const typename ThreadSafeStruct<Struct>::WriterGate&
ThreadSafeStruct<Struct>::getWriterGate(const ExclusiveLockBase*)
{
    assert(LockPolicy::isLockedExclusively(mutex));
    return writer_gate;
}

template<typename Struct>
void ThreadSafeStruct<Struct>::cancelThreads()
{
    activeThread = std::thread::id {};
}

template<typename Struct>
std::thread& ThreadSafeStruct<Struct>::getDedicatedThread()
{
    return dedicatedThread;
}

template<typename Struct>
auto ThreadSafeStruct<Struct>::getMutexPair() const
{
    return &mutex;
}

}  // namespace Base::Threads
