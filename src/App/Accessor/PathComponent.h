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

#ifndef APP_Accessor_PathComponent_H
#define APP_Accessor_PathComponent_H

#include <string>

#include <FCGlobal.h>

// The user shall not be required to include the header for the iterator.
#include "PathComponentIterator.h"

namespace App::Accessor
{

class Accessor;

/*!
 * \brief xxx.
 * \attention The \class PathComponent has iterators,
 * but it is not iterable, because the \class PathComponent
 * is not aware of how the previous \class PathComponent resolves.
 * It can be iterated, but the iterator cannot be referenced: *it or it->m.
 * The \class Path, however, can be iterated.
 */
class AppExport PathComponent
{
public:
    using iterator = PathComponentIterator;
    friend class iterator;

    PathComponent(const std::string& name) : name(name) {}
    PathComponent(std::string&& name) : name(name) {}

    template<typename TT>
    void setName(TT&& name);
    const std::string& getName() const {return name;}

    virtual PathComponentIterator begin(Accessor& xxx = nullptr) const = 0;
    virtual PathComponentIterator end() const = 0;

private:
    std::string name;
};

} //namespace App::Accessor


#endif // APP_Accessor_PathComponent_H
