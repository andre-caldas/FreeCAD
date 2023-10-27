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

#ifndef BASE_Threads_IteratorWrapper_H
#define BASE_Threads_IteratorWrapper_H

#include <type_traits>
#include <utility>

namespace Base::Threads
{

template<typename ItType, typename GetReference>
class IteratorWrapper
{
public:
    using difference_type = typename ItType::difference_type;
    using iterator_category = typename ItType::iterator_category;
    using value_type = typename GetReference::value_type;
    using pointer = typename GetReference::pointer;
    using reference = typename GetReference::reference;

    IteratorWrapper(const IteratorWrapper& other) : it(other.it) {}
    IteratorWrapper(const ItType& it) : it(it) {}

    IteratorWrapper& operator=(const IteratorWrapper& other)
    {it = other.it; return *this;}
    IteratorWrapper& operator=(const ItType& other)
    {it = other; return *this;}

    constexpr bool operator==(const IteratorWrapper& other) const
    {return it == other.it;}
    constexpr bool operator!=(const IteratorWrapper& other) const
    {return it != other.it;}

    constexpr bool operator==(const ItType& other) const
    {return it == other;}
    constexpr bool operator!=(const ItType& other) const
    {return it != other;}

    IteratorWrapper& operator++()
    {++it; return *this;}
    IteratorWrapper operator++(int)
    {IteratorWrapper result(*this); ++it; return result;}

    auto& operator*() const
    {return GetReference{it}();}
    auto* operator->() const
    {return &GetReference{it}();}

    ItType& getIterator() {return it;}

private:
    ItType it;
};

namespace ReferenceGetter {

template<typename, typename ItType>
struct Identity
{
    using value_type = typename ItType::value_type;
    using pointer = typename ItType::pointer;
    using reference = typename ItType::reference;

    Identity(const ItType& it) : it(it) {}
    auto& operator()() {return *it;}
    const ItType& it;
};

template<typename ContainerType, typename ItType>
struct GetSecond
{
    using container_const_iterator = typename ContainerType::const_iterator;
    static constexpr bool is_const_v = std::is_same_v<ItType, container_const_iterator>;

    using it_value_type = typename ItType::value_type;
    using cv_value_type = std::remove_reference_t<typename it_value_type::second_type>;

    using value_type = std::remove_cv_t<cv_value_type>;
    using pointer = std::conditional<is_const_v, const value_type*, value_type*>;
    using reference = std::conditional<is_const_v, const value_type&, value_type&>;

    GetSecond(const ItType& it) : it(it) {}
    reference operator()() {return it->second;}
    const ItType& it;
};

template<typename, typename ItType>
struct GetSecondPointerAsReference
{
    using it_value_type = typename ItType::value_type;

    using pointer = typename it_value_type::second_type;
    using reference = std::remove_pointer_t<pointer>&;
    using value_type =  std::remove_cv_t<std::remove_pointer_t<pointer>>;

    GetSecondPointerAsReference(const ItType& it) : it(it) {}
    reference operator()() {return *it->second;}
    const ItType& it;
};

} //namespace: ReferenceGetter

template<typename ContainerType, typename ItType>
using IteratorSecond =
    IteratorWrapper<ItType,
                    ReferenceGetter::GetSecond<ContainerType, ItType>>;

template<typename ContainerType, typename ItType>
using IteratorSecondPtrAsRef =
    IteratorWrapper<ItType,
                    ReferenceGetter::GetSecondPointerAsReference<ContainerType, ItType>>;

} //namespace ::Threads

#endif // BASE_Threads_IteratorWrapper_H
