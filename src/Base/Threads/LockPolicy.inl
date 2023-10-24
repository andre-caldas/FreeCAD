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
         std::enable_if_t<(std::is_same_v<MutexPair, MutN> && ...)>*>
LockPolicy::LockPolicy(bool is_exclusive, MutN*... mutex)
    : mutexes{mutex...}
{
    _processLock(is_exclusive);
}


/*
 * Traits: we want to pass either a mutex or a container to ExclusiveLock.
 */
template<typename C>
struct MutexPairPointer
{
    MutexPairPointer(const C& container) : container(container) {}
    auto getPair() {return container.getMutexPtr();}
    const C& container;
};
template<>
struct MutexPairPointer<MutexPair*>
{
    MutexPairPointer(MutexPair* mutex) : mutex(mutex) {}
    auto getPair() {return mutex;}
    MutexPair* mutex;
};
template<>
struct MutexPairPointer<MutexPair> : MutexPairPointer<MutexPair*>
{
    MutexPairPointer(MutexPair& mutex) : MutexPairPointer<MutexPair*>(&mutex) {}
};

template<typename... MutexPairOrContainer>
ExclusiveLock<MutexPairOrContainer...>::ExclusiveLock(MutexPairOrContainer&... pair_or_container)
    : LockPolicy(true, MutexPairPointer{pair_or_container}.getPair()...)
{
    /*
     * Here we know that if this is not the first lock,
     * all previous locks in the same layer were exclusive
     * and the very first one already contains this one.
     *
     * If this is not the first one, mutexes is empty.
     * Otherwise, mutexes = {container.getMutexPtr()...}.
     *
     * Example:
     * ExclusiveLock l1(m1, m2);
     * ExclusiveLock l2(m1); // Does nothing.
     */
    assert(mutexes.empty() || mutexes.size() == sizeof...(MutexPairOrContainer));
    if(mutexes.size() == sizeof...(MutexPairOrContainer))
    {
        /*
         * It would be more natural if we could pass "mutexes" to the constructor.
         * But there is only the option to list all mutexes at compile time.
         * Fortunately, mutexes = {container.getMutexPtr()...}.
         */
        locks = std::make_unique<
            std::scoped_lock<typename detail::ForEach<std::shared_mutex, MutexPairOrContainer>::type...>
        >(MutexPairPointer{pair_or_container}.getPair()->mutex...);
    }
}

template<typename... MutexHolder>
template<typename SomeHolder>
auto ExclusiveLock<MutexHolder...>::operator[](SomeHolder& tsc) const
{
    auto gate = tsc.getModifierGate(this);
    if(!isLockedExclusively(gate.getMutexPtr()))
    {
        throw ExceptionNeedLockToAccessContainer();
    }
    return gate;
}

} //namespace Base::Threads
