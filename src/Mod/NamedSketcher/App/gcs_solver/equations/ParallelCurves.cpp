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

#include <cmath>

#include <Base/Exception.h>

#include "../parameters/ParameterGroupManager.h"
#include "../parameters/ParameterValueMapper.h"
#include "../../geometries/GeometryBase.h"
#include "ParallelCurves.h"

namespace NamedSketcher::GCS
{

void ParallelCurves::set(GeometryBase* c1, Parameter* t1, GeometryBase* c2, Parameter* t2)
{
    curve1 = c1;
    curve2 = c2;
    parameter_t1 = t1;
    parameter_t2 = t2;
}

// sqrt(det(normal1(t), normal2(t)))
double ParallelCurves::error(const ParameterGroupManager& manager) const
{
    auto n1 = curve1->normalAtParameter(manager, parameter_t1);
    auto n2 = curve2->normalAtParameter(manager, parameter_t2);
    double result = std::sqrt(n1.x*n2.y - n1.y*n2.x);
    return result;
}

ParameterVector ParallelCurves::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    /*
     * Here we calculate the partial derivatives of sqrt(det(normal1(t), normal2(t))).
     */
    auto n1 = curve1->normalAtParameter(_, parameter_t1);
    auto n2 = curve2->normalAtParameter(_, parameter_t2);

    double denominator = std::sqrt(n1.x*n2.y - n1.y*n2.x);

    double v1x = n2.y;
    double v1y = -n2.x;
    double v2x = -n1.y;
    double v2y = n1.x;
    if(denominator != 0)
    {
        v1x /= denominator;
        v1y /= denominator;
        v2x /= denominator;
        v2y /= denominator;
    }

    ParameterVector result;

    /*
     * For the curve parameter h, the derivative is:
     * +(dc1/dh) dot_product (c1(t) - c2(t)) / ||c1(t) - c2(t)||.
     * -(dc2/dh) dot_product (c1(t) - c2(t)) / ||c1(t) - c2(t)||.
     */
    GeometryBase::derivative_map partial_derivatives1;
    curve1->partialDerivativesPoint(_, partial_derivatives1, parameter_t1);
    for(const auto& [parameter, vector]: partial_derivatives1)
    {
        // Chain rule.
        double partial = v1x*vector.x + v1y*vector.y;
        result.set(parameter, partial);
    }

    GeometryBase::derivative_map partial_derivatives2;
    curve2->partialDerivativesPoint(_, partial_derivatives2, parameter_t2);
    for(const auto& [parameter, vector]: partial_derivatives2)
    {
        // Chain rule.
        double partial = -(v2x*vector.x + v2y*vector.y);
        result.set(parameter, partial);
    }
    return result;
}

OptimizedVector ParallelCurves::differentialOptimized(const ParameterGroupManager& manager) const
{
    return manager.optimizeVector(differentialNonOptimized(manager));
}

void ParallelCurves::declareParameters(ParameterGroupManager& manager) const
{
    manager.addParameter(parameter_t1);
    manager.addParameter(parameter_t2);
    for(Parameter* p: curve1->getParameters())
    {
        manager.addParameter(p);
    }
    for(Parameter* p: curve2->getParameters())
    {
        manager.addParameter(p);
    }
}

} // namespace NamedSketcher::GCS
