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

#include "../Exception.h"

#include "LockPolicy.h"

namespace Base::Threads
{

namespace
{
thread_local std::unordered_set<const MutexPair*> threadExclusiveMutexes;
thread_local std::unordered_set<const MutexPair*> threadNonExclusiveMutexes;
thread_local std::stack<bool> isLayerExclusive;
thread_local std::stack<std::unordered_set<const MutexPair*>> threadMutexLayers;
thread_local bool isExecutingLockFree = false;
}  // namespace

bool LockPolicy::hasAnyLock()
{
    assert(threadMutexLayers.empty()
           == (threadExclusiveMutexes.empty() && threadNonExclusiveMutexes.empty()));
    return !threadMutexLayers.empty();
}

bool LockPolicy::isLocked(const MutexPair* mutex)
{
    bool has_exclusive = threadExclusiveMutexes.count(mutex);
    bool has_non_exclusive = threadNonExclusiveMutexes.count(mutex);
    return has_exclusive || has_non_exclusive;
}

bool LockPolicy::isLocked(const MutexPair& mutex)
{
    return isLocked(&mutex);
}

bool LockPolicy::isLockedExclusively(const MutexPair* mutex)
{
    return threadExclusiveMutexes.count(mutex);
}

bool LockPolicy::isLockedExclusively(const MutexPair& mutex)
{
    return isLockedExclusively(&mutex);
}

void LockPolicy::detachFromThread()
{
    if (has_ignored_mutexes) {
        assert(false);
        throw ExceptionNewThreadRequiresReleaseableLock {};
    }
    assert(!mutexes.empty());

    if (isExecutingLockFree) {
        assert(false);
        throw ExceptionNewThreadRequiresReleaseableLock {};
    }

    // Confirm all mutexes are in the topmost layer.
    assert(!threadMutexLayers.empty());
    auto layer = threadMutexLayers.top();
    for (auto mutex : mutexes) {
        if (!layer.count(mutex)) {
            assert(false);
            throw ExceptionNewThreadRequiresReleaseableLock {};
        }
    }

    _detachFromThread();
}


bool LockPolicy::isDetached() const
{
    return is_detached;
}

bool LockPolicy::hasIgnoredMutexes() const
{
    return has_ignored_mutexes;
}

const std::unordered_set<const MutexPair*>& LockPolicy::getMutexes() const
{
    return mutexes;
}


void LockPolicy::_detachFromThread()
{
    is_detached = true;
    if (mutexes.empty()) {
        return;
    }
    isExecutingLockFree = false;

    for (auto mutex : mutexes) {
        threadExclusiveMutexes.erase(mutex);
        threadNonExclusiveMutexes.erase(mutex);
        assert(!threadMutexLayers.empty());
        threadMutexLayers.top().erase(mutex);
    }

    assert(!threadMutexLayers.empty());
    if (!threadMutexLayers.empty() && threadMutexLayers.top().empty()) {
        threadMutexLayers.pop();
    }
    else {
        assert(!isExecutingLockFree);
    }
}

void LockPolicy::attachToThread(bool is_exclusive)
{
    assert(!isExecutingLockFree);
    assert(!hasAnyLock());
    assert(!mutexes.empty());
    assert(!has_ignored_mutexes);
    if (!is_detached) {
        assert(false);
        throw ExceptionNewThreadRequiresMovedLock {};
    }
    _processLock(is_exclusive, false);
    assert(mutexes.size() == threadExclusiveMutexes.size()
           || mutexes.size() == threadNonExclusiveMutexes.size());
}


bool LockPolicy::_areParentsLocked() const
{
    assert(!threadMutexLayers.empty());
    for (auto mutex : mutexes) {
        if (!mutex->parent_pair) {
            continue;
        }
        bool parent_locked = false;
        parent_locked = parent_locked || threadExclusiveMutexes.count(mutex->parent_pair);
        parent_locked = parent_locked || threadNonExclusiveMutexes.count(mutex->parent_pair);
        if (!parent_locked) {
            return false;
        }
    }
    return true;
}


void LockPolicy::_processLock(bool is_exclusive, bool is_lock_free)
{
    assert(is_detached);
    is_detached = false;

    if (mutexes.empty()) {
        // A container.end() does this!
        return;
    }

    /*
     * When we have absolutely no mutexes,
     * just lock as required.
     */
    if (!hasAnyLock()) {
        if (isExecutingLockFree) {
            assert(false);
            throw ExceptionNoLocksAfterLockFree {};
        }
        isExecutingLockFree = is_lock_free;

        assert(threadExclusiveMutexes.empty());
        assert(threadNonExclusiveMutexes.empty());

        // Lock everything that was requested.
        isLayerExclusive.push(is_exclusive);
        threadMutexLayers.push(mutexes);
        if (is_exclusive) {
            threadExclusiveMutexes = mutexes;
        }
        else {
            threadNonExclusiveMutexes = mutexes;
        }
        return;
    }

    if (is_exclusive) {
        _processExclusiveLock(is_lock_free);
    }
    else {
        _processNonExclusiveLock(is_lock_free);
    }
}

void LockPolicy::_processExclusiveLock(bool is_lock_free)
{
    assert(!threadExclusiveMutexes.empty() || !threadNonExclusiveMutexes.empty());
    assert(!threadMutexLayers.empty());

    // remove already locked "exclusive" mutexes from the list.
    for (auto it = mutexes.begin(); it != mutexes.end();) {
        if (threadExclusiveMutexes.count(*it)) {
            assert(isLockedExclusively(*it));
            it = mutexes.erase(it);
            has_ignored_mutexes = true;
        }
        else {
            ++it;
        }
    }

    // do not allow already locked "non-exclusive" mutexes in the list.
    for (auto mutex : mutexes) {
        if (threadNonExclusiveMutexes.count(mutex)) {
            // We cannot exclusively lock if it is already shared.
            throw ExceptionNoExclusiveOverNonExclusive();
        }
    }

    if (mutexes.empty()) {
        // Nothing to lock.
        assert(!threadMutexLayers.top().empty());
        return;
    }


    if (isExecutingLockFree) {
        assert(false);
        throw ExceptionNoLocksAfterLockFree {};
    }
    isExecutingLockFree = is_lock_free;

    // All parents must be locked.
    if (!_areParentsLocked()) {
        throw ExceptionExclusiveParentNotLocked();
    }

    threadMutexLayers.push(mutexes);
    threadExclusiveMutexes.insert(mutexes.cbegin(), mutexes.cend());
    isLayerExclusive.push(true);
    assert(!threadMutexLayers.top().empty());
    assert(!threadExclusiveMutexes.empty());
}

void LockPolicy::_processNonExclusiveLock(bool is_lock_free)
{
    assert(!threadExclusiveMutexes.empty() || !threadNonExclusiveMutexes.empty());
    assert(!threadMutexLayers.empty());

    // remove already locked "exclusive" and "non-exclusive" mutexes from the list.
    for (auto it = mutexes.begin(); it != mutexes.end();) {
        if (threadExclusiveMutexes.count(*it) || threadNonExclusiveMutexes.count(*it)) {
            assert(isLocked(*it));
            it = mutexes.erase(it);
            has_ignored_mutexes = true;
        }
        else {
            ++it;
        }
    }

    if (mutexes.empty()) {
        // Nothing to lock.
        assert(!threadMutexLayers.top().empty());
        return;
    }

    if (isExecutingLockFree) {
        assert(false);
        throw ExceptionNoLocksAfterLockFree {};
    }
    isExecutingLockFree = is_lock_free;

    assert(!isLayerExclusive.empty());
    if (isLayerExclusive.top()) {
        // Are parents really locked!
        if (!is_lock_free && true /*!_areParentsLocked()*/) {
            throw ExceptionNoLocksAfterExclusiveLock();
        }
        threadMutexLayers.emplace();
        isLayerExclusive.push(false);
    }

    threadMutexLayers.top().insert(mutexes.cbegin(), mutexes.cend());
    threadNonExclusiveMutexes.insert(mutexes.cbegin(), mutexes.cend());
}


LockPolicy::~LockPolicy()
{
    _detachFromThread();
}


SharedLock::SharedLock()
    : LockPolicy(false, false)
{}

SharedLock::SharedLock(bool is_lock_free, MutexPair& mutex)
    : LockPolicy(false, is_lock_free, &mutex)
{
    if (!getMutexes().empty()) {
        assert(getMutexes().size() == 1);
        assert(getMutexes().count(&mutex));
        lock = std::shared_lock(mutex.mutex);
    }
}

SharedLockFreeLock::SharedLockFreeLock(MutexPair& mutex)
    : SharedLock(true, mutex)
{}

}  // namespace Base::Threads
