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

// sqrt(det((end - start), (displaced_point - start))^2/||end-start||^2) - ||totalDisplacement|| = 0
double OrthogonalDisplacement::error(const ParameterGroupManager& manager) const
{
    double sx = manager.getValue(&start->x);
    double sy = manager.getValue(&start->y);
    double ex = manager.getValue(&end->x);
    double ey = manager.getValue(&end->y);
    double dx = manager.getValue(&displaced_point->x);
    double dy = manager.getValue(&displaced_point->y);

    double esx = ex - sx;
    double esy = ey - sy;
    double dmx = dx - (0.5*sx + 0.5*ex);
    double dmy = dy - (0.5*sy + 0.5*ey);

    double d = totalDisplacement(manager);

    if(isCoincident(manager))
    {
        // ||displaced_point - start|| - |distance| = 0
        return std::sqrt(dmx*dmx + dmy*dmy) - std::abs(d);
    }

    if(isHorizontal(manager))
    {
        // |displaced_point_y - start_y)| - |distance| = 0.
        return std::abs(dmy) - std::abs(d);
    }

    if(isVertical(manager))
    {
        // |displaced_point_x - start_x)| - |distance| = 0.
        return std::abs(dmx) - std::abs(d);
    }

    double det = esx*dmy - esy*dmx;
    double det2 = det * det;
    double norm2 = esx*esx + esy*esy;

std::cout << "Error: " << std::sqrt(det2/norm2) - std::abs(d) << std::endl;
    return std::sqrt(det2/norm2) - std::abs(d);
}

ParameterVector OrthogonalDisplacement::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    double sx = _(&start->x);
    double sy = _(&start->y);
    double ex = _(&end->x);
    double ey = _(&end->y);
    double dx = _(&displaced_point->x);
    double dy = _(&displaced_point->y);

    double esx = ex - sx;
    double esy = ey - sy;
    double dmx = dx - (0.5*sx + 0.5*ex);
    double dmy = dy - (0.5*sy + 0.5*ey);

    double det = esx*dmy - esy*dmx;
    double det2 = det * det;
    double norm2 = esx*esx + esy*esy;
    double sqrt = std::sqrt(det2 / norm2);

    /*
     * Differentiate
     * sqrt(det^2/||end - start||^2) - |d|
     */
    double dex_det2 = (+dmy + 0.5*esy) * (2.0 * det);
    double dey_det2 = (-dmx - 0.5*esx) * (2.0 * det);
    double dsx_det2 = (-dmy + 0.5*esy) * (2.0 * det);
    double dsy_det2 = (+dmx - 0.5*esx) * (2.0 * det);
    double ddx_det2 = -esy * (2.0 * det);
    double ddy_det2 = +esx * (2.0 * det);

    double dex_norm2 = +esx;
    double dey_norm2 = +esy;
    double dsx_norm2 = -esx;
    double dsy_norm2 = -esy;

    double dex_quocient = (dex_det2 * norm2 - det2 * dex_norm2) / (norm2 * norm2);
    double dey_quocient = (dey_det2 * norm2 - det2 * dey_norm2) / (norm2 * norm2);
    double dsx_quocient = (dsx_det2 * norm2 - det2 * dsx_norm2) / (norm2 * norm2);
    double dsy_quocient = (dsy_det2 * norm2 - det2 * dsy_norm2) / (norm2 * norm2);
    double ddx_quocient = ddx_det2 / norm2;
    double ddy_quocient = ddy_det2 / norm2;

    double dex_sqrt = dex_quocient / (2.0 * sqrt);
    double dey_sqrt = dey_quocient / (2.0 * sqrt);
    double dsx_sqrt = dsx_quocient / (2.0 * sqrt);
    double dsy_sqrt = dsy_quocient / (2.0 * sqrt);
    double ddx_sqrt = ddx_quocient / (2.0 * sqrt);
    double ddy_sqrt = ddy_quocient / (2.0 * sqrt);

    ParameterVector result;
    setDisplacementDifferentials(_, result);

    result.set(&end->x  , dex_sqrt);
    result.set(&end->y  , dey_sqrt);
    result.set(&start->x, dsx_sqrt);
    result.set(&start->y, dsy_sqrt);
    result.set(&displaced_point->x, ddx_sqrt);
    result.set(&displaced_point->y, ddy_sqrt);
    return result;
}

OptimizedVector OrthogonalDisplacement::differentialOptimized(const ParameterGroupManager& manager) const
{
    if(isHorizontal(manager) || isVertical(manager) || isCoincident(manager))
    {
        OptimizedVector result;
        setDisplacementDifferentials(manager, result);

        double sx = manager.getValue(&start->x);
        double sy = manager.getValue(&start->y);
        double dx = manager.getValue(&displaced_point->x);
        double dy = manager.getValue(&displaced_point->y);

        double dsx = dx - sx;
        double dsy = dy - sy;

        if(isCoincident(manager))
        {
            // ||displaced_point - start|| - |distance| = 0
            // std::sqrt(dsx*dsx + dsy*dsy) - std::abs(d);
            double dsx_norm2 = -dsx;
            double dsy_norm2 = -dsy;
            double ddx_norm2 = +dsx;
            double ddy_norm2 = +dsy;

            double sqrt = std::sqrt(dsx*dsx + dsy*dsy);
            if(sqrt == 0.0)
            {
                sqrt = 1.0;
            }

            double dsx_sqrt = dsx_norm2 / (2.0 * sqrt);
            double dsy_sqrt = dsy_norm2 / (2.0 * sqrt);
            double ddx_sqrt = ddx_norm2 / (2.0 * sqrt);
            double ddy_sqrt = ddy_norm2 / (2.0 * sqrt);

            result.set(manager.getOptimizedParameter(&start->x), dsx_sqrt);
            result.set(manager.getOptimizedParameter(&start->y), dsy_sqrt);
            result.set(manager.getOptimizedParameter(&displaced_point->x), ddx_sqrt);
            result.set(manager.getOptimizedParameter(&displaced_point->y), ddy_sqrt);
            return result;
        }

        if(isHorizontal(manager))
        {
            // |displaced_point_y - start_y)| - |distance| = 0.
            // std::abs(dsy) - std::abs(d);
            double sign = (dsy >= 0)?1.0:-1.0;
            result.set(manager.getOptimizedParameter(&start->x), -sign*sy);
            result.set(manager.getOptimizedParameter(&displaced_point->x), sign*dy);
            return result;
        }

        if(isVertical(manager))
        {
            // |displaced_point_x - start_x)| - |distance| = 0.
            // std::abs(dsx) - std::abs(d);
            double sign = (dsx >= 0)?1.0:-1.0;
            result.set(manager.getOptimizedParameter(&start->x), -sign*sx);
            result.set(manager.getOptimizedParameter(&displaced_point->x), sign*dx);
            return result;
        }
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
    return std::accumulate(displacement_combinations.cbegin(), displacement_combinations.cend(), 0, [&_](double t, const std::pair<double,Parameter*>& a){return t += a.first * _(a.second);});
}

void OrthogonalDisplacement::setDisplacementDifferentials(const ParameterGroupManager& manager, OptimizedVector& result) const
{
    double d = totalDisplacement(manager);
    double sign = (d >= 0.0)?1.0:-1.0;

    // It can happen that parameters are equal in this combination.
    // So, instead of setting them, we add.
    for(auto& p: displacement_combinations)
    {
        result.add(manager.getOptimizedParameter(p.second), -sign * p.first);
    }
}

void OrthogonalDisplacement::setDisplacementDifferentials(const ParameterValueMapper& _, ParameterVector& result) const
{
    double d = totalDisplacement(_);
    double sign = (d >= 0.0)?1.0:-1.0;

    // It can happen that parameters are equal in this combination.
    // So, instead of setting them, we add.
    for(auto& p: displacement_combinations)
    {
        result.add(p.second, -sign * p.first);
    }
}


void OrthogonalDisplacement::report() const
{
    std::cerr << "Orthogonal displacement";
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
