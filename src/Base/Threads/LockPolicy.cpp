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

#include "LockPolicy.h"

namespace Base::Threads
{

LockPolicy::~LockPolicy()
{
    for(auto mutex: mutexes)
    {
        threadMutexes.erase(mutex);
    }
}


SharedLock::SharedLock()
    : LockPolicy(false)
{}

SharedLock::SharedLock(std::shared_mutex& mutex)
    : LockPolicy(false, &mutex)
{
    if(!mutexes.empty())
    {
        assert(mutexes.size() == 1);
        assert(mutexes.count(&mutex));
        lock = std::shared_lock(mutex);
    }
}

thread_local bool LockPolicy::isExclusive = false;
thread_local std::unordered_set<const std::shared_mutex*> LockPolicy::threadMutexes;

} //namespace Base::Threads
