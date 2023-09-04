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

// SQRT of:
// det((ax, ay, 1),
//     (bx, by, 1),
//     (cx, cy, 1))^2 / (||a||^2 * ||b||^2 * ||c||^2)
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
            double ay = manager.getValue(&a->y);
            double cy = manager.getValue(&c->y);
            return ay - cy;
        }

        // ay - by = 0.
        double ay = manager.getValue(&a->y);
        double by = manager.getValue(&b->y);
        return ay - by;
    }

    if(isVertical(manager))
    {
        if(manager.areParametersEqual(&a->x, &b->x))
        {
            // ax - cx = 0.
            double ax = manager.getValue(&a->x);
            double cx = manager.getValue(&c->x);
            return ax - cx;
        }

        // ax - bx = 0.
        double ax = manager.getValue(&a->x);
        double bx = manager.getValue(&b->x);
        return ax - bx;
    }

    double ax = manager.getValue(&a->x);
    double ay = manager.getValue(&a->y);
    double bx = manager.getValue(&b->x);
    double by = manager.getValue(&b->y);
    double cx = manager.getValue(&c->x);
    double cy = manager.getValue(&c->y);

    double det = (bx*cy - by*cx) + (ay*cx - ax*cy) + (ax*by - ay*bx);
    double norms_2 = (3.0 + ax*ax + ay*ay + bx*bx + by*by + cx*cx + cy*cy);
    return std::sqrt(det * det / norms_2);
}

ParameterVector Colinear::differentialNonOptimized(const GCS::ParameterValueMapper& _) const
{
    double ax = _(a->x);
    double ay = _(a->y);
    double bx = _(b->x);
    double by = _(b->y);
    double cx = _(c->x);
    double cy = _(c->y);

    double det = (bx*cy - by*cx) + (ay*cx - ax*cy) + (ax*by - ay*bx);
    double det2 = det * det;
    double norms2 = (3.0 + ax*ax + ay*ay + bx*bx + by*by + cx*cx + cy*cy);
    double sqrt = std::sqrt(det2 / norms2);

    ParameterVector result;
    double dax_det2 = (by-cy) * 2.0 * det;
    double day_det2 = (cx-bx) * 2.0 * det;
    double dbx_det2 = (cy-ay) * 2.0 * det;
    double dby_det2 = (ax-cx) * 2.0 * det;
    double dcx_det2 = (ay-by) * 2.0 * det;
    double dcy_det2 = (bx-ax) * 2.0 * det;

    double dax_norms2 = ax;
    double day_norms2 = ay;
    double dbx_norms2 = bx;
    double dby_norms2 = by;
    double dcx_norms2 = cx;
    double dcy_norms2 = cy;

    double dax_quocient = (dax_det2 * norms2 - det2 * dax_norms2) / (norms2*norms2);
    double day_quocient = (day_det2 * norms2 - det2 * day_norms2) / (norms2*norms2);
    double dbx_quocient = (dbx_det2 * norms2 - det2 * dbx_norms2) / (norms2*norms2);
    double dby_quocient = (dby_det2 * norms2 - det2 * dby_norms2) / (norms2*norms2);
    double dcx_quocient = (dcx_det2 * norms2 - det2 * dcx_norms2) / (norms2*norms2);
    double dcy_quocient = (dcy_det2 * norms2 - det2 * dcy_norms2) / (norms2*norms2);

    double dax_sqrt = dax_quocient / (2.0 * sqrt);
    double day_sqrt = day_quocient / (2.0 * sqrt);
    double dbx_sqrt = dbx_quocient / (2.0 * sqrt);
    double dby_sqrt = dby_quocient / (2.0 * sqrt);
    double dcx_sqrt = dcx_quocient / (2.0 * sqrt);
    double dcy_sqrt = dcy_quocient / (2.0 * sqrt);

    result.set(&a->x, dax_sqrt);
    result.set(&a->y, day_sqrt);
    result.set(&b->x, dbx_sqrt);
    result.set(&b->y, dby_sqrt);
    result.set(&c->x, dcx_sqrt);
    result.set(&c->y, dcy_sqrt);
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
            // ay - cy = 0.
            result.set(manager.getOptimizedParameter(&a->y), 1);
            result.set(manager.getOptimizedParameter(&c->y), -1);
        } else {
            // ay - by = 0.
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
            // ax - cx = 0.
            result.set(manager.getOptimizedParameter(&a->x), 1);
            result.set(manager.getOptimizedParameter(&c->x), -1);
        } else {
            // ax - bx = 0.
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


void Colinear::report() const
{
    std::cerr << "Colinear";
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
