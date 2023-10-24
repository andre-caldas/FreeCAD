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

#ifndef BASE_Threads_ReduceToRaw_H
#define BASE_Threads_ReduceToRaw_H

#include <utility>
#include <type_traits>
#include <memory>
#include <string>

#include "../AtomicSharedPtr.h"

namespace Base::Threads
{

namespace rtr_detail {
template<typename T>
struct ReduceToRawAux
{
    static_assert(!std::is_class_v<T>, "Need a rule to reduce this class.");
    using from_type = T;
    using type = T;
    static type reduce(from_type val) {return val;}
};

template<typename T>
struct ReduceToRawAux<T&>
    : ReduceToRawAux<std::remove_cv_t<T>>
{};

template<typename T>
struct ReduceToRawAux<T*>
{
    using from_type = const T*;
    using type = const T*;
    static type reduce(from_type ptr) {return ptr;}
};

class StringHash;
template<>
struct ReduceToRawAux<std::string>
{
    using from_type = std::string;
    using type = const StringHash*;
    static type reduce(const from_type& str) {return (const StringHash*)std::hash<std::string>{}(str);}
};

template<>
struct ReduceToRawAux<const char*>
{
    using from_type = const char*;
    using next_type = std::string;
    using type = typename ReduceToRawAux<next_type>::type;
    static type reduce(const char* str) {return ReduceToRawAux<next_type>::reduce(std::string(str));}
};

template<typename T>
struct ReduceToRawAux<std::shared_ptr<T>>
{
    using from_type = std::shared_ptr<T>;
    using next_type = std::remove_cv_t<T>*;
    using type = typename ReduceToRawAux<next_type>::type;
    static type reduce(const from_type& ptr) {return ReduceToRawAux<next_type>::reduce(ptr.get());}
};

template<typename T>
struct ReduceToRawAux<std::unique_ptr<T>>
{
    using from_type = std::unique_ptr<T>;
    using next_type = std::remove_cv_t<T>*;
    using type = typename ReduceToRawAux<next_type>::type;
    static type reduce(const from_type& ptr) {return ReduceToRawAux<next_type>::reduce(ptr.get());}
};

template<typename T>
struct ReduceToRawAux<AtomicSharedPtr<T>>
{
    using from_type = AtomicSharedPtr<T>;
    using next_type = std::shared_ptr<T>;
    using type = typename ReduceToRawAux<next_type>::type;
    static type reduce(const from_type& ptr) {return ReduceToRawAux<next_type>::reduce(ptr.load());}
};

} //namespace rtr_detail

/**
 * @brief Types in the MultiIndexMap are reduced to a "common type".
 * For example, we treat std::string and const char* as equivalent,
 * because they reduce to the same type.
 *
 * Anything that reduces to the same "type" can be used for look up.
 * For example,
 * a shared_ptr<DocumentObject> can be searched for using another
 * shared_ptr<DocumentObject> or a DocumentObject "raw pointer".
 *
 * The reduced value is the actual key that is indexed in the internal containers.
 */
template<typename T>
struct ReduceToRaw
    : rtr_detail::ReduceToRawAux<std::remove_cv_t<T>>
{};

} //namespace Base::Threads

#endif // BASE_Threads_ReduceToRaw_H
