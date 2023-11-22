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

#include "../Exception.h"
#include "LockPolicy.h"

namespace Base::Threads
{

template<typename... MutN, std::enable_if_t<(std::is_same_v<MutexPair, MutN> && ...)>*>
LockPolicy::LockPolicy(bool is_exclusive, bool is_lock_free, MutN*... mutex)
    : mutexes {mutex...}
{
    _processLock(is_exclusive, is_lock_free);
}


// clang-format off
/*
 * Traits: we want to pass either a mutex or a container to ExclusiveLock.
 */
template<typename C>
struct MutexPairPointer
{
    MutexPairPointer(const C& container): container(container) {}
    auto getPair() {return container.getMutexPair();}
    const C& container;
};

template<>
struct MutexPairPointer<MutexPair*>
{
    MutexPairPointer(MutexPair* mutex): mutex(mutex) {}
    auto getPair() {return mutex;}
    MutexPair* mutex;
};

template<>
struct MutexPairPointer<MutexPair>: MutexPairPointer<MutexPair*>
{
    MutexPairPointer(MutexPair& mutex): MutexPairPointer<MutexPair*>(&mutex) {}
};
// clang-format on


template<typename FirstMutexHolder, typename... MutexHolder>
ExclusiveLock<FirstMutexHolder, MutexHolder...>::ExclusiveLock(bool is_lock_free,
                                                               FirstMutexHolder& first_holder,
                                                               MutexHolder&... holder)
    : LockPolicy(true,
                 is_lock_free,
                 MutexPairPointer {first_holder}.getPair(),
                 MutexPairPointer {holder}.getPair()...)
    , firstMutexHolder(first_holder)
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
    assert(getMutexes().empty() || getMutexes().size() == 1 + sizeof...(MutexHolder));
    if (getMutexes().size() == 1 + sizeof...(MutexHolder)) {
        /*
         * It would be more natural if we could pass "mutexes" to the constructor.
         * But there is only the option to list all mutexes at compile time.
         * Fortunately, mutexes = {container.getMutexPtr()...}.
         */
        locks = std::make_unique<locks_t>(MutexPairPointer {first_holder}.getPair()->mutex,
                                          MutexPairPointer {holder}.getPair()->mutex...);
    }
}

template<typename FirstMutexHolder, typename... MutexHolder>
template<typename SomeHolder>
auto& ExclusiveLock<FirstMutexHolder, MutexHolder...>::operator[](SomeHolder& tsc) const
{
    if (!isLockedExclusively(tsc.getMutexPair())) {
        throw ExceptionNeedLockToAccessContainer();
    }
    auto& gate = tsc.getWriterGate(this);
    return *gate;
}

template<typename FirstMutexHolder, typename... MutexHolder>
template<typename>
auto ExclusiveLock<FirstMutexHolder, MutexHolder...>::operator->() const
{
    if (!isLockedExclusively(firstMutexHolder.getMutexPair())) {
        throw ExceptionNeedLockToAccessContainer();
    }
    auto& gate = firstMutexHolder.getWriterGate(this);
    return &*gate;
}

template<typename FirstMutexHolder, typename... MutexHolder>
void ExclusiveLock<FirstMutexHolder, MutexHolder...>::release()
{
    locks.reset();
}

}  // namespace Base::Threads
