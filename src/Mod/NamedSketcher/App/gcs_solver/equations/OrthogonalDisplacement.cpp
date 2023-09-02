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

#include <algorithm>

#include <Base/Exception.h>

#include "../parameters/ParameterGroupManager.h"
#include "../parameters/ParameterValueMapper.h"

#include "OrthogonalDisplacement.h"

namespace NamedSketcher::GCS
{

void OrthogonalDisplacement::set(Point* _start, Point* _end, Point* _displaced_point, std::vector<std::pair<double,Parameter*>> _displacement_combinations)
{
    if(_start == _end)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    start = _start;
    end = _end;
    displaced_point = _displaced_point;
    displacement_combinations = std::move(_displacement_combinations);
}

void OrthogonalDisplacement::set(Point* _start, Point* _end, Point* _displaced_point, Parameter* d)
{
    set(_start, _end, _displaced_point, {{1.0, d}});
}

// det((end - start), (displaced_point - start))^2 - totalDisplacement^2 * ||end-start||^2 = 0
double OrthogonalDisplacement::error(const ParameterGroupManager& manager) const
{
    double sx = manager.getValue(&start->x);
    double sy = manager.getValue(&start->y);
    double ex = manager.getValue(&end->x);
    double ey = manager.getValue(&end->y);
    double dx = manager.getValue(&displaced_point->x);
    double dy = manager.getValue(&displaced_point->y);

    double vx = ex - sx;
    double vy = ey - sy;
    double wx = dx - sx;
    double wy = dy - sy;

    double d = totalDisplacement(manager);

    if(isCoincident(manager))
    {
        // ||displaced_point - start||^2 - distance^2 = 0
        return wx*wx + wy*wy - d*d;
    }

    if(isHorizontal(manager))
    {
        // (end_x - start_x)^2 - distance^2 = 0.
        return vx*vx - d*d;
    }

    if(isVertical(manager))
    {
        // (end_y - start_y)^2 - distance^2 = 0.
        return vy*vy - d*d;
    }

    double det = vx*wy - vy*wx;
    return det*det - d*d*(vx*vx + vy*vy);
}

ParameterVector OrthogonalDisplacement::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    double sx = _(&start->x);
    double sy = _(&start->y);
    double ex = _(&end->x);
    double ey = _(&end->y);
    double dx = _(&displaced_point->x);
    double dy = _(&displaced_point->y);

    double vx = ex - sx;
    double vy = ey - sy;
    double wx = dx - sx;
    double wy = dy - sy;
    double norm_v2 = vx*vx + vy*vy;
    double det = vx*wy - vy*wx;

    double d = totalDisplacement(_);

    /*
     * Differentiate
     * (vx*wy - vywx)^2 - d*d*(vx*vx + vy*vy)
     */
    ParameterVector result;
    result.set(&displaced_point->x, -vy * 2.0 * det);
    result.set(&displaced_point->y, +vx * 2.0 * det);
    setDisplacementDifferentials(_, result, norm_v2);
    result.set(&end->x  , +wy - d * d * 2.0 * vx);
    result.set(&end->y  , -wx - d * d * 2.0 * vy);
    result.set(&start->x, -wy + d * d * 2.0 * vx);
    result.set(&start->y, +wx + d * d * 2.0 * vy);
    return result;
}

OptimizedVector OrthogonalDisplacement::differentialOptimized(const ParameterGroupManager& manager) const
{
    double sx = manager.getValue(&start->x);
    double sy = manager.getValue(&start->y);
    double ex = manager.getValue(&end->x);
    double ey = manager.getValue(&end->y);
    double dx = manager.getValue(&displaced_point->x);
    double dy = manager.getValue(&displaced_point->y);

    double vx = ex - sx;
    double vy = ey - sy;
    double wx = dx - sx;
    double wy = dy - sy;

    if(isCoincident(manager))
    {
        // wx*wx + wy*wy - d*d
        OptimizedVector result;
        result.set(manager.getOptimizedParameter(&start->x), -2.0 * wx);
        result.set(manager.getOptimizedParameter(&start->y), -2.0 * wy);
        result.set(manager.getOptimizedParameter(&displaced_point->x), 2.0 * wx);
        result.set(manager.getOptimizedParameter(&displaced_point->y), 2.0 * wy);
        setDisplacementDifferentials(manager, result);
        return result;
    }

    if(isHorizontal(manager) || isVertical(manager))
    {
        OptimizedVector result;
        if(isHorizontal(manager))
        {
            // (end_x - start_x)^2 - distance^2 = 0.
            result.set(manager.getOptimizedParameter(&end->x), 2.0 * vx);
            result.set(manager.getOptimizedParameter(&start->x), -2.0 * vx);
        }
        else if(isVertical(manager))
        {
            // (end_y - start_y)^2 - distance^2 = 0.
            result.set(manager.getOptimizedParameter(&end->y), 2.0 * vy);
            result.set(manager.getOptimizedParameter(&start->y), -2.0 * vy);
        }
        setDisplacementDifferentials(manager, result);
        return result;
    }

    return manager.optimizeVector(differentialNonOptimized(manager));
}

void OrthogonalDisplacement::declareParameters(ParameterGroupManager& manager) const
{
    manager.addParameter(&start->x);
    manager.addParameter(&start->y);
    manager.addParameter(&end->x);
    manager.addParameter(&end->y);
    manager.addParameter(&displaced_point->x);
    manager.addParameter(&displaced_point->y);
    for(auto& p: displacement_combinations)
    {
        manager.addParameter(p.second);
    }
}

bool OrthogonalDisplacement::isCoincident(const ParameterGroupManager& manager) const
{
    return (isHorizontal(manager) && isVertical(manager));
}

bool OrthogonalDisplacement::isHorizontal(const ParameterGroupManager& manager) const
{
    return manager.areParametersEqual(&start->y, &end->y);
}

bool OrthogonalDisplacement::isVertical(const ParameterGroupManager& manager) const
{
    return manager.areParametersEqual(&start->x, &end->x);
}

double OrthogonalDisplacement::totalDisplacement(const ParameterValueMapper& _) const
{
    return std::accumulate(displacement_combinations.cbegin(), displacement_combinations.cend(), 0, [_](double t, const std::pair<double,Parameter*>& a){return t += a.first * _(a.second);});
}

void OrthogonalDisplacement::setDisplacementDifferentials(const ParameterGroupManager& manager, OptimizedVector& result, double factor) const
{
    double d = totalDisplacement(manager);

    // It can happen that parameters are equal in this combination.
    // So, instead of setting them, we add.
    for(auto& p: displacement_combinations)
    {
        double value = -2.0 * p.first * d * factor;
        result.add(manager.getOptimizedParameter(p.second), value);
    }
}

void OrthogonalDisplacement::setDisplacementDifferentials(const ParameterValueMapper& _, ParameterVector& result, double factor) const
{
    double d = totalDisplacement(_);

    // It can happen that parameters are equal in this combination.
    // So, instead of setting them, we add.
    for(auto& p: displacement_combinations)
    {
        double value = -2.0 * p.first * d * factor;
        result.add(p.second, value);
    }
}


void OrthogonalDisplacement::report() const
{
    std::cerr << "Orthogonal displacement";
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
