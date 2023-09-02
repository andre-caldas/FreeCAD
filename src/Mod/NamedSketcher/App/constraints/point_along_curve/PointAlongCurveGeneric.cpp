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
#include <iostream>
#endif // _PreComp_
#include <cmath>

#include "../../geometries/GeometryBase.h"
#include "../../gcs_solver/equations/EquationProxy.h"

#include "PointAlongCurveGeneric.h"

namespace NamedSketcher::Specialization
{

PointAlongCurveGeneric::PointAlongCurveGeneric(
    GCS::EquationProxy& proxy,
    GCS::Point* point, GeometryBase* curve,
    GCS::Parameter* parameter_t)
    : point(point)
    , curve(curve)
    , parameter_t(parameter_t)
{
    proxy.set(&equation);
}

void PointAlongCurveGeneric::preprocessParameters()
{
    // TODO: improve this search.
    double min_dist = 100000000.;
    auto& pt1 = *point;
    for(GCS::Parameter p(0); p <= 1.0; p += 0x1p-4)
    {
        auto pt2 = curve->positionAtParameter({}, &p);
        double x = pt1.x - pt2.x;
        double y = pt1.y - pt2.y;
        double dist = x*x + y*y;
        if(dist <= min_dist)
        {
            min_dist = dist;
            *parameter_t = p;
        }
    }
}

void PointAlongCurveGeneric::setEquations()
{
    equation.set(point, curve, parameter_t);
}


void PointAlongCurveGeneric::report() const
{
    auto pt_curve = curve->positionAtParameter({}, parameter_t);

    std::cerr << "Point along curve (generic): ";
    std::cerr << "candidate point " << *point;
    std::cerr << " <-" << parameter_t << "-> ";
    std::cerr << "curve(t) " << pt_curve;
    std::cerr << std::endl;

    double diff_x = (point->x - pt_curve.x);
    double diff_y = (point->y - pt_curve.y);
    std::cerr << "\tError: (";
    std::cerr << std::sqrt(diff_x*diff_x + diff_y*diff_y);
    std::cerr << ")" << std::endl;
}

} // namespace NamedSketcher
