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

#include "TangentCurvesGeneric.h"

namespace NamedSketcher::Specialization
{

TangentCurvesGeneric::TangentCurvesGeneric(
    GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2,
    GeometryBase* c1, GeometryBase* c2,
    GCS::Parameter* parameter_t1, GCS::Parameter* parameter_t2)
    : curve1(c1)
    , curve2(c2)
    , parameter_t1(parameter_t1)
    , parameter_t2(parameter_t2)
{
    proxy1.set(&equationConcurrent);
    proxy2.set(&equationParallel);
}

void TangentCurvesGeneric::preprocessParameters()
{
    // TODO: improve this search.
    double min_det = 100000000.;
    for(GCS::Parameter p1(0); p1 <= 1.0; p1 += 0x1p-4)
    {
        for(GCS::Parameter p2(0); p2 <= 1.0; p2 += 0x1p-4)
        {
            auto n1 = curve1->normalAtParameter({}, &p1);
            auto n2 = curve2->normalAtParameter({}, &p2);
            double det = std::abs(n1.x*n2.y - n1.y*n2.x);
            if(det <= min_det)
            {
                min_det = det;
                *parameter_t1 = double(p1);
                *parameter_t2 = double(p2);
            }
        }
    }
}

void TangentCurvesGeneric::setEquations()
{
    equationConcurrent.set(curve1, parameter_t1, curve2, parameter_t2);
    equationParallel.set(curve1, parameter_t1, curve2, parameter_t2);
}


void TangentCurvesGeneric::report() const
{
    auto pt_curve1 = curve1->positionAtParameter({}, parameter_t1);
    auto pt_curve2 = curve2->positionAtParameter({}, parameter_t2);
    auto n1 = curve1->normalAtParameter({}, parameter_t1);
    auto n2 = curve2->normalAtParameter({}, parameter_t2);

    std::cerr << "Generic tangent curves: " << std::endl;
    std::cerr << "* Curve 1 " << *parameter_t1 << " -> " << pt_curve1 << ". ";
    std::cerr << "Normal 1 " << n1 << ".";
    std::cerr << std::endl;

    std::cerr << "* Curve 2 " << *parameter_t2 << " -> " << pt_curve2 << ". ";
    std::cerr << "Normal 2 " << n2 << ".";
    std::cerr << std::endl;
}

} // namespace NamedSketcher
