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

#include "PreCompiled.h"

#ifndef _PreComp_
#include <memory>
#endif // _PreComp_

#include <cmath>
#include <iostream>

#include <Base/Writer.h>
#include <Base/Exception.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryCircle.h"

namespace NamedSketcher
{

GeometryCircle::GeometryCircle(std::unique_ptr<Part::GeomCircle>&& geo)
    : GeometryBaseT(std::move(geo))
    , center(geometry->getLocation())
    , radius(geometry->getRadius())
{
}

void GeometryCircle::commitChanges() const
{
    geometry->setLocation(center);
    geometry->setRadius(radius);
}


unsigned int GeometryCircle::getMemSize () const
{
    return geometry->getMemSize() + sizeof(*this);
}

GCS::Parameter* GeometryCircle::resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Parameter*)
{
    assert(start != end);

    if(*start == "radius")
    {
        ++start;
        return &radius;
    }

    token_iterator pos = start;
    GCS::Point* result = nullptr;
    if(*pos == "center")
    {
        ++pos;
        result = &center;
    }
    else
    {
        return nullptr;
    }

    if(pos == end)
    {
        // TODO: warning. Did you mean to get a point?
        return nullptr;
    }

    if(*pos == "x")
    {
        start = ++pos;
        return &result->x;
    }
    if(*pos == "y")
    {
        start = ++pos;
        return &result->y;
    }
    return nullptr;
}

GCS::Point* GeometryCircle::resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Point*)
{
    assert(start != end);

    if(*start == "center")
    {
        ++start;
        return &center;
    }

    return nullptr;
}

GCS::Point GeometryCircle::positionAtParameter(const GCS::ParameterValueMapper& _, const GCS::Parameter* t) const
{
    double x = _(center.x) + _(radius) * std::cos(_(t));
    double y = _(center.y) + _(radius) * std::sin(_(t));
    return GCS::Point{x,y};
}

GCS::Point GeometryCircle::normalAtParameter(const GCS::ParameterValueMapper& _, const GCS::Parameter* t) const
{
    double x = std::cos(_(t));
    double y = std::sin(_(t));
    return GCS::Point{x,y};
}

void GeometryCircle::report() const
{
    std::cout << "(" << this << ") Circle: center (" << &center << ")";
    std::cout << "(" << center.x << ", " << center.y << ")";
    std::cout << ", radius (" << &radius << ") = " << radius;
    std::cout << std::endl;
}

} // namespace NamedSketcher
