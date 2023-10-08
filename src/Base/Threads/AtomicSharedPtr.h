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

#ifndef BASE_Threads_AtomicSharedPtr_H
#define BASE_Threads_AtomicSharedPtr_H

#include <memory>
#include <atomic>

namespace Base::Threads
{

template<typename T>
class AtomicSharedPtr
{
public:
    AtomicSharedPtr() = default;
    AtomicSharedPtr(std::nullptr_t) : AtomicSharedPtr() {}
    AtomicSharedPtr(std::shared_ptr<T> desired) : ptr(std::move(desired)) {}

    void operator=(const AtomicSharedPtr&) = delete;
    void operator=(std::nullptr_t) {store(std::shared_ptr<T>());}
    void operator=(std::shared_ptr<T> desired) {store(std::move(desired));}

    bool is_lock_free() const noexcept
    {return std::atomic_is_lock_free(&ptr);}

    void store(std::shared_ptr<T> desired,
               std::memory_order order = std::memory_order_seq_cst)
    {std::atomic_store_explicit(&ptr, std::move(desired), order);}

    std::shared_ptr<T> load( std::memory_order /*order*/ = std::memory_order_seq_cst ) const
    {return std::atomic_load(&ptr);}

    operator std::shared_ptr<T>() const {return load();}

    std::shared_ptr<T> exchange(std::shared_ptr<T> desired,
                                std::memory_order order = std::memory_order_seq_cst)
    {return std::atomic_exchange(std::move(desired), order);}

    bool compare_exchange_strong(std::shared_ptr<T>& expected, std::shared_ptr<T> desired,
                                 std::memory_order success, std::memory_order failure)
    {return std::atomic_compare_exchange_strong_explicit(&ptr, &expected, std::move(desired), success, failure);}

    bool compare_exchange_weak(std::shared_ptr<T>& expected, std::shared_ptr<T> desired,
                               std::memory_order success, std::memory_order failure)
    {return std::atomic_compare_exchange_weak_explicit(&ptr, &expected, std::move(desired), success, failure);}

    /// @attention We are ignoring "memory order"
    bool compare_exchange_strong(std::shared_ptr<T>& expected, std::shared_ptr<T> desired,
                                 std::memory_order /*order*/ = std::memory_order_seq_cst)
    {return std::atomic_compare_exchange_strong(&ptr, &expected, std::move(desired));}

    /// @attention We are ignoring "memory order"
    bool compare_exchange_weak(std::shared_ptr<T>& expected, std::shared_ptr<T> desired,
                               std::memory_order /*order*/ = std::memory_order_seq_cst)
    {return std::atomic_compare_exchange_weak(&ptr, &expected, std::move(desired));}

    /**
     * @brief Compares and if stored shared_ptr is equal to @a expected,
     * sets the stored to @a desired.
     * @param expected - Stores only if equal to @a expected.
     * @param desired - Value to store when condition is met.
     * @return True if condition was met and @a desired was stored. False otherwise.
     */
    bool compare_store(std::shared_ptr<T> expected, std::shared_ptr<T> desired)
    {return compare_exchange_strong(expected, std::move(desired));}

    /* Missing wait and notify_xxx */

private:
#if __cplusplus > 202000L
    std::atomic<std::shared_ptr<T>> ptr;
#else
    std::shared_ptr<T> ptr;
#endif

    static constexpr bool is_always_lock_free = false;
};

} //namespace Base::Threads

#endif // BASE_Threads_AtomicSharedPtr_H
