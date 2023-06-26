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

#include <utility>

#include "Reference.h"

#include "PathComponent.h"
#include "PathSimpleComponent.h"
#include "PathArrayComponent.h"
#include "PathMapComponent.h"

namespace App::Accessor
{

template<typename TT>
void PathComponent::setName(TT&& name)
{
    name(std::forward(name));
}

void PathComponent::setRoot(Reference* resolved_reference)
{
    xxxx;
}

template<typename ReferenceType>
ReferenceType* PathComponent::getRoot() const
{
    xxxx;
    // dynamic_cast.
}


PathComponentIterator PathSimpleComponent::begin() const
{
    xxx;
}

PathComponentIterator PathSimpleComponent::end() const
{
    xxx;
}


PathArrayComponent::PathArrayComponent(std::string& name,
                   std::unique_ptr<NumberExpression>&& begin,
                   std::unique_ptr<NumberExpression>&& end,
                   std::unique_ptr<NumberExpression>&& step)
    : name(name)
    , begin(begin)
    , end(end)
    , step(step)
{
}

PathComponentIterator PathArrayComponent::begin() const
{
    xxx;
}

PathComponentIterator PathArrayComponent::end() const
{
    xxx;
}


PathMapComponent::PathMapComponent(std::string& name,
                                   std::vector<std::unique_ptr<StringExpression>>&& keys)
{
    xxx;
}

PathComponentIterator PathMapComponent::begin() const
{
    xxx;
}

PathComponentIterator PathMapComponent::end() const
{
    xxx;
}

} // namespace App::Accessor
