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

#ifndef BASE_Threads_Traits_Utils_H
#define BASE_Threads_Traits_Utils_H

#include <type_traits>

#include "IndexTraits.h"

namespace Base::Threads
{

/**
 * @brief Used to unpack packed template types.
 * @see ForEach_t.
 */
template<typename Result, typename From>
struct ForEach {using type = Result;};
/**
 * @brief When you have template packed types Fn,
 * you can use @class ForEach to replace each Fn by some
 * constant type Result:
 * ForEach_t<std::string, Fn>...
 */
template<typename Result, typename From>
using ForEach_t = typename ForEach<Result, From>::type;


namespace utils_detail {
// https://stackoverflow.com/a/72263473/1290711
template<typename T>
struct MemberPointerToAux;
template<class C, typename T>
struct MemberPointerToAux<T C::*>
{
    using type = T;
};
} //namespace: utils_detail

/**
 * @brief Given a "member pointer" T that points to a member of X,
 * informs which type is X.
 */
template<auto T>
struct MemberPointerTo
    : utils_detail::MemberPointerToAux<std::remove_cv_t<decltype(T)>>
{};
template<auto T>
/**
 * @brief Given a "member pointer" T that points to a member of X,
 * the type X.
 */
using MemberPointerTo_t = typename MemberPointerTo<T>::type;

} //namespace Base::Threads

#endif // BASE_Threads_Traits_Utils_H
