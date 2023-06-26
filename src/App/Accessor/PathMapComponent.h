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

#ifndef APP_Accessor_PathMapComponent_H
#define APP_Accessor_PathMapComponent_H

namespace App {
//class StringExpression;
}

#include <memory>
#include <string>

#include <FCGlobal.h>

// The user shall not be required to include the header for the iterator.
#include "PathMapComponentIterator.h"
#include "PathComponent.h"

namespace App::Accessor
{

// Temporary while I do not know how to use NumberExpression.
using StringExpression = std::string;
class PathMapComponentIterator;
class MapReference;

/*!
 * \brief xxx.
 */
class AppExport PathMapComponent : public PathComponent
{
public:
    using iterator = PathMapComponentIterator;
    friend class iterator;

    PathMapComponent(std::string& name,
                     std::vector<std::unique_ptr<StringExpression>>&& keys);

    PathComponentIterator begin() const override;
    PathComponentIterator end() const override;

protected:
    using getAccessor = PathComponent::getRoot<MapReference>;
};

} //namespace App::Accessor


#endif // APP_Accessor_PathMapComponent_H
