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

#include "../../geometries/GeometryLineSegment.h"
#include "../../geometries/GeometryCircle.h"

#include "../../gcs_solver/equations/EquationProxy.h"

#include "TangentCurvesLineCircle.h"

namespace NamedSketcher::Specialization
{

TangentCurvesLineCircle::TangentCurvesLineCircle(
    GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2,
    GeometryLineSegment* line, GeometryCircle* circle, bool right_side)
    : line(line)
    , circle(circle)
    , right_side(right_side)
{
    proxy1.set(&equation);
    proxy2.reset();
}

TangentCurvesLineCircle::TangentCurvesLineCircle(
    GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2,
    GeometryCircle* circle, GeometryLineSegment* line, bool right_side)
    : TangentCurvesLineCircle(proxy1, proxy2, line, circle, right_side)
{
}

void TangentCurvesLineCircle::preprocessParameters()
{
}

void TangentCurvesLineCircle::setEquations()
{
    if(right_side)
    {
        equation.set(&line->start, &line->end, &circle->center, &circle->radius);
    } else {
        equation.set(&line->end, &line->start, &circle->center, &circle->radius);
    }
}


void TangentCurvesLineCircle::report() const
{
    std::cerr << "Line to circle tangent curves: " << std::endl;
    std::cerr << "* ";
    line->report();
    std::cerr << "* ";
    circle->report();
}

} // namespace NamedSketcher
