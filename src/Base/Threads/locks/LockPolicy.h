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

#ifndef BASE_Threads_LockPolicy_H
#define BASE_Threads_LockPolicy_H

#ifndef FC_GLOBAL_H
#include <FCGlobal.h>
#endif
#include <type_traits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include <stack>

#include "../type_traits/Utils.h"

#include "YesItIsAMutex.h"

namespace Base::Threads
{

/**
 * @brief Even when an @class ExlcusiveLock is requested in a situation
 * where we already have any @class SharedLock,
 * this will be allowed as long as long as the following conditions are satisfied.
 * 1. This @a mutex is not locked yet.
 * 2. Its @ parent_mutex is already locked (only a shared_lock is enough).
 */
struct MutexPair
{
    MutexPair() = default;
    MutexPair(MutexPair* parent)
        : parent_pair(parent)
    {}
    YesItIsAMutex mutex;
    MutexPair* parent_pair = nullptr;
};


/**
 * @brief Implements the following policy: (see README.md)
 * 1. A `ExclusiveLock` is allowed to be called if the same thread
 *    already has an `ExclusiveLock`.
 *    But only if all mutexes being locked are in fact already locked.
 *    Otherwise, an `ExclusiveLock` cannot be constructed by a thread
 *    that already owns any kind of lock on any mutex.
 *    This is a programming error and should not happen.
 *    An exception will be thrown.
 * 2. If `SharedLock` requests a mutex G and the thread already has an `ExclusiveLock U`:
 *    2.1. If U **does not own** G, throw and exception.
 *         This is a programming error and should not happen.
 *    2.2. If U **owns** G, do nothing. Just pretend we acquired the lock.
 *
 * @attention Description needs update to explain mutexes hierarchy.
 */
class BaseExport LockPolicy
{
public:
    static bool hasAnyLock();
    static bool isLocked(const MutexPair* mutex);
    static bool isLocked(const MutexPair& mutex);
    static bool isLockedExclusively(const MutexPair* mutex);
    static bool isLockedExclusively(const MutexPair& mutex);

    void detachFromThread();
    void attachToThread(bool is_exclusive);

protected:
    /**
     * @brief Implements the lock policy.
     * @param is_exclusive - Is it an exclusive lock?
     * @param is_lock_free - Is the programmer sure that
     * no other lock will be waited for while holding this one?
     * @param mutex - Each pair is composed of the mutex to be locked (first)
     * and a mutex that if already locked imposes a new layer for threadMutexesLayers.
     */
    template<typename... MutN,
             std::enable_if_t<(std::is_same_v<MutexPair, MutN> && ...)>* = nullptr>
    LockPolicy(bool is_exclusive, bool is_lock_free, MutN*... mutex);

    LockPolicy() = delete;
    virtual ~LockPolicy();

    bool isDetached() const;
    bool hasIgnoredMutexes() const;
    const std::unordered_set<const MutexPair*>& getMutexes() const;

private:
    bool is_detached = true;
    bool has_ignored_mutexes = false;
    std::unordered_set<const MutexPair*> mutexes;

    bool _areParentsLocked() const;
    void _processLock(bool is_exclusive, bool is_lock_free);
    void _processExclusiveLock(bool is_lock_free);
    void _processNonExclusiveLock(bool is_lock_free);
    /**
     * @brief Removes information from thread_local variables.
     */
    void _detachFromThread();
};

/*
 * TODO: It hurts my feelings to disable clang-format.
 * Please, remove it if you find it inappropriate.
 * It is used here to avoid [[nodiscard]] being put in the same line as
 * the constructor/method declaration.
 */

class BaseExport SharedLock: public LockPolicy
{
public:
    SharedLock();

    // clang-format off
    [[nodiscard]]
    SharedLock(MutexPair& mutex)
        : SharedLock(false, mutex)
    {}
    // clang-format on

protected:
    [[nodiscard]] SharedLock(bool is_lock_free, MutexPair& mutex);

private:
    std::shared_lock<YesItIsAMutex> lock;
};


class BaseExport SharedLockFreeLock: public SharedLock
{
public:
    SharedLockFreeLock() = default;

    [[nodiscard]] SharedLockFreeLock(MutexPair& mutex);
};


class ExclusiveLockBase
{
};

/**
 * @brief Locks and gives access to locked classes of type "MutexHolder".
 * The MutexHolder must:
 * 1. Define a MutexHolder::WriterGate class
 *    that implements the container methods that demand ExclusiveLock.
 * 2. Define a method that takes an ExclusiveLock as argument,
 *    and returns a WriterGate instance.
 */
template<typename FirstMutexHolder, typename... MutexHolder>
class ExclusiveLock: public LockPolicy, public ExclusiveLockBase
{
public:
    // clang-format off
    [[nodiscard]]
    ExclusiveLock(FirstMutexHolder& first_holder, MutexHolder&... mutex_holder)
        : ExclusiveLock(false, first_holder, mutex_holder...)
    {}
    // clang-format on

    // This could actually be static.
    template<typename SomeHolder>
    auto& operator[](SomeHolder& tsc) const;

    template<typename = std::enable_if_t<sizeof...(MutexHolder) == 0>>
    auto operator->() const;

    void release();
    [[maybe_unused]] auto detachFromThread();

protected:
    // clang-format off
    [[nodiscard]]
    ExclusiveLock(bool is_lock_free, FirstMutexHolder& first_holder, MutexHolder&... mutex_holder);
    // clang-format on

private:
    using locks_t = std::scoped_lock<YesItIsAMutex, ForEach_t<YesItIsAMutex, MutexHolder>...>;
    /*
     * After constructed, std::scoped_lock cannot be changed.
     * So, we usa a unique_ptr.
     * We could, of course, simply not use a scoped_lock,
     * and lock on construction and unlock on destruction.
     * But, unfortunately, std::lock needs two or more mutexes,
     * and we do not want the code full of ifs.
     *
     * We use a shared_ptr because when a new thread is created,
     * we want them both to hold the lock, while we store the new thread information:
     * the std::thread instance.
     */
    std::shared_ptr<locks_t> locks;

    FirstMutexHolder& firstMutexHolder;
};

template<typename FirstMutexHolder, typename... MutexHolder>
class ExclusiveLockFreeLock: public ExclusiveLock<FirstMutexHolder, MutexHolder...>
{
public:
    // clang-format off
    [[nodiscard]]
    ExclusiveLockFreeLock(FirstMutexHolder& first_holder, MutexHolder&... mutex_holder)
        : ExclusiveLock<FirstMutexHolder, MutexHolder...>(true, first_holder, mutex_holder...)
    {}
    // clang-format on
};

}  // namespace Base::Threads

#include "LockPolicy.inl"

#endif  // BASE_Threads_LockPolicy_H
