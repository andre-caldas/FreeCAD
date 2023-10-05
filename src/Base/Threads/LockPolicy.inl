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

#include <algorithm>
#include <shared_mutex>

#include "Exception.h"
#include "LockPolicy.h"

namespace Base::Threads
{

template<typename... MutN,
         std::enable_if_t<(std::is_same_v<std::shared_mutex, MutN> && ...)>*>
LockPolicy::LockPolicy(bool is_exclusive, MutN*... mutex)
    : mutexes{mutex...}
{
    if(threadMutexes.empty())
    {
        // Lock anything that was requested.
        threadMutexes = mutexes;
        isExclusive = is_exclusive;
        return;
    }

    if(is_exclusive && !isExclusive)
    {
        // No request for exclusive lock when we already hold shared locks.
        throw ExceptionExclusiveNotFirst();
    }

    // remove already  mutexes from the list.
    for(auto element: threadMutexes)
    {
        // This only erases if element exists in "mutexes". :-)
        mutexes.erase(element);
    }

    if(mutexes.empty())
    {
        // Nothing to lock.
        return;
    }

    if(isExclusive)
    {
        // If we hold exclusive locks, we cannot wait for new mutexes.
        // This is to avoid deadlocks.
        throw ExceptionNoLocksAfterExclusiveLock();
    }

    threadMutexes.merge(mutexes);
}


template<typename... MutN,
         std::enable_if_t<(std::is_same_v<std::shared_mutex, MutN> && ...)>*>
ExclusiveLock::ExclusiveLock(MutN&... each_mutex)
    : LockPolicy(true, &each_mutex...)
{
    // Policy dictates that we can do:
    // ExclusiveLock l1(m1, m2);
    // ExclusiveLock l2(m1); // Does nothing.
    assert(mutexes.empty() || mutexes.size() == sizeof...(MutN));
    if(mutexes.size() == sizeof...(MutN))
    {
        locks = std::make_unique<std::scoped_lock>(each_mutex...);
    }
}

template<typename ThrSfCont>
decltype(ThrSfCont::container)& ExclusiveLock::operator[](ThrSfCont& tsc)
{
    if(!threadMutexes.count(&tsc.mutex))
    {
        throw ExceptionNeedLockToAccessContainer();
    }
    return tsc.container;
}

} //namespace Base::Threads
