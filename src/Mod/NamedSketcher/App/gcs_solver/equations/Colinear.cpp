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
#include "Colinear.h"

namespace NamedSketcher::GCS
{

void Colinear::set(Point* x, Point* y, Point* z)
{
    if(x == y || x == z || y == z)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    b = y;
    c = z;
}

// det((a1, a2, 1),
//     (b1, b2, 1),
//     (c1, c2, 1))
double Colinear::error(const ParameterGroupManager& manager) const
{
    if(isAlreadyColinear(manager))
    {
        return 0.0;
    }

    if(isHorizontal(manager))
    {
        if(manager.areParametersEqual(&a->y, &b->y))
        {
            // ay - cy = 0.
            double a2 = manager.getValue(&a->y);
            double c2 = manager.getValue(&c->y);
            return a2 - c2;
        }

        // ay - by = 0.
        double a2 = manager.getValue(&a->y);
        double b2 = manager.getValue(&b->y);
        return a2 - b2;
    }

    if(isVertical(manager))
    {
        if(manager.areParametersEqual(&a->x, &b->x))
        {
            // ax - cx = 0.
            double a1 = manager.getValue(&a->x);
            double c1 = manager.getValue(&c->x);
            return a1 - c1;
        }

        // ax - bx = 0.
        double a1 = manager.getValue(&a->x);
        double b1 = manager.getValue(&b->x);
        return a1 - b1;
    }

    double a1 = manager.getValue(&a->x);
    double a2 = manager.getValue(&a->y);
    double b1 = manager.getValue(&b->x);
    double b2 = manager.getValue(&b->y);
    double c1 = manager.getValue(&c->x);
    double c2 = manager.getValue(&c->y);
    return (b1*c2 - b2*c1) + (a2*c1 - a1*c2) + (a1*b2 - a2*b1);
}

ParameterVector Colinear::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    double a1 = _(a->x);
    double a2 = _(a->y);
    double b1 = _(b->x);
    double b2 = _(b->y);
    double c1 = _(c->x);
    double c2 = _(c->y);

    ParameterVector result;
    result.set(&a->x, b2-c2);
    result.set(&a->y, c1-b1);
    result.set(&b->x, c2-a2);
    result.set(&b->y, a1-c1);
    result.set(&c->x, a2-b2);
    result.set(&c->y, b1-a1);
    return result;
}

OptimizedVector Colinear::differentialOptimized(const ParameterGroupManager& manager) const
{
    if(isAlreadyColinear(manager))
    {
        return OptimizedVector();
    }

    if(isHorizontal(manager))
    {
        OptimizedVector result;
        if(manager.areParametersEqual(&a->y, &b->y))
        {
            // a2 - c2 = 0.
            result.set(manager.getOptimizedParameter(&a->y), 1);
            result.set(manager.getOptimizedParameter(&c->y), -1);
        } else {
            // a2 - b2 = 0.
            result.set(manager.getOptimizedParameter(&a->y), 1);
            result.set(manager.getOptimizedParameter(&b->y), -1);
        }
        return result;
    }

    if(isVertical(manager))
    {
        OptimizedVector result;
        if(manager.areParametersEqual(&a->x, &b->x))
        {
            // a1 - c1 = 0.
            result.set(manager.getOptimizedParameter(&a->x), 1);
            result.set(manager.getOptimizedParameter(&c->x), -1);
        } else {
            // a1 - b1 = 0.
            result.set(manager.getOptimizedParameter(&a->x), 1);
            result.set(manager.getOptimizedParameter(&b->x), -1);
        }
        return result;
    }

    return manager.optimizeVector(differentialNonOptimized(manager));
}

bool Colinear::isAlreadyColinear(const ParameterGroupManager& manager) const
{
    if(manager.areParametersEqual(&a->x, &b->x) && manager.areParametersEqual(&a->x, &c->x))
    {
        return true;
    }

    if(manager.areParametersEqual(&a->y, &b->y) && manager.areParametersEqual(&a->y, &c->y))
    {
        return true;
    }

    return false;
}

bool Colinear::isHorizontal(const ParameterGroupManager& manager) const
{
    return (manager.areParametersEqual(&a->y, &b->y) || manager.areParametersEqual(&a->y, &c->y) || manager.areParametersEqual(&b->y, &c->y));
}

bool Colinear::isVertical(const ParameterGroupManager& manager) const
{
    return (manager.areParametersEqual(&a->x, &b->x) || manager.areParametersEqual(&a->x, &c->x) || manager.areParametersEqual(&b->x, &c->x));
}

void Colinear::declareParameters(ParameterGroupManager& manager) const
{
    manager.addParameter(&a->x);
    manager.addParameter(&a->y);
    manager.addParameter(&b->x);
    manager.addParameter(&b->y);
    manager.addParameter(&c->x);
    manager.addParameter(&c->y);
}

bool Colinear::optimizeParameters(ParameterGroupManager& manager) const
{
    bool result = false;
    if(isHorizontal(manager))
    {
        result = manager.setParameterEqual(&a->y, &b->y) || result;
        result = manager.setParameterEqual(&a->y, &c->y) || result;
    }

    if(isVertical(manager))
    {
        result = manager.setParameterEqual(&a->x, &b->x) || result;
        result = manager.setParameterEqual(&a->x, &c->x) || result;
    }
    return result;
}

} // namespace NamedSketcher::GCS
