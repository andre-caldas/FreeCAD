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

#include "../../gcs_solver/equations/EquationProxy.h"

#include "PointAlongCurveLine.h"

namespace NamedSketcher::Specialization
{

PointAlongCurveLine::PointAlongCurveLine(
    GCS::EquationProxy& proxy,
    GCS::Point* point, GeometryLineSegment* line)
    : point(point)
    , line(line)
{
    proxy.set(&equation);
}

void PointAlongCurveLine::preprocessParameters()
{
}

void PointAlongCurveLine::setEquations()
{
    equation.set(&line->start, &line->end, point);
}


void PointAlongCurveLine::report() const
{
    std::cerr << "Point along line: " << std::endl;
    std::cerr << "* " << *point << std::endl;
    std::cerr << "* ";
    line->report();
}

} // namespace NamedSketcher
