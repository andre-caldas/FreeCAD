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

#ifndef BASE_Threads_IndexTraits_H
#define BASE_Threads_IndexTraits_H

#include <type_traits>

#include "ReduceToRaw.h"

namespace Base::Threads
{

namespace idx_detail
{

template<bool equal, std::size_t I, auto V, auto V1, auto... Vn>
struct IndexFromLocalPointerAux
    : IndexFromLocalPointerAux<(I > sizeof...(Vn)) || (V = V1), I + 1, V, Vn..., V1>
{
};

template<std::size_t I, auto V, auto V1, auto... Vn>
struct IndexFromLocalPointerAux<true, I, V, V1, Vn...>
{
    static_assert(I - 1 <= sizeof...(Vn), "Index not found.");
    static constexpr std::size_t value = I - 1;
};


template<bool equal, std::size_t I, typename V, typename V1, typename... Vn>
struct IndexFromTypeAux
    : IndexFromTypeAux<(I > sizeof...(Vn)) || std::is_same_v<V, V1>, I + 1, V, Vn..., V1>
{
};

template<std::size_t I, typename V, typename V1, typename... Vn>
struct IndexFromTypeAux<true, I, V, V1, Vn...>
{
    static_assert(I - 1 <= sizeof...(Vn), "Index not found.");
    static constexpr std::size_t value = I - 1;
};

}  // namespace idx_detail


/**
 * @brief Informs the first "j" for which
 * V is the "same" type as Vj.
 */
template<typename V, typename... Vn>
struct IndexFromType
{
    static constexpr auto value =
        idx_detail::IndexFromTypeAux<false,
                                     0,
                                     std::remove_cv_t<std::remove_reference_t<V>>,
                                     std::remove_cv_t<std::remove_reference_t<Vn>>...>::value;
};

/**
 * @brief Informs the first "j" for which
 * V is reduced to the same type as Vj is.
 */
template<typename X, typename... Vn>
struct IndexFromRaw
{
    static constexpr auto value =
        idx_detail::IndexFromTypeAux<false,
                                     0,
                                     typename ReduceToRaw<X>::type,
                                     typename ReduceToRaw<Vn>::type...>::value;
};

/**
 * @brief Informs the first "j" for which
 * V is equal to Vj.
 */
template<auto X, auto... Vn>
struct IndexFromLocalPointer
{
    static constexpr auto value = idx_detail::IndexFromLocalPointerAux<false, 0, X, Vn...>::value;
};


/**
 * @brief For a given index "j", iforms which type is Vj.
 */
template<std::size_t I, typename V1, typename... Vn>
struct TypeFromIndex: TypeFromIndex<I - 1, Vn...>
{
};
template<typename V1, typename... Vn>
struct TypeFromIndex<0, V1, Vn...>
{
    using type = V1;
};


/**
 * @brief For a given index "j", iforms to which type Vj reduces.
 */
template<std::size_t I, typename V1, typename... Vn>
struct RawFromIndex
{
    using type = typename ReduceToRaw<TypeFromIndex<I, V1, Vn...>>::type;
};


/**
 * @brief For a given index "j", informs the value Valj.
 */
template<std::size_t I, auto Val1, auto... ValN>
struct ValueAtIndex: ValueAtIndex<I - 1, ValN...>
{
};
template<auto Val1, auto... ValN>
struct ValueAtIndex<0, Val1, ValN...>
{
    static constexpr auto value = Val1;
};
/**
 * @brief The value Valj for a given index "j".
 */
template<std::size_t I, auto... ValN>
static constexpr auto ValueAtIndex_v = ValueAtIndex<I, ValN...>::value;

}  // namespace Base::Threads

#endif  // BASE_Threads_IndexTraits_H
