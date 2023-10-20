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

#ifndef BASE_Threads_record_items_H
#define BASE_Threads_record_items_H

#include <utility>
#include <type_traits>
#include <memory>

#include "../AtomicSharedPtr.h"

namespace Base::Threads
{

/*
 * Types in the MultiIndexMap are reduced to a "type",
 * and this is "type" is actuall indexed.
 * Anything that reduces to the same "type" can be used for look up.
 * For example,
 * a shared_ptr<DocumentObject> can be searched for using another
 * shared_ptr<DocumentObject> or a DocumentObject "raw pointer".
 */

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

template<typename T>
struct ReduceToRaw
    : ReduceToRawAux<std::remove_cv_t<T>>
{};


/*
 * Index traits.
~ */
template<bool equal, std::size_t I, typename V, typename V1, typename ...Vn>
struct IndexFromTypeAux
    : IndexFromTypeAux<
          (I > sizeof...(Vn))
              ||
              (
                  std::is_reference_v<V1>
                  &&
                  std::is_same_v<std::remove_cv_t<std::remove_reference_t<V>>,std::remove_cv_t<std::remove_reference_t<V>>>
              )
              ||
              (
                  std::is_pointer_v<V1>
                  &&
                  std::is_same_v<std::remove_cv_t<std::remove_pointer_t<V>>,std::remove_cv_t<std::remove_pointer_t<V>>>
              )
          , I+1, V, Vn..., V1>
{};

template<std::size_t I, typename V, typename V1, typename ...Vn>
struct IndexFromTypeAux<true, I, V, V1, Vn...>
{
    static_assert(I-1 <= sizeof...(Vn), "Index not found.");
    static constexpr std::size_t value = I-1;
};

template<typename V, typename ...Vn>
struct IndexFromType
{
    static constexpr auto value = IndexFromTypeAux<false, 0, std::remove_cv_t<V>, std::remove_cv_t<Vn>...>::value;
};

template<typename X, typename ...Vn>
struct IndexFromRaw
{
    static constexpr auto value = IndexFromTypeAux<
        false, 0,
        typename ReduceToRaw<std::remove_cv_t<X>>::type,
        typename ReduceToRaw<std::remove_cv_t<Vn>>::type...
    >::value;
};

template<std::size_t I, typename V1, typename ...Vn>
struct TypeFromIndex : TypeFromIndex<I-1, Vn...> {};
template<typename V1, typename ...Vn>
struct TypeFromIndex<0, V1, Vn...>
{
    using type = V1;
};

template<std::size_t I, typename V1, typename ...Vn>
struct RawFromIndex
{
    using type = typename ReduceToRaw<TypeFromIndex<I, V1, Vn...>>::type;
};

template<std::size_t I, auto Val1, auto ...ValN>
struct ValueAtIndex : ValueAtIndex<I-1, ValN...> {};
template<auto Val1, auto ...ValN>
struct ValueAtIndex<0, Val1, ValN...>
{
    static constexpr auto value = Val1;
};
template<std::size_t I, auto ...ValN>
static constexpr auto ValueAtIndex_v = ValueAtIndex<I, ValN...>::value;

} //namespace ::Threads

#endif // BASE_Threads_record_items_H
