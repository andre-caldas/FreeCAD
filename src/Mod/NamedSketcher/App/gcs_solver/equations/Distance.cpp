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

#include "Distance.h"

namespace NamedSketcher::GCS
{

void Distance::set(Point* x, Point* y, std::vector<std::pair<double,Parameter*>> distance_combs)
{
    if(x == y)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    b = y;
    distance_combinations = std::move(distance_combs);
}

void Distance::set(Point* x, Point* y, Parameter* d)
{
    set(x, y, {{1.0, d}});
}

// |a-b|^2 - distance^2 = 0
double Distance::error(const ParameterGroupManager& manager) const
{
    if(isCoincident(manager))
    {
        // -distance^2 = 0
        double d = totalDistance(manager);
        return - d*d;
    }

    if(isHorizontal(manager))
    {
        // (ax - bx)^2 - distance^2 = 0.
        double dif = manager.getValue(&a->x) - manager.getValue(&b->x);
        double d = totalDistance(manager);
        return dif*dif - d*d;
    }

    if(isVertical(manager))
    {
        // (ay - by)^2 - distance^2 = 0.
        double dif = manager.getValue(&a->y) - manager.getValue(&b->y);
        double d = totalDistance(manager);
        return dif*dif - d*d;
    }

    double a1 = manager.getValue(&a->x);

    double a2 = manager.getValue(&a->y);
    double b1 = manager.getValue(&b->x);
    double b2 = manager.getValue(&b->y);
    double d = totalDistance(manager);
    return (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) - d*d;
}

ParameterVector Distance::differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const
{
    double a1 = parameter_mapper(&a->x);
    double a2 = parameter_mapper(&a->y);
    double b1 = parameter_mapper(&b->x);
    double b2 = parameter_mapper(&b->y);
    double d = totalDistance(parameter_mapper);

    ParameterVector result;
    result.set(&a->x, 2.0*(a1-b1));
    result.set(&a->y, 2.0*(a2-b2));
    result.set(&b->x, 2.0*(b1-a1));
    result.set(&b->y, 2.0*(b2-a2));
    // It can happen that parameters are equal in this combination.
    // So, instead of setting them, we add.
    for(auto& p: distance_combinations)
    {
        result.add(p.second, -2.0 * p.first * d);
    }
    return result;
}

OptimizedVector Distance::differentialOptimized(const ParameterGroupManager& manager) const
{
    if(isCoincident(manager))
    {
        return OptimizedVector();
    }

    double d = totalDistance(manager);
    if(isHorizontal(manager) || isVertical(manager))
    {
        OptimizedVector result;
        if(isHorizontal(manager))
        {
            // (ax - bx)^2 - distance^2 = 0.
            double diff = manager.getValue(&a->x) - manager.getValue(&b->x);
            result.set(manager.getOptimizedParameter(&a->x), +2.0 * diff);
            result.set(manager.getOptimizedParameter(&b->x), -2.0 * diff);
        }
        else if(isVertical(manager))
        {
            // (ay - by)^2 - distance^2 = 0.
            double diff = manager.getValue(&a->y) - manager.getValue(&b->y);
            result.set(manager.getOptimizedParameter(&a->y), +2.0 * diff);
            result.set(manager.getOptimizedParameter(&b->y), -2.0 * diff);
        }
        // It can happen that parameters are equal in this combination.
        // So, instead of setting them, we add.
        for(auto& p: distance_combinations)
        {
            result.add(manager.getOptimizedParameter(p.second), -2.0 * p.first * d);
        }
        return result;
    }

    return manager.optimizeVector(differentialNonOptimized(manager));
}

void Distance::declareParameters(ParameterGroupManager& manager) const
{
    manager.addParameter(&a->x);
    manager.addParameter(&a->y);
    manager.addParameter(&b->x);
    manager.addParameter(&b->y);
    for(auto& p: distance_combinations)
    {
        manager.addParameter(p.second);
    }
}

bool Distance::isCoincident(const ParameterGroupManager& manager) const
{
    return (isHorizontal(manager) && isVertical(manager));
}

bool Distance::isHorizontal(const ParameterGroupManager& manager) const
{
    return manager.areParametersEqual(&a->y, &b->y);
}

bool Distance::isVertical(const ParameterGroupManager& manager) const
{
    return manager.areParametersEqual(&a->x, &b->x);
}

double Distance::totalDistance(const ParameterValueMapper& _) const
{
    return std::accumulate(distance_combinations.cbegin(),
                           distance_combinations.cend(), 0.,
                           [&_](double t, const std::pair<double,Parameter*>& a){return t += a.first * _(a.second);});
}


void Distance::report() const
{
    std::cerr << "Distance";
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
