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

#include "Colinear.h"

namespace NamedSketcher::GCS
{

void Colinear::set(ProxiedParameter* x, ProxiedParameter* y, ProxiedParameter* z)
{
    if(x == y || x == z || y == z)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.")
    }
    a = x;
    b = y;
    c = z;
}

// det((a1, a2, 1),
//     (b1, b2, 1),
//     (c1, c2, 1))
double Colinear::error() const
{
    if(isAlreadyColinear())
    {
        return 0.0;
    }

    if(isHorizontal())
    {
        if(a->y.samePointer(b->y))
        {
            // ay - cy = 0.
            double a2 = a->y.getValue();
            double c2 = c->y.getValue();
            return a2 - c2;
        }

        // ay - by = 0.
        double a2 = a->y.getValue();
        double b2 = b->y.getValue();
        return a2 - b2;
    }

    if(isVertical())
    {
        if(a->x.samePointer(b->x))
        {
            // ax - cx = 0.
            double a1 = a->x.getValue();
            double c1 = c->x.getValue();
            return a1 - c1;
        }

        // ax - bx = 0.
        double a1 = a->x.getValue();
        double b1 = b->x.getValue();
        return a1 - b1;
    }

    double a1 = a->x.getValue();
    double a2 = a->y.getValue();
    double b1 = b->x.getValue();
    double b2 = b->y.getValue();
    double c1 = c->x.getValue();
    double c2 = c->y.getValue();
    return (b1*c2 - b2*c1) + (a2*c1 - a1*c2) + (a1*b2 - a2*b1);
}

Vector Colinear::differentialNonOptimized() const
{
    double a1 = a->x.getValue();
    double a2 = a->y.getValue();
    double b1 = b->x.getValue();
    double b2 = b->y.getValue();
    double c1 = c->x.getValue();
    double c2 = c->y.getValue();

    Vector result;
    result.set(&a->x, b2-c2);
    result.set(&a->y, c1-b1);
    result.set(&b->x, c2-a2);
    result.set(&b->y, a1-c1);
    result.set(&c->x, a2-b2);
    result.set(&c->y, b1-a1);
    return result;
}

OptimizedVector Colinear::differentialOptimized() const
{
    if(isAlreadyColinear())
    {
        return OptimizedVector();
    }

    if(isHorizontal())
    {
        OptimizedVector result;
        if(a->y.samePointer(b->y))
        {
            // a2 - c2 = 0.
            result.set(a->y.getPointer(), 1);
            result.set(c->y.getPointer(), -1);
        } else {
            // a2 - b2 = 0.
            result.set(a->y.getPointer(), 1);
            result.set(b->y.getPointer(), -1);
        }
        return result;
    }

    if(isVertical())
    {
        OptimizedVector result;
        if(a->x.samePointer(b->x))
        {
            // a1 - c1 = 0.
            result.set(a->x.getPointer(), 1);
            result.set(c->x.getPointer(), -1);
        } else {
            // a1 - b1 = 0.
            result.set(a->x.getPointer(), 1);
            result.set(b->x.getPointer(), -1);
        }
        return result;
    }

    return optimizeVector(differentialNonOptimized());
}

bool Colinear::isAlreadyColinear() const
{
    if(a->x.samePointer(b->x) && a->x.samePointer(c->x))
    {
        return true;
    }

    if(a->y.samePointer(b->y) && a->y.samePointer(c->y))
    {
        return true;
    }

    return false;
}

bool Colinear::isHorizontal() const
{
    return (a->y.samePointer(b->y) || a->y.samePointer(c->y) || b->y.samePointer(c->y));
}

bool Colinear::isVertical() const
{
    return (a->x.samePointer(b->x) || a->x.samePointer(c->x) || b->x.samePointer(c->x));
}

void Difference::setProxies(ParameterProxyManager& manager) const
{
    manager.add(&a->x);
    manager.add(&a->y);
    manager.add(&b->x);
    manager.add(&b->y);
    manager.add(&c->x);
    manager.add(&c->y);
}

bool Colinear::optimizeProxies(ParameterProxyManager& manager) const
{
    bool result = false;
    if(isHorizontal())
    {
        result = manager.setEqual(a->y, b->y) || result;
        result = manager.setEqual(a->y, c->y) || result;
    }

    if(isVertical())
    {
        result = manager.setEqual(a->x, b->x) || result;
        result = manager.setEqual(a->x, c->x) || result;
    }
    return result;
}

} // namespace NamedSketcher::GCS
