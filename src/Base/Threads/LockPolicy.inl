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
#include <memory>
#include <mutex>
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


/*
 * Traits: we want to pass either a mutex or a container to ExclusiveLock.
 */
template<typename C>
struct MutexPointer
{
    MutexPointer(const C& container) : container(container) {}
    auto operator()() {return container.getMutexPtr();}
    const C& container;
};
template<>
struct MutexPointer<std::shared_mutex*>
{
    MutexPointer(std::shared_mutex* mutex) : mutex(mutex) {}
    auto operator()() {return mutex;}
    std::shared_mutex* mutex;
};
template<>
struct MutexPointer<std::shared_mutex> : MutexPointer<std::shared_mutex*>
{
    MutexPointer(std::shared_mutex& mutex) : MutexPointer<std::shared_mutex*>(&mutex) {}
};

namespace detail {
template<typename Result, typename From>
struct ForEach{using type = Result;};
}

template<typename... MutexOrContainer>
ExclusiveLock<MutexOrContainer...>::ExclusiveLock(MutexOrContainer&... mutex_or_container)
    : LockPolicy(true, MutexPointer{mutex_or_container}()...)
{
    /*
     * Here we know that if this is not the first lock,
     * all previous locks were exclusive and the very first one
     * already contains this one.
     *
     * If this is not the first one, mutexes is empty.
     * Otherwise, mutexes = {container.getMutexPtr()...}.
     *
     * Example:
     * ExclusiveLock l1(m1, m2);
     * ExclusiveLock l2(m1); // Does nothing.
     */
    assert(mutexes.empty() || mutexes.size() == sizeof...(MutexOrContainer));
    if(mutexes.size() == sizeof...(MutexOrContainer))
    {
        /*
         * It would be more natural if we could pass "mutexes" to the constructor.
         * But there is only the option to list all mutexes at compile time.
         * Fortunately, mutexes = {container.getMutexPtr()...}.
         */
        locks = std::make_unique<
            std::scoped_lock<typename detail::ForEach<std::shared_mutex, MutexOrContainer>::type...>
        >(*MutexPointer{mutex_or_container}()...);
    }
}

template<typename... MutexHolder>
template<typename SomeHolder>
auto ExclusiveLock<MutexHolder...>::operator[](SomeHolder& tsc) const
{
    auto gate = tsc.getModifierGate(this);
    if(!threadMutexes.count(gate.getMutexPtr()))
    {
        throw ExceptionNeedLockToAccessContainer();
    }
    return gate;
}

} //namespace Base::Threads
