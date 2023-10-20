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

#include <utility>

namespace Base::Threads
{

template<typename ItType, typename GetReference>
class IteratorWrapper
{
public:
    using difference_type = typename ItType::difference_type;
    using value_type = typename ItType::value_type;
    using pointer = typename ItType::pointer;
    using reference = typename ItType::reference;
    using iterator_category = typename ItType::iterator_category;

    IteratorWrapper(const ItType& it) : it(it) {}
    IteratorWrapper(IteratorWrapper&& other) : it(std::move(other.it)) {}
    IteratorWrapper(const IteratorWrapper& other) : it(other.it) {}

    /*
     * C++ loves when we do all that by hand... :-(
     */
    constexpr bool operator==(const IteratorWrapper& other) const {return it == other.it;}
    constexpr bool operator!=(const IteratorWrapper& other) const {return it != other.it;}
    auto& operator=(const IteratorWrapper& other) {it = other.it; return *this;}
    auto& operator++() {++it; return *this;}
    auto operator++(int) {IteratorWrapper result(this->it); ++it; return result;}
    auto& operator*() const {return GetReference{it}();}
    auto* operator->() const {return &GetReference{it}();}

private:
    ItType it;
};

namespace detail {
template<typename ItType>
struct GetSecond
{
    GetSecond(const ItType& it) : it(it) {}
    auto& operator()() {return it->second;}
    const ItType& it;
};

template<typename ItType>
struct GetSecondPointerAsReference
{
    GetSecondPointerAsReference(const ItType& it) : it(it) {}
    auto& operator()() {return *it->second;}
    const ItType& it;
};
}

template<typename ItType>
using IteratorSecond = IteratorWrapper<ItType, detail::GetSecond<ItType>>;

template<typename ItType>
using IteratorSecondPtrAsRef = IteratorWrapper<ItType, detail::GetSecondPointerAsReference<ItType>>;

} //namespace ::Threads

#endif // BASE_Threads_IteratorWrapper_H
