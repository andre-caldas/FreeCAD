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

#ifndef BASE_Threads_IsIterator_H
#define BASE_Threads_IsIterator_H

#include <type_traits>
#include <iterator>

namespace Base::Threads
{

/*
 * See:
 * https://stackoverflow.com/questions/25290462/how-to-define-is-iterator-type-trait
 */

template<typename T, typename Enable = void>
struct IsIterator : std::false_type {};

template<typename T>
struct IsIterator<
    T,
    std::enable_if_t<
        std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category>
    >
> : std::true_type {};

} //namespace ::Threads

#endif // BASE_Threads_type_traits_H
