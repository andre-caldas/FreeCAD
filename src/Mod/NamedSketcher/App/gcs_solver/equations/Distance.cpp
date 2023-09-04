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

void Distance::set(Point* x, Point* y, double distance)
{
    auto const_dist = std::make_unique<Parameter>(distance);
    set(x, y, const_dist.get());
    constant_distance = std::move(const_dist);
}

void Distance::set(Point* x, Point* y, std::vector<std::pair<double,Parameter*>> distance_combs)
{
    constant_distance.reset();
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

// |a-b| - |distance| = 0
double Distance::error(const ParameterGroupManager& manager) const
{
    double ax = manager.getValue(&a->x);
    double ay = manager.getValue(&a->y);
    double bx = manager.getValue(&b->x);
    double by = manager.getValue(&b->y);
    double d = totalDistance(manager);
    return std::sqrt((ax-bx)*(ax-bx) + (ay-by)*(ay-by)) - std::abs(d);
}

ParameterVector Distance::differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const
{
    double ax = parameter_mapper(&a->x);
    double ay = parameter_mapper(&a->y);
    double bx = parameter_mapper(&b->x);
    double by = parameter_mapper(&b->y);
    double norm2 = (ax-bx)*(ax-bx) + (ay-by)*(ay-by);
    double sqrt = std::sqrt(norm2);

    ParameterVector result;
    double dax_norm2 = 2.0 * (ax-bx);
    double day_norm2 = 2.0 * (ay-by);
    double dbx_norm2 = 2.0 * (bx-ax);
    double dby_norm2 = 2.0 * (by-ay);

    double dax_sqrt = dax_norm2 / (2.0 * sqrt);
    double day_sqrt = day_norm2 / (2.0 * sqrt);
    double dbx_sqrt = dbx_norm2 / (2.0 * sqrt);
    double dby_sqrt = dby_norm2 / (2.0 * sqrt);

    result.set(&a->x, dax_sqrt);
    result.set(&a->y, day_sqrt);
    result.set(&b->x, dbx_sqrt);
    result.set(&b->y, dby_sqrt);

    // It can happen that parameters are equal in this combination.
    // So, instead of setting them, we add.
    double d = totalDistance(parameter_mapper);
    for(auto& p: distance_combinations)
    {
        // -|d|
        double sign = (d > 0.0)?1.0:-1.0;
        result.add(p.second, -sign * p.first);
    }
    return result;
}

OptimizedVector Distance::differentialOptimized(const ParameterGroupManager& manager) const
{
    // There is no need to optimize.
    return manager.optimizeVector(differentialNonOptimized(manager));
}

void Distance::declareParameters(ParameterGroupManager& manager) const
{
    if(constant_distance)
    {
        manager.addParameter(constant_distance.get());
        manager.setParameterConstant(constant_distance.get());
    }
    manager.addParameter(&a->x);
    manager.addParameter(&a->y);
    manager.addParameter(&b->x);
    manager.addParameter(&b->y);
    for(auto& p: distance_combinations)
    {
        manager.addParameter(p.second);
    }
}

double Distance::limitStep(const ParameterGroupManager& manager, const OptimizedVector& step) const
{
    double limit = 1.0;
    // Do not change distance sign.
    double required_distance_before = totalDistance(manager);
    double required_distance_change = 0.0;
    for(auto& [coef, param]: distance_combinations)
    {
        required_distance_change += coef * step[manager.getOptimizedParameter(param)];
    }
    double required_distance_after = required_distance_before + required_distance_change;
    if(std::signbit(required_distance_before) != std::signbit(required_distance_after))
    {
        limit = std::min(limit, std::abs(required_distance_before) / std::abs(required_distance_change));
    }

    double ax = manager.getOptimizedParameterValue(&a->x);
    double ay = manager.getOptimizedParameterValue(&a->y);
    double bx = manager.getOptimizedParameterValue(&b->x);
    double by = manager.getOptimizedParameterValue(&b->y);

    double cdx = ax - bx;
    double cdy = ay - by;
    double current_distance = std::sqrt(cdx*cdx + cdy*cdy);

    double step_ax = step[manager.getOptimizedParameter(&a->x)];
    double step_ay = step[manager.getOptimizedParameter(&a->y)];
    double step_bx = step[manager.getOptimizedParameter(&b->x)];
    double step_by = step[manager.getOptimizedParameter(&b->y)];

    double sdx = step_ax - step_bx;
    double sdy = step_ay - step_by;
    double step_distance = std::sqrt(sdx*sdx + sdy*sdy);

    return std::min(limit,
                    std::max(current_distance,std::abs(required_distance_before)) / step_distance);
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
