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

#ifndef BASE_Threads_LockedIterator_H
#define BASE_Threads_LockedIterator_H

#include <shared_mutex>

#include "LockPolicy.h"

namespace Base::Threads
{

/**
 * @brief
 */
template<typename ItType>
class LockedIterator
{
public:
    /* I have absolutely no idea if those are correct. Probably not! :-( */
    using difference_type = typename ItType::difference_type;
    using value_type = typename ItType::value_type;
    using pointer = typename ItType::pointer;
    using reference = typename ItType::reference;
    using iterator_category = typename ItType::iterator_category;

    LockedIterator(const LockedIterator& other) : it(other.it) {}

    /**
     * @brief An iterator (wrapper) that locks the mutex using SharedLock.
     * @param mutex - the mutex to lock.
     * @param it - original iterator to be wrapped.
     */
    LockedIterator(std::shared_mutex& mutex, ItType&& it)
        : lock(mutex)
        , it(std::move(it))
    {}

    // We need LockedIterator to be copy assignable to use algorithms.
    // But we shall never use it. We do not change mutexes, only the iterator.
    // So, this messes with LockPolicy.
    LockedIterator& operator=(const LockedIterator& other)
    {assert(false); it = other.it; return *this;}

    constexpr bool operator==(const LockedIterator& other) const {return it == other.it;}
    LockedIterator& operator++() {it++; return *this;}
    auto& operator*() const {return *it;}
    auto* operator->() const {return &*it;}
    operator ItType&() {return it;}

    template<typename Container>
    static LockedIterator<ItType> MakeEndIterator(Container& c)
    {return LockedIterator(c.end());}

    template<typename Container>
    static LockedIterator<ItType> MakeCEndIterator(const Container& c)
    {return LockedIterator(c.cend());}

private:
    mutable SharedLock lock;
    ItType it;

    /**
     * @brief A wrapper that does not actually lock anything.
     * Shall be used only for the "end()" iterator.
     * @param it - the "end()" iterator to be wrapped.
     * @attention It is better if the end() iterator and the other iterators
     * are of the same type.
     * Therefore, we also wrap the "end()" iterator.
     * But, since it might be short lived, we do not want the lock to be tied to it.
     */
    LockedIterator(ItType&& it)
        : it(std::move(it))
    {}
};

} //namespace Base::Threads

#endif // BASE_Threads_LockedIterator_H
