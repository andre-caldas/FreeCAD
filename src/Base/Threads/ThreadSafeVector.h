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

#ifndef BASE_Threads_ThreadSafeVector_H
#define BASE_Threads_ThreadSafeVector_H

#include <vector>

#include "ThreadSafeContainer.h"

namespace Base::Threads
{

/**
 * @brief This wraps an std::map and provides a shared_mutex
 * to make the map readable by many only when the map structure
 * is not being modified.
 * The map elements are not protected, just the map structure.
 * To protect each element, use std::atomic.
 *
 * Iterators use a shared_lock.
 * Methods that change the map structure use a lock.
 */
template<typename Val>
class ThreadSafeVector
    : public ThreadSafeContainer<std::vector<Val>>
{
};

} //namespace Base::Threads

#endif // BASE_Threads_ThreadSafeVector_H
