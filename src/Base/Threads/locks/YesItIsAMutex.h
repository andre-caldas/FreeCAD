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

#ifndef BASE_Threads_YesItIsAMutex_H
#define BASE_Threads_YesItIsAMutex_H

#ifndef FC_GLOBAL_H
#include <FCGlobal.h>
#endif
#include <mutex>
#include <condition_variable>

namespace Base::Threads
{

/**
 * @brief Like a shared_mutex. But it is not bound to a thread.
 */
class BaseExport YesItIsAMutex
{
public:
    YesItIsAMutex() = default;
    YesItIsAMutex(const YesItIsAMutex&) = delete;
    YesItIsAMutex operator=(const YesItIsAMutex&) = delete;

    void lock();
    bool try_lock();
    void unlock();

    void lock_shared();
    bool try_lock_shared();
    void unlock_shared();

private:
    /*
     * This should be implemented with two semaphores: one binary and one counter.
     * But we are still in C++17 in FreeCAD.
     */
    std::mutex pivot;
    int shared_counter = 0;
    bool is_exclusively_locked = false;
    /**
     * @brief Locking consists of trying to lock and waiting for the condition to change.
     */
    std::condition_variable released;
    std::mutex released_condition_lock;
};

} //namespace Base::Threads

#endif // BASE_Threads_YesItIsAMutex_H
