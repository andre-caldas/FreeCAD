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

template<typename ItType>
class EndAwareIterator
{
public:
    /* I have absolutely no idea if those are correct. Probably not! :-( */
    using difference_type = typename ItType::difference_type;
    using value_type = typename ItType::value_type;
    using pointer = typename ItType::pointer;
    using reference = typename ItType::reference;
    using iterator_category = typename ItType::iterator_category;

    EndAwareIterator(const EndAwareIterator& other)
        : it(other.it)
        , end_it(other.end_it)
    {}

    EndAwareIterator(ItType&& it, ItType end_it)
        : it(std::move(it))
        , end_it(std::move(end_it))
    {}

    EndAwareIterator(std::pair<ItType,ItType>&& range)
        : it(std::move(range.first))
        , end_it(std::move(range.second))
    {}

    /**
     * @brief This iterator is aware of its "end".
     * So, we can convert it to "true" when it is not "end",
     * and false otherwise.
     */
    operator bool() const {return it != end_it;}

    EndAwareIterator& operator=(const EndAwareIterator& other)
    {it = other.it; end_it = other.end_it; return *this;}

    constexpr bool operator==(const EndAwareIterator& other) const {return it == other.it;}
    constexpr bool operator!=(const EndAwareIterator& other) const {return it != other.it;}
    EndAwareIterator& operator++() {++it; return *this;}
    EndAwareIterator operator++(int) {EndAwareIterator result(*this); ++it; return result;}
    auto& operator*() const {return *it;}
    auto* operator->() const {return &*it;}

    virtual ~EndAwareIterator() = default;

private:
    ItType it;
    ItType end_it;
};


/**
 * @brief
 */
template<typename ItType>
class LockedIterator
    : public EndAwareIterator<ItType>
{
public:
    /* I have absolutely no idea if those are correct. Probably not! :-( */
    using difference_type = typename ItType::difference_type;
    using value_type = typename ItType::value_type;
    using pointer = typename ItType::pointer;
    using reference = typename ItType::reference;
    using iterator_category = typename ItType::iterator_category;

    // Attention: do not lock mutex again!
    LockedIterator(const LockedIterator& other)
        : EndAwareIterator<ItType>(other)
    {}

    /**
     * @brief An iterator (wrapper) that locks the mutex using SharedLock.
     * @param mutex - the mutex to lock.
     * @param it - original iterator to be wrapped.
     * @param end_it - end iterator, so we know when we reach the end.
     */
    LockedIterator(MutexPair& mutex, ItType&& it, ItType end_it)
        : EndAwareIterator<ItType>(std::move(it), std::move(end_it))
        , lock(mutex)
    {}

    LockedIterator(MutexPair& mutex, std::pair<ItType,ItType>&& range)
        : EndAwareIterator<ItType>(std::move(range))
        , lock(mutex)
    {}

    // We need LockedIterator to be copy assignable to use algorithms.
    // But we shall never use it. We do not change mutexes, only the iterator.
    // So, this messes with LockPolicy.
    LockedIterator& operator=(const LockedIterator& other)
    {assert(false); it = other.it; end_it = other.end_it; return *this;}

    constexpr bool operator==(const LockedIterator& other) const {return it == other.it;}
    constexpr bool operator!=(const LockedIterator& other) const {return it != other.it;}
    LockedIterator& operator++() {++it; return *this;}
    LockedIterator operator++(int) {LockedIterator result(*this); ++it; return result;}
    auto& operator*() const {return *it;}
    auto* operator->() const {return &*it;}

    const auto& getIterator() const {return it;}
    auto& getIterator() {return it;}

    /**
     * @brief Static method to provide a LockedIterator that does not lock anything.
     * It shall be used only to construct and "end" iterator.
     * @param end_it - original container's end iterator.
     * @return An "end" iterator of type LockedIterator<ItType>.
     */
    template<typename EndIterator>
    static LockedIterator<ItType> MakeEndIterator(EndIterator&& end_it)
    {return LockedIterator(std::move(end_it));}

private:
    mutable SharedLock lock;
    ItType it;
    ItType end_it;

    /**
     * @brief A wrapper that does not actually lock anything.
     * Shall be used only for the "end()" iterator.
     * @param it - the "end()" iterator to be wrapped.
     * @attention It is better if the end() iterator and the other iterators
     * are of the same type.
     * Therefore, we also wrap the "end()" iterator.
     * But, since it might be short lived, we do not want the lock to be tied to it.
     */
    explicit LockedIterator(ItType&& it)
        : EndAwareIterator<ItType>(std::move(it), it)
    {}
};

} //namespace Base::Threads

#endif // BASE_Threads_LockedIterator_H
