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

#include <random>

#include <Base/Exception.h>

#include "../parameters/ParameterGroupManager.h"
#include "../parameters/ParameterValueMapper.h"
#include "Distance.h"

namespace NamedSketcher::GCS
{

void Distance::set(Point* x, Point* y, Parameter* d)
{
    if(x == y)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    b = y;
    distance = d;
}

// |a-b|^2 - distance^2 = 0
double Distance::error(const ParameterGroupManager& manager) const
{
    if(isCoincident(manager))
    {
        // -distance = 0
        return -manager.getValue(distance);
    }

    if(isHorizontal(manager))
    {
        // ax - bx - distance = 0.
        return manager.getValue(&a->x) - manager.getValue(&b->x) - manager.getValue(distance);
    }

    if(isVertical(manager))
    {
        // ay - by - distance = 0.
        return manager.getValue(&a->y) - manager.getValue(&b->y) - manager.getValue(distance);
    }

    double a1 = manager.getValue(&a->x);
    double a2 = manager.getValue(&a->y);
    double b1 = manager.getValue(&b->x);
    double b2 = manager.getValue(&b->y);
    double d = manager.getValue(distance);
    return (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) - d*d;
}

ParameterVector Distance::differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const
{
    double a1 = parameter_mapper(&a->x);
    double a2 = parameter_mapper(&a->y);
    double b1 = parameter_mapper(&b->x);
    double b2 = parameter_mapper(&b->y);

    ParameterVector result;
    result.set(&a->x, 2*(a1-b1));
    result.set(&a->y, 2*(a2-b2));
    result.set(&b->x, 2*(b1-a1));
    result.set(&b->y, 2*(b2-a2));
    return result;
}

OptimizedVector Distance::differentialOptimized(const ParameterGroupManager& manager) const
{
    if(isCoincident(manager))
    {
        return OptimizedVector();
    }

    if(isHorizontal(manager))
    {
        // ax - bx - distance = 0.
        OptimizedVector result;
        result.set(manager.getOptimizedParameter(&a->x), 1);
        result.set(manager.getOptimizedParameter(&b->x), -1);
        return result;
    }

    if(isVertical(manager))
    {
        // ay - by - distance = 0.
        OptimizedVector result;
        result.set(manager.getOptimizedParameter(&a->y), 1);
        result.set(manager.getOptimizedParameter(&b->y), -1);
        return result;
    }

    return manager.optimizeVector(differentialNonOptimized(manager));
}

void Distance::declareParameters(ParameterGroupManager& manager)
{
    manager.addParameter(&a->x);
    manager.addParameter(&a->y);
    manager.addParameter(&b->x);
    manager.addParameter(&b->y);
    manager.addParameter(distance);
    manager.setParameterConstant(distance);
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

} // namespace NamedSketcher::GCS
