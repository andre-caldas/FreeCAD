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

#ifndef BASE_Threads_ThrowingOptional_H
#define BASE_Threads_ThrowingOptional_H

#include <optional>

namespace Base::Threads
{

/**
 * @brief Almost exaclty the same as std::optional.
 * The difference being that operator* and operator-> do throw
 * when an invalid data is accessed.
 */
template<typename T>
class ThrowingOptional: private std::optional<T>
{
public:
    using value_type = T;
    using std::optional<T>::optional;

    constexpr const T* operator->() const;
    constexpr T* operator->();
    constexpr const T& operator*() const&;
    constexpr T& operator*() &;
    constexpr const T&& operator*() const&&;
    constexpr T&& operator*() &&;

    using operator=;
    using operator bool;
    using emplace;
    using has_value;
    using reset;
    using swap;
    using value;
    using value_or;

    using operator==;
    using operator!=;
    using operator<;
    using operator<=;
    using operator>;
    using operator>=;
};

}  // namespace Base::Threads

#include "ThrowingOptional.inl"

#endif  // BASE_Threads_ThrowingOptional_H
