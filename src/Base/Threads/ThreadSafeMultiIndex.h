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

#ifndef BASE_Threads_ThreadSafeMultiIndex_H
#define BASE_Threads_ThreadSafeMultiIndex_H

#include "MultiIndexContainer.h"

#include "ThreadSafeContainer.h"

namespace Base::Threads
{

/**
 * @brief Implements a thread safe container that:
 * 0. Manages a tuple of objects.
 * 1. Keeps the order of insertion.
 * 2. Can look up any object with efficenty.
 *
 * Possible uses:
 * 1. An std::map that keeps track the order of insertion.
 * 2. An std::map that can be searched in both directions.
 */
template<typename Record, auto ...LocalPointers>
class ThreadSafeMultiIndex
    : public ThreadSafeContainer<MultiIndexContainer<Record, LocalPointers...>>
{
public:
    using self_t = ThreadSafeMultiIndex;
    using parent_t = ThreadSafeContainer<MultiIndexContainer<Record, LocalPointers...>>;
    using iterator = typename parent_t::iterator;
    using const_iterator = typename parent_t::const_iterator;

    using element_t = Record;

#if defined(__GNUC__)
    // TODO: remove "typename" in a few years. gcc-14 works. (2023, now)
    // https://stackoverflow.com/questions/25940365/what-is-the-standard-conform-syntax-for-template-constructor-inheritance
    using typename parent_t::ThreadSafeContainer;
#else
    using parent_t::ThreadSafeContainer;
#endif

    template<typename Key>
    auto find(const Key& key)
    {return iterator(mutex, container.find(key));}

    template<typename Key>
    auto find(const Key& key) const
    {return const_iterator(mutex, container.find(key));}

    template<std::size_t I, typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains<>(key);}

    template<typename Key>
    bool contains(const Key& key) const
    {SharedLock l(mutex); return container.template contains<>(key);}

protected:
    using parent_t::mutex;
    using parent_t::container;
};

} //namespace ::Threads

#endif // BASE_Threads_ThreadSafeMultiIndex_H
