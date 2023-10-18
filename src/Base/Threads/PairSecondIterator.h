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

#ifndef BASE_Threads_PairSecondIterator_H
#define BASE_Threads_PairSecondIterator_H

#include <utility>

namespace Base::Threads
{

template<typename ItType>
class PairSecondIterator
{
public:
    using difference_type = typename ItType::difference_type;
    using value_type = typename ItType::value_type;
    using pointer = typename ItType::pointer;
    using reference = typename ItType::reference;
    using iterator_category = typename ItType::iterator_category;

    PairSecondIterator(ItType&& it) : it(it) {}

    /*
     * CPP loves when we do all that by hand... :-(
     */
    template<typename OtherIt>
    constexpr bool operator==(const OtherIt& other) const {return it == other.it;}
    template<typename OtherIt>
    constexpr bool operator!=(const OtherIt& other) const {return it != other.it;}
    auto& operator++() {++it; return *this;}
    auto operator++(int) {PairSecondIterator result(this->it); ++it; return result;}
    auto& operator*() const {return it->second;}
    auto* operator->() const {return &it->second;}

private:
    ItType it;
};

} //namespace ::Threads

#endif // BASE_Threads_PairSecondIterator_H
