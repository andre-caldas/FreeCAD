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
#include <Base/Vector3D.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryPoint.h"

namespace NamedSketcher
{

GeometryPoint::GeometryPoint(std::unique_ptr<Part::GeomPoint>&& geo)
    : GeometryBaseT(std::move(geo))
    , point("point", geometry->getPoint())
{
}

// Unfortunately, GeomPoint does not have a construction for (x,y,z). :-(
GeometryPoint::GeometryPoint(double x, double y)
    : GeometryBaseT(std::make_shared<Part::GeomPoint>(Base::Vector3d(x,y,0)))
    , point("point", x, y)
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


std::vector<Base::Accessor::ReferenceTo<GCS::Parameter>>
GeometryPoint::getReferences(GCS::Parameter*)
{
    using ref = Base::Accessor::ReferenceTo<GCS::Parameter>;
    return {
        ref(this, "point", "x"),
        ref(this, "point", "y")
    };
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

std::vector<Base::Accessor::ReferenceTo<GCS::Point>>
GeometryPoint::getReferences(GCS::Point*)
{
    using ref = Base::Accessor::ReferenceTo<GCS::Point>;
    return {
        ref(this, "point")
    };
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

GCS::Point GeometryPoint::positionAtParameter(const GCS::ParameterValueMapper& value_mapper, const GCS::Parameter* /*t*/) const
{
    return {value_mapper(&point.x), value_mapper(&point.x)};
}

GCS::Point GeometryPoint::normalAtParameter(const GCS::ParameterValueMapper& /*value_mapper*/, const GCS::Parameter* /*t*/) const
{
    return GCS::Point{0,0};
}

void GeometryPoint::report() const
{
    std::cout << "Point: ";
    std::cout << point;
    std::cout << std::endl;
}

} // namespace NamedSketcher
