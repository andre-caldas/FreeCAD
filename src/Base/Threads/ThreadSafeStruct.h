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

#ifndef BASE_Threads_ThreadSafeStruct_H
#define BASE_Threads_ThreadSafeStruct_H

#include <thread>

#include "Exception.h"
#include "locks/ReaderLock.h"
#include "locks/WriterLock.h"

namespace Base::Threads
{

/**
 * @brief Encapsulates a struct/class to use SharedLock and ExclusiveLock.
 */
template<typename Struct>
class ThreadSafeStruct
{
public:
    using self_t = ThreadSafeStruct;
    using element_t = Struct;

    using r_lock_t = ReaderLock<ThreadSafeStruct<Struct>>;
    template<auto LocalPointer>
    using r_lock_local_pointer_t = ReaderLock<ThreadSafeStruct<Struct>, LocalPointer>;

    using w_lock_t = WriterLock<ThreadSafeStruct<Struct>>;
    template<auto LocalPointer>
    using w_lock_local_pointer_t = WriterLock<ThreadSafeStruct<Struct>, LocalPointer>;

    ThreadSafeStruct() = default;

    template<typename MutexHolder>
    ThreadSafeStruct(MutexHolder& holder) : mutex(holder.getMutexPair()) {}

    auto lockForReading() const
    {return r_lock_t{*this};}

    auto continueReading()
    {
        w_lock_t lock{*this, true};
        lock.release();
        lock.resumeReading();
        return lock;
    }

    template<auto LocalPointer>
    auto lockPointerForReading() const
    {return r_lock_local_pointer_t<LocalPointer>{*this};}

    auto startWriting()
    {return w_lock_t{*this};}

    auto continueWriting()
    {return w_lock_t{*this, true};}

    template<auto LocalPointer>
    auto lockPointerForWriting()
    {w_lock_local_pointer_t<LocalPointer>{*this};}

    struct ReaderGate
    {
        ReaderGate(const self_t* self) : self(self) {}
        const self_t* self;

        const element_t& operator*() const {return self->theStruct;}
        const element_t* operator->() const {return &self->theStruct;}
    };
    ReaderGate getReaderGate(const SharedLock*) const
    {assert(LockPolicy::isLocked(mutex)); return ReaderGate{this};}

    struct WriterGate
    {
        WriterGate(self_t* self) : self(self) {}
        self_t* self;

        element_t& operator*() const {return self->theStruct;}
        element_t* operator->() const {return &self->theStruct;}
    };
    WriterGate getWriterGate(const ExclusiveLockBase*)
    {assert(LockPolicy::isLockedExclusively(mutex)); return WriterGate{this};}

    void cancelThreads()
    {activeThread = std::thread::id{};}

public:
    // TODO: eliminate this or the gate version.
    auto getMutexPair() const {return &mutex;}

private:
    mutable MutexPair mutex;
    element_t theStruct;

    std::thread::id activeThread;
    friend class WriterLock<ThreadSafeStruct<Struct>>;
};

} //namespace ::Threads

#endif // BASE_Threads_ThreadSafeStruct_H
