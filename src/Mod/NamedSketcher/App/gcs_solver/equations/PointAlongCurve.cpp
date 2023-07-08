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

#include <iostream>
#include <cmath>

#include <Base/Exception.h>

#include "../parameters/ParameterGroupManager.h"
#include "../parameters/ParameterValueMapper.h"
#include "../../geometries/GeometryBase.h"
#include "PointAlongCurve.h"

namespace NamedSketcher::GCS
{

void PointAlongCurve::set(Point* p, GeometryBase* c, Parameter* t)
{
    point = p;
    curve = c;
    parameter_t = t;
}

// ||curve(t) - point||
double PointAlongCurve::error(const ParameterGroupManager& manager) const
{
    double px = manager.getValue(&point->x);
    double py = manager.getValue(&point->y);
    auto c = curve->positionAtParameter(manager, parameter_t);
    double result = std::sqrt((c.x-px)*(c.x-px) + (c.y-py)*(c.y-py));
    return result;
}

ParameterVector PointAlongCurve::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    /*
     * Here we calculate the partial derivatives of ||curve(t) - point||.
     */
    double px = _(point->x);
    double py = _(point->y);
    auto c = curve->positionAtParameter(_, parameter_t);

    double vx = (c.x - px);
    double vy = (c.y - py);
    double denominator = std::sqrt(vx*vx + vy*vy);
    if(denominator != 0)
    {
        vx /= denominator;
        vy /= denominator;
    }

    ParameterVector result;
    /*
     * For the x and y coordinate of point,
     * it is the x and y coordinates of -(c(t) - p) / ||c(t) - p||.
     */
    result.set(&point->x, -vx);
    result.set(&point->y, -vy);

    GeometryBase::derivative_map partial_derivatives;
    curve->partialDerivativesPoint(_, partial_derivatives, parameter_t);

    /*
     * For the curve parameter h, the derivative is:
     * (dc/dh) dot_product (c(t) - p) / ||c(t) - p||.
     */
    for(const auto& [parameter, vector]: partial_derivatives)
    {
        // Chain rule.
        double partial = vx*vector.x + vy*vector.y;
        result.set(parameter, partial);
    }

    for(const auto& [parameter,partial]: result.values)
    {
        std::cout << "d/d("<<parameter->name<<") --> " << partial << std::endl;
    }
    return result;
}

OptimizedVector PointAlongCurve::differentialOptimized(const ParameterGroupManager& manager) const
{
    return manager.optimizeVector(differentialNonOptimized(manager));
}

void PointAlongCurve::declareParameters(ParameterGroupManager& manager) const
{
    manager.addParameter(parameter_t);
    manager.addParameter(&point->x);
    manager.addParameter(&point->y);
    for(Parameter* p: curve->getParameters())
    {
        manager.addParameter(p);
    }
}

} // namespace NamedSketcher::GCS
