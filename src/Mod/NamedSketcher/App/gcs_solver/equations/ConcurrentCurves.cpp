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
#include "ConcurrentCurves.h"

namespace NamedSketcher::GCS
{

void ConcurrentCurves::set(GeometryBase* c1, Parameter* t1, GeometryBase* c2, Parameter* t2)
{
    curve1 = c1;
    curve2 = c2;
    parameter_t1 = t1;
    parameter_t2 = t2;
}

// ||curve2(t) - curve1(t)||
double ConcurrentCurves::error(const ParameterGroupManager& manager) const
{
    auto c1 = curve1->positionAtParameter(manager, parameter_t1);
    auto c2 = curve2->positionAtParameter(manager, parameter_t2);
    double result = std::sqrt((c2.x-c1.x)*(c2.x-c1.x) + (c2.y-c1.y)*(c2.y-c1.y));
    return result;
}

ParameterVector ConcurrentCurves::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    /*
     * Here we calculate the partial derivatives of
     * ||curve2(t, p1, ..., pn) - curve1(t, q1, ..., qm)||.
     */
    auto c1 = curve1->positionAtParameter(_, parameter_t1);
    auto c2 = curve2->positionAtParameter(_, parameter_t2);

    /*
     * Grad(||a||) = (ax, ay) / ||a||
     */
    double vx = (c2.x - c1.x);
    double vy = (c2.y - c1.y);
    double denominator = std::sqrt(vx*vx + vy*vy);
    if(denominator != 0)
    {
        vx /= denominator;
        vy /= denominator;
    }

    ParameterVector result;

    /*
     * For the curve parameter h, the derivative is:
     * +(dc2/dh) dot_product (c2(t) - c1(t)) / ||c2(t) - c1(t)||.
     * -(dc1/dh) dot_product (c2(t) - c1(t)) / ||c2(t) - c1(t)||.
     */
    GeometryBase::derivative_map partial_derivatives2;
    curve2->partialDerivativesPoint(_, partial_derivatives2, parameter_t2);
    for(const auto& [parameter, vector]: partial_derivatives2)
    {
        // Chain rule.
        double partial = vx*vector.x + vy*vector.y;
        result.set(parameter, partial);
    }

    GeometryBase::derivative_map partial_derivatives1;
    curve1->partialDerivativesPoint(_, partial_derivatives1, parameter_t1);
    for(const auto& [parameter, vector]: partial_derivatives1)
    {
        // Chain rule.
        double partial = -(vx*vector.x + vy*vector.y);
        result.add(parameter, partial);
    }
    return result;
}

OptimizedVector ConcurrentCurves::differentialOptimized(const ParameterGroupManager& manager) const
{
    return manager.optimizeVector(differentialNonOptimized(manager));
}

void ConcurrentCurves::declareParameters(ParameterGroupManager& manager) const
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


void ConcurrentCurves::report() const
{
    std::cerr << "Concurrent Curves";
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
