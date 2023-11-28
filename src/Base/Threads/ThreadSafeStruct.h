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

    template<typename... Args>
    ThreadSafeStruct(Args&&... args);

    virtual ~ThreadSafeStruct();

    [[maybe_unused]] auto lockForReading() const;

    auto continueReading();

    template<auto LocalPointer>
    auto lockPointerForReading() const;

    auto startWriting();

    auto continueWriting();

    template<auto LocalPointer>
    auto lockPointerForWriting();

    struct ReaderGate
    {
        ReaderGate(const self_t* self);
        const self_t* self;

        const Struct& operator*() const;
        const Struct* operator->() const;
    };
    const ReaderGate& getReaderGate(const SharedLock*) const;

    struct WriterGate
    {
        WriterGate(self_t* self);
        self_t* self;

        Struct& operator*() const;
        Struct* operator->() const;
    };
    const WriterGate& getWriterGate(const ExclusiveLockBase*);

    void cancelThreads();

    std::thread& getDedicatedThread();

public:
    // TODO: eliminate this or the gate version.
    auto getMutexPair() const;

private:
    const ReaderGate reader_gate {this};
    const WriterGate writer_gate {this};

    mutable MutexPair mutex;
    Struct theStruct;

    std::thread::id activeThread;
    std::thread dedicatedThread;
    friend class WriterLock<ThreadSafeStruct<Struct>>;
};

}  // namespace Base::Threads

#include "ThreadSafeStruct.inl"

#endif  // BASE_Threads_ThreadSafeStruct_H
