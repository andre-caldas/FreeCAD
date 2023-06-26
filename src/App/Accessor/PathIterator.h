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

#ifndef APP_Accessor_PathIterator_H
#define APP_Accessor_PathIterator_H

#include <iterator>
#include <stack>

#include <FCGlobal.h>

namespace App::Accessor
{

class Path;
class Reference;
class PathComponentIterator;

/*!
 * \brief xxx.
 */
class AppExport PathIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = void;
    using value_type = Reference;
    using reference = Reference&;
    using pointer = Reference*;

    PathIterator(const Path& path, bool end = false);

    PathIterator& operator++();
    PathIterator operator++(int);
    bool operator==(const PathIterator& other) const;
    bool operator!=(const PathIterator& other) const {return !(*this == other);}
    Reference& operator*() const;
    Reference* operator->() const;

private:
    const Path& path;
    std::stack<PathComponentIterator> iterator_stack;

    void completeIteratorChain();
};

} //namespace App::Accessor

#endif // APP_Accessor_PathIterator_H
