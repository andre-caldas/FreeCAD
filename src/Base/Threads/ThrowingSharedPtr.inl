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

#include <optional>

#include "ThrowingSharedPtr.h"

namespace Base::Threads
{

template<typename T>
ThrowingSharedPtr<T>::ThrowingSharedPtr(const std::shared_ptr<T>& shared)
    : std::shared_ptr<T>(shared)
{}

template<typename T>
ThrowingSharedPtr<T>::ThrowingSharedPtr(std::shared_ptr<T>&& shared)
    : std::shared_ptr<T>(std::move(shared))
{}

template<typename T>
constexpr T* ThrowingSharedPtr<T>::operator->()
{
    if (!*this) {
        throw std::bad_optional_access {};
    }
    return std::shared_ptr<T>::operator->();
}

template<typename T>
constexpr T& ThrowingSharedPtr<T>::operator*() &
{
    if (!*this) {
        throw std::bad_optional_access {};
    }
    return std::shared_ptr<T>::operator*();
}

template<typename T>
ThrowingSharedPtr<T>::operator std::shared_ptr<T>() const
{
    if (!*this) {
        throw std::bad_optional_access {};
    }
    return std::shared_ptr<T>(*this);
}

}  // namespace Base::Threads
