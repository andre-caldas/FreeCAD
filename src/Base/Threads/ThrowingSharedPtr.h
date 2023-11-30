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

#ifndef BASE_Threads_ThrowingSharedPtr_H
#define BASE_Threads_ThrowingSharedPtr_H

#include <memory>

namespace Base::Threads
{

/**
 * @brief Shall be used as the return value of a function
 * instead of a regular std::shared_ptr,
 * so the programmer (may, but) does not need to check
 * for the pointer validity.
 *
 * @example
 * // If you are sure the pointer is valid...
 * getPointer()->doStuff();  // Throws if the pointer is invalid.
 *
 * // Otherwise...
 * auto ptr = getPointer();
 * if(!ptr) {
 *     return;
 * }
 * ptr->doStuff();
 */
template<typename T>
class ThrowingSharedPtr: private std::shared_ptr<T>
{
public:
    using value_type = T;
    using std::shared_ptr<T>::shared_ptr;
    ThrowingSharedPtr(const std::shared_ptr<T>& shared);
    ThrowingSharedPtr(std::shared_ptr<T>&& shared);

    constexpr T* operator->();
    constexpr T& operator*() &;

    using std::shared_ptr<T>::get;
    using std::shared_ptr<T>::operator bool;

    operator std::shared_ptr<T>() const;
};

}  // namespace Base::Threads

#include "ThrowingSharedPtr.inl"

#endif  // BASE_Threads_ThrowingSharedPtr_H
