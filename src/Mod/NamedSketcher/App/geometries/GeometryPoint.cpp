/***************************************************************************
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
#include <memory>
#endif // _PreComp_

#include <iostream>

#include <Base/Writer.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryPoint.h"

namespace NamedSketcher
{

GeometryPoint::GeometryPoint(std::unique_ptr<Part::GeomPoint>&& geo)
    : GeometryBaseT(std::move(geo))
    , point(geometry->getPoint().x, geometry->getPoint().y)
{
}

void GeometryPoint::commitChanges() const
{
    geometry->setPoint(point);
}


unsigned int GeometryPoint::getMemSize() const
{
    return geometry->getMemSize() + sizeof(*this);
}

GCS::Parameter* GeometryPoint::resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Parameter*)
{
    assert(start != end);
    token_iterator pos = start;
    if(*pos == "point")
    {
        ++pos;
    }
    if(pos == end)
    {
        // TODO: warning. Did you mean to get a point?
        return nullptr;
    }

    if(*pos == "x")
    {
        start = ++pos;
        return &x;
    }
    if(*pos == "y")
    {
        start = ++pos;
        return &y;
    }
    return nullptr;
}

GCS::Point* GeometryPoint::resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Point*)
{
    assert(start != end);
    if(*start == "point")
    {
        ++start;
        return &point;
    }
    return nullptr;
}

void GeometryPoint::report() const
{
    std::cout << "Point: ";
    std::cout << "(" << point.x << ", " << point.y << ")";
    std::cout << std::endl;
}

} // namespace NamedSketcher
