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

#ifndef BASE_Threads_WeakPtrRecord_H
#define BASE_Threads_WeakPtrRecord_H

#include <memory>

#include "../ThreadSafeMultiIndex.h"

namespace Base::Threads
{

template<typename T>
struct WeakPtrRecord
{
    WeakPtrRecord(std::shared_ptr<T> smart)
        : raw_pointer(smart.get())
        , smart_pointer(std::move(smart))
    {}

    T* raw_pointer;
    std::weak_ptr<T> smart_pointer;

    std::shared_ptr<T> lock() const
    {return smart_pointer.lock();}

    // TODO: use <=> when we pass to C++20
    bool operator==(WeakPtrRecord& other)
    {return raw_pointer == other.raw_pointer;}

    // TODO: use <=> when we pass to C++20
    bool operator==(T* other)
    {return raw_pointer == other;}
};

/**
 * @brief A list of weak_ptr that can be iterated in insertion order,
 * and can be searched by raw pointer.
 */
template<typename T>
using ThreadSafeWeakPtrList = ThreadSafeMultiIndex<WeakPtrRecord<T>,
                                                   &WeakPtrRecord<T>::raw_pointer>;

} // namespace Base::Threads

#endif // BASE_Threads_WeakPtrRecord_H
