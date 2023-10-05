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

namespace Base::Threads
{

/**
 * @brief
 */
template<typename ItType>
class LockedIterator
{
public:
    LockedIterator(std::shared_mutex& mutex, ItType&& it)
        : lock(mutex)
        , it(std::move(it))
    {}
    LockedIterator(ItType&& it)
        : it(std::move(it))
    {}

    LockedIterator& operator=(const LockedIterator&) = delete;
    LockedIterator& operator++() {it++; return *this;}
    typename ItType::value_type& operator*() {return *it;}
    operator ItType&() {return it;}

private:
    mutable std::shared_lock<std::shared_mutex> lock;
    ItType it;
};

} //namespace Base::Threads

#endif // BASE_Threads_LockedIterator_H
