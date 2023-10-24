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

#include "LockPolicy.h"

namespace Base::Threads
{

bool LockPolicy::hasAnyLock()
{
    assert(threadMutexLayers.empty() ==
           (threadExclusiveMutexes.empty() && threadNonExclusiveMutexes.empty()));
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

bool LockPolicy::_areParentsLocked() const
{
    assert(!threadMutexLayers.empty());

    for(auto mutex: mutexes)
    {
        if(!mutex->parent_pair)
        {
            return false;
        }
        bool parent_locked = false;
        parent_locked = parent_locked || threadExclusiveMutexes.count(mutex->parent_pair);
        parent_locked = parent_locked || threadNonExclusiveMutexes.count(mutex->parent_pair);
        if(!parent_locked)
        {
            return false;
        }
    }
    return true;
}


void LockPolicy::_processLock(bool is_exclusive)
{
    if(mutexes.empty())
    {
        // A container.end() does this!
        return;
    }

    /*
     * When we have absolutely no mutexes,
     * just lock as required.
     */
    if(!hasAnyLock())
    {
        assert(threadExclusiveMutexes.empty());
        assert(threadNonExclusiveMutexes.empty());

        // Lock everything that was requested.
        isLayerExclusive.push(is_exclusive);
        threadMutexLayers.push(mutexes);
        if(is_exclusive)
        {
            threadExclusiveMutexes = mutexes;
        }
        else
        {
            threadNonExclusiveMutexes = mutexes;
        }
        return;
    }

    if(is_exclusive)
    {
        _processExclusiveLock();
    }
    else
    {
        _processNonExclusiveLock();
    }
}

void LockPolicy::_processExclusiveLock()
{
    assert(!threadExclusiveMutexes.empty() || !threadNonExclusiveMutexes.empty());
    assert(!threadMutexLayers.empty());

    // remove already locked "exclusive" mutexes from the list.
    for(auto element: threadExclusiveMutexes)
    {
        mutexes.erase(element);
    }

    // do not allow already locked "non-exclusive" mutexes in the list.
    for(auto element: threadNonExclusiveMutexes)
    {
        if(mutexes.count(element))
        {
            // We cannot exclusively lock if it is already shared.
            throw ExceptionNoExclusiveOverNonExclusive();
        }
    }

    if(mutexes.empty())
    {
        // Nothing to lock.
        assert(!threadMutexLayers.top().empty());
        return;
    }

    // All parents must be locked.
    if(!_areParentsLocked())
    {
        throw ExceptionExclusiveParentNotLocked();
    }

    threadMutexLayers.emplace(mutexes);
    threadExclusiveMutexes.merge(mutexes);
    isLayerExclusive.push(true);
    assert(!threadMutexLayers.top().empty());
    assert(!threadExclusiveMutexes.empty());
}

void LockPolicy::_processNonExclusiveLock()
{
    assert(!threadExclusiveMutexes.empty() || !threadNonExclusiveMutexes.empty());
    assert(!threadMutexLayers.empty());

    // remove already locked "exclusive" mutexes from the list.
    for(auto element: threadExclusiveMutexes)
    {
        mutexes.erase(element);
    }

    // remove already locked "non-exclusive" mutexes from the list.
    for(auto element: threadNonExclusiveMutexes)
    {
        mutexes.erase(element);
    }

    if(mutexes.empty())
    {
        // Nothing to lock.
        assert(!threadMutexLayers.top().empty());
        return;
    }

    if(isLayerExclusive.top())
    {
        if(!_areParentsLocked())
        {
            throw ExceptionNoLocksAfterExclusiveLock();
        }
        threadMutexLayers.emplace();
        isLayerExclusive.push(false);
    }

    threadMutexLayers.top().merge(mutexes);
    threadNonExclusiveMutexes.merge(mutexes);
}

LockPolicy::~LockPolicy()
{
    if(mutexes.empty())
    {
        return;
    }

    for(auto mutex: mutexes)
    {
        threadExclusiveMutexes.erase(mutex);
        threadNonExclusiveMutexes.erase(mutex);
        threadMutexLayers.top().erase(mutex);
    }

    assert(!threadMutexLayers.empty());
    if(!threadMutexLayers.empty() && threadMutexLayers.top().empty())
    {
        threadMutexLayers.pop();
    }
}


SharedLock::SharedLock()
    : LockPolicy(false)
{}

SharedLock::SharedLock(MutexPair& mutex)
    : LockPolicy(false, &mutex)
{
    if(!mutexes.empty())
    {
        assert(mutexes.size() == 1);
        assert(mutexes.count(&mutex));
        lock = std::shared_lock(mutex.mutex);
    }
}

thread_local std::unordered_set<const MutexPair*> LockPolicy::threadExclusiveMutexes;
thread_local std::unordered_set<const MutexPair*> LockPolicy::threadNonExclusiveMutexes;
thread_local std::stack<bool> LockPolicy::isLayerExclusive;
thread_local std::stack<std::unordered_set<const MutexPair*>> LockPolicy::threadMutexLayers;

} //namespace Base::Threads
