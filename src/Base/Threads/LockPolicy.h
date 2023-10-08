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

namespace Base::Threads
{

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
 */
class LockPolicy
{
public:
    static bool hasAnyLock() {return !threadMutexes.empty();}

protected:
    LockPolicy() = delete;
    template<typename... MutN,
             std::enable_if_t<(std::is_same_v<std::shared_mutex, MutN> && ...)>* = nullptr>
    LockPolicy(bool is_exclusive, MutN*... mutex);
    ~LockPolicy();

    std::unordered_set<std::shared_mutex*> mutexes;

protected:
    static thread_local bool isExclusive;
    static thread_local std::unordered_set<std::shared_mutex*> threadMutexes;
};


class SharedLock : public LockPolicy
{
public:
    SharedLock();
    SharedLock(std::shared_mutex& mutex);

private:
    std::shared_lock<std::shared_mutex> lock;
};


class ExclusiveLock : public LockPolicy
{
public:
    template<typename... ThrSfCont>
    ExclusiveLock(ThrSfCont&... container);

    // This could actually be static.
    template<typename ThrSfCont>
    typename ThrSfCont::container_type& operator[](ThrSfCont& tsc);

    static bool hasAnyLock() {return isExclusive && !threadMutexes.empty();}
    template<typename ThrSfCont>
    static bool hasLock(const ThrSfCont& c) {return isExclusive && threadMutexes.count(&c.mutex);}

private:
    /*
     * After constructed, std::scoped_lock cannot be changed.
     * So, we usa a unique_ptr.
     * We could, of course, simply not use a scoped_lock,
     * and lock on construction and unlock on destruction.
     * But, unfortunately, std::lock needs two or more mutexes,
     * and we do not want the code full of ifs.
     */
    std::unique_ptr<std::scoped_lock<std::shared_mutex>> locks;
};

} //namespace Base::Threads

#include "LockPolicy.inl"

#endif // BASE_Threads_LockPolicy_H
