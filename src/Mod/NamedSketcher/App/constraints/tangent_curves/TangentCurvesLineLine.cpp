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

#include "TangentCurvesLineLine.h"

namespace NamedSketcher::Specialization
{

TangentCurvesLineLine::TangentCurvesLineLine(
    GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2,
    GeometryLineSegment* l1, GeometryLineSegment* l2)
    : line1(l1)
    , line2(l2)
{
    proxy1.set(&equationColinearPoints1);
    proxy2.set(&equationColinearPoints2);
}

void TangentCurvesLineLine::preprocessParameters()
{
}

void TangentCurvesLineLine::setEquations()
{
    equationColinearPoints1.set(&line1->start, &line1->end, &line2->start);
    equationColinearPoints1.set(&line1->start, &line1->end, &line2->end);
}


void TangentCurvesLineLine::report() const
{
    std::cerr << "Line to line tangent curves: " << std::endl;
    std::cerr << "* ";
    line1->report();
    std::cerr << "* ";
    line2->report();
}

} // namespace NamedSketcher
