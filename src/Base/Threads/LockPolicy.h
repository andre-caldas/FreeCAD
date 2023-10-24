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

#include <type_traits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include <stack>

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
    std::shared_mutex mutex;
    MutexPair* parent_pair = nullptr;
};


/**
 * @brief Implements the following policy: (see README.md)
 * 1. A `ExclusiveLock` is allowed to be called if the same thread already has a `ExclusiveLock`.
 *    But only if all mutexes being  are in fact already .
 *    Otherwise, a `ExclusiveLock` cannot be constructed by a thread that already owns any kind of lock on any mutex.
 *    This is a programming error and should not happen. An exception will be thrown.
 * 2. If `SharedLock` requests a mutex G and the thread already has a `ExclusiveLock U`:
 *    2.1. If U **does not own** G, throw and exception.
 *         This is a programming error and should not happen.
 *    2.2. If U **owns** G, do nothing. Just pretend we acquired the lock.
 *
 * @attention Description needs update to explain mutexes hierarchy.
 */
class LockPolicy
{
public:
    static bool hasAnyLock();
    static bool isLocked(const MutexPair* mutex);
    static bool isLocked(const MutexPair& mutex);
    static bool isLockedExclusively(const MutexPair* mutex);
    static bool isLockedExclusively(const MutexPair& mutex);

protected:
    /**
     * @brief Implements the lock policy.
     * @param is_exclusive - Is it an exclusive lock?
     * @param mutex - Each pair is composed of the mutex to be locked (first)
     * and a mutex that if already locked imposes a new layer for threadMutexesLayers.
     */
    template<typename... MutN,
             std::enable_if_t<(std::is_same_v<MutexPair, MutN> && ...)>* = nullptr>
    LockPolicy(bool is_exclusive, MutN*... mutex);
    LockPolicy() = delete;
    ~LockPolicy();

    std::unordered_set<const MutexPair*> mutexes;

private:
    static thread_local std::unordered_set<const MutexPair*> threadExclusiveMutexes;
    static thread_local std::unordered_set<const MutexPair*> threadNonExclusiveMutexes;
    static thread_local std::stack<bool> isLayerExclusive;
    static thread_local std::stack<std::unordered_set<const MutexPair*>> threadMutexLayers;

    bool _areParentsLocked() const;
    void _processLock(bool is_exclusive);
    void _processExclusiveLock();
    void _processNonExclusiveLock();
};


class SharedLock : public LockPolicy
{
public:
    SharedLock();
    [[nodiscard]]
    SharedLock(MutexPair& mutex);

private:
    std::shared_lock<std::shared_mutex> lock;
};


namespace detail {
template<typename Result, typename From>
struct ForEach {using type = Result;};
}

class ExclusiveLockBase {};

/**
 * @brief Locks and gives access to locked classes of type "MutexHolder".
 * The MutexHolder must:
 * 1. Define a MutexHolder::ModifierGate class
 *    that implements the container methods that demand ExclusiveLock.
 * 2. Define a method that takes an ExclusiveLock as argument,
 *    and returns a ModifierGate instance.
 */
template<typename... MutexHolder>
class ExclusiveLock
    : public LockPolicy
    , public ExclusiveLockBase
{
public:
    [[nodiscard]]
    ExclusiveLock(MutexHolder&... mutex_holder);

    // This could actually be static.
    template<typename SomeHolder>
    auto operator[](SomeHolder& tsc) const;

private:
    /*
     * After constructed, std::scoped_lock cannot be changed.
     * So, we usa a unique_ptr.
     * We could, of course, simply not use a scoped_lock,
     * and lock on construction and unlock on destruction.
     * But, unfortunately, std::lock needs two or more mutexes,
     * and we do not want the code full of ifs.
     */
    std::unique_ptr<std::scoped_lock<typename detail::ForEach<std::shared_mutex,MutexHolder>::type...>> locks;
};

} //namespace Base::Threads

#include "LockPolicy.inl"

#endif // BASE_Threads_LockPolicy_H
