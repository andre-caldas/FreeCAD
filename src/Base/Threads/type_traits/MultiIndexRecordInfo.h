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

#ifndef BASE_Threads_MultiIndexRecordInfo_H
#define BASE_Threads_MultiIndexRecordInfo_H

#include <type_traits>
#include <map>
#include <unordered_map>

#include "Utils.h"
#include "IndexTraits.h"
#include "ReduceToRaw.h"

namespace Base::Threads
{

template<typename... Vn>
struct TypesInfo
{
    template<std::size_t I>
    using type_from_index_t = typename TypeFromIndex<I, Vn...>::type;
    template<std::size_t I>
    using raw_from_index_t = typename RawFromIndex<I, Vn...>::type;

    template<typename X>
    static constexpr auto index_from_raw_v = IndexFromRaw<X, Vn...>::value;
    template<typename X>
    static constexpr auto index_from_type_v = IndexFromType<X, Vn...>::value;
};


/**
 * @brief Collects the information on "multi index records".
 * Pass a struct Record and the member pointers to be indexed.
 */
template<typename Record, auto... LocalPointers>
struct MultiIndexRecordInfo
{
    static constexpr auto tuple_size = sizeof...(LocalPointers);

    using element_t = Record;

    using types_info_t = TypesInfo<MemberPointerTo_t<LocalPointers>...>;

    template<std::size_t I>
    static constexpr auto pointer_v = ValueAtIndex_v<I, LocalPointers...>;

    template<typename X>
    static constexpr auto index_from_raw_v = types_info_t::template index_from_raw_v<X>;
    template<typename V>
    static constexpr auto index_from_type_v = types_info_t::template index_from_raw_v<V>;
    template<auto V>
    static constexpr auto index_from_local_pointer_v =
        types_info_t::template index_from_local_pointer_v<V>;

    template<std::size_t I>
    using raw_from_index_t = typename types_info_t::template raw_from_index_t<I>;
    template<std::size_t I>
    using type_from_index_t = typename types_info_t::template type_from_index_t<I>;

    using indexes_tuple_t =
        std::tuple<std::multimap<typename ReduceToRaw<MemberPointerTo_t<LocalPointers>>::type,
                                 const Record*>...>;
};

}  // namespace Base::Threads

#endif  // BASE_Threads_type_traits_H
