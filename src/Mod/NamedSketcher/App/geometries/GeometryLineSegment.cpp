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

#include <iostream>

#include <Base/Writer.h>
#include <Base/Exception.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryLineSegment.h"

namespace NamedSketcher
{

GeometryLineSegment::GeometryLineSegment(std::unique_ptr<Part::GeomLineSegment>&& geo)
    : GeometryBaseT(std::move(geo))
    , start("start", geometry->getStartPoint())
    , end("end", geometry->getEndPoint())
{
}

void GeometryLineSegment::commitChanges() const
{
    geometry->setPoints(start, end);
}


unsigned int GeometryLineSegment::getMemSize () const
{
    return geometry->getMemSize() + sizeof(*this);
}

GCS::Parameter* GeometryLineSegment::resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Parameter*)
{
    assert(start != end);
    token_iterator pos = start;

    GCS::Point* result = nullptr;
    if(*pos == "start")
    {
        ++pos;
        result = &this->start;
    }
    else if(*pos == "end")
    {
        ++pos;
        result = &this->end;
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

GCS::Point* GeometryLineSegment::resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Point*)
{
    assert(start != end);

    if(*start == "start")
    {
        ++start;
        return &this->start;
    }
    if(*start == "end")
    {
        ++start;
        return &this->end;
    }

    return nullptr;
}

GCS::Point GeometryLineSegment::positionAtParameter(const GCS::ParameterValueMapper& _, const GCS::Parameter* t) const
{
    double x = _(t) * _(start.x) + (1-_(t)) * _(end.x);
    double y = _(t) * _(start.y) + (1-_(t)) * _(end.y);
    return GCS::Point{x,y};
}

GCS::Point GeometryLineSegment::normalAtParameter(const GCS::ParameterValueMapper& _, const GCS::Parameter* /*t*/) const
{
    double x = _(start.x) + _(end.x);
    double y = _(start.y) + _(end.y);
    // Rotate clockwise.
    return GCS::Point{y,-x}.normalize();
}

void GeometryLineSegment::report() const
{
    std::cout << "Line segment: ";
    std::cout << start;
    std::cout << " --> ";
    std::cout << end;
    std::cout << std::endl;
}

} // namespace NamedSketcher
