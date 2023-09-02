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
#include <Base/Vector3D.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryCircle.h"

// A shame we had to wait C++20 for std::numeric::pi. :-(
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

namespace NamedSketcher
{

GeometryCircle::GeometryCircle(std::unique_ptr<Part::GeomCircle>&& geo)
    : GeometryBaseT(std::move(geo))
    , center("center", geometry->getLocation())
    , radius("radius", geometry->getRadius())
{
}

// Unfortunately, GeomCircle does not have a non-default constructor. :-(
GeometryCircle::GeometryCircle(double x, double y, double r)
    : GeometryBaseT(std::make_shared<Part::GeomCircle>())
    , center("center", x, y)
    , radius("radius", std::abs(r))
{
    geometry->setLocation(Base::Vector3d(x,y,0));
    geometry->setRadius(radius);
}

void GeometryCircle::commitChanges() const
{
    geometry->setLocation(center);
    try
    {
        geometry->setRadius(std::abs(radius));
    } catch(Base::ValueError&) {
        GCS::Parameter new_radius = radius;
        new_radius += .000001;
        geometry->setRadius(new_radius);
        // Ignore when things are to small...
    }
}


unsigned int GeometryCircle::getMemSize () const
{
    return geometry->getMemSize() + sizeof(*this);
}


std::vector<Base::Accessor::ReferenceTo<GCS::Parameter>>
GeometryCircle::getReferences(GCS::Parameter*)
{
    using ref = Base::Accessor::ReferenceTo<GCS::Parameter>;
    return {
        ref(this, "radius"),
        ref(this, "center", "x"),
        ref(this, "center", "y")
    };
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

std::vector<Base::Accessor::ReferenceTo<GCS::Point>>
GeometryCircle::getReferences(GCS::Point*)
{
    using ref = Base::Accessor::ReferenceTo<GCS::Point>;
    return {
        ref(this, "center")
    };
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
    double r = _(radius);
    double _t = (2.0 * M_PI) * _(t);
    double x = _(center.x) + r * std::cos(_t);
    double y = _(center.y) + r * std::sin(_t);
    return GCS::Point{x,y};
}

GCS::Point GeometryCircle::normalAtParameter(const GCS::ParameterValueMapper& _, const GCS::Parameter* t) const
{
    double _t = (2.0 * M_PI) * _(t);
    double x = std::cos(_t);
    double y = std::sin(_t);
    return GCS::Point{x,y};
}

void GeometryCircle::partialDerivativesPoint(const GCS::ParameterValueMapper& _, derivative_map& map, const GCS::Parameter* t) const
{
    map.try_emplace(&center.x, 1, 0);
    map.try_emplace(&center.y, 0, 1);

    double r = _(radius);
    double _t = (2.0 * M_PI) * _(t);
    map.try_emplace(&radius, std::cos(_t), std::sin(_t));

    double dx = -(2.0 * M_PI) * r * std::sin(_t);
    double dy = +(2.0 * M_PI) * r * std::cos(_t);
    map.try_emplace(t, dx, dy);
}

void GeometryCircle::partialDerivativesNormal(const GCS::ParameterValueMapper& _, derivative_map& map, const GCS::Parameter* t) const
{
    map.try_emplace(&center.x, 0, 0);
    map.try_emplace(&center.y, 0, 0);

    double _t = (2.0 * M_PI) * _(t);
    map.try_emplace(&radius, std::cos(_t), std::sin(_t));

    double dx = -(2.0 * M_PI) * std::sin(_t);
    double dy = +(2.0 * M_PI) * std::cos(_t);
    map.try_emplace(t, dx, dy);
}

void GeometryCircle::report() const
{
    std::cerr << "Circle: center ";
    std::cerr << center;
    std::cerr << ", radius = " << radius;
    std::cerr << std::endl;
}

} // namespace NamedSketcher
