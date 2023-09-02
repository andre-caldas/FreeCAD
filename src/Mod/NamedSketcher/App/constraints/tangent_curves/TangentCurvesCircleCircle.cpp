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

#include "../../geometries/GeometryCircle.h"

#include "../../gcs_solver/equations/EquationProxy.h"

#include "TangentCurvesCircleCircle.h"

namespace NamedSketcher::Specialization
{

TangentCurvesCircleCircle::TangentCurvesCircleCircle(
    GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2,
    GeometryCircle* circle1, GeometryCircle* circle2, bool in)
    : circle1(circle1)
    , circle2(circle2)
    , in(in)
{
    proxy1.set(&equation);
    proxy2.reset();
}

void TangentCurvesCircleCircle::preprocessParameters()
{
}

void TangentCurvesCircleCircle::setEquations()
{
    if(in)
    {
        equation.set(&circle1->center, &circle2->center, {{1.0,&circle1->radius}, {-1.0,&circle2->radius}});
    } else {
        equation.set(&circle1->center, &circle2->center, {{1.0,&circle1->radius}, {+1.0,&circle2->radius}});
    }
}


void TangentCurvesCircleCircle::report() const
{
    std::cerr << "Circle to circle tangent curves: " << std::endl;
    std::cerr << "* ";
    circle1->report();
    std::cerr << "* ";
    circle2->report();
}

} // namespace NamedSketcher
