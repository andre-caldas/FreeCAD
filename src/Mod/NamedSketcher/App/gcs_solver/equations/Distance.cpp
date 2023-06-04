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

#include "../ParameterProxyManager.h"
#include "Distance.h"

namespace NamedSketcher::GCS
{

Distance::set(ProxiedPoint* x, ProxiedPoint* y, ProxiedParameter* d)
{
    if(x == y)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.")
    }
    a = x;
    b = y;
    distance = d;
}

// |a-b|^2 - distance^2 = 0
double Distance::error() const
{
    if(isCoincident())
    {
        // -distance = 0
        return -distance->getValue();
    }

    if(isHorizontal())
    {
        // ax - bx - distance = 0.
        return a->x.getValue() - b->x.getValue() - distance->getValue();
    }

    if(isVertical())
    {
        // ay - by - distance = 0.
        return a->y.getValue() - b->y.getValue() - distance->getValue();
    }

    double a1 = a->x.getValue();
    double a2 = a->y.getValue();
    double b1 = b->x.getValue();
    double b2 = b->y.getValue();
    double d = distance->getValue();
    return (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) - d*d;
}

ParameterVector Distance::differentialNonOptimized() const
{
    double a1 = *a->x;
    double a2 = *a->y;
    double b1 = *b->x;
    double b2 = *b->y;

    Vector result;
    result.set(a->x, 2*(a1-b1));
    result.set(a->y, 2*(a2-b2));
    result.set(b->x, 2*(b1-a1));
    result.set(b->y, 2*(b2-a2));
    return result;
}

OptimizedVector Distance::differentialOptimized(ParameterProxyManager& manager) const
{
    if(isCoincident(manager))
    {
        return OptimizedVector();
    }

    if(isHorizontal(manager))
    {
        // ax - bx - distance = 0.
        OptimizedVector result;
        result.set(manager.getPointer(a->x), 1);
        result.set(manager.getPointer(b->x), -1);
        return result;
    }

    if(isVertical(manager))
    {
        // ay - by - distance = 0.
        OptimizedVector result;
        result.set(manager.getPointer(a->y), 1);
        result.set(manager.getPointer(b->y), -1);
        return result;
    }

    return optimizeVector(differentialNonOptimized());
}

void Distance::setProxies(ParameterProxyManager& manager) const
{
    manager.add(&a->x);
    manager.add(&a->y);
    manager.add(&b->x);
    manager.add(&b->y);
}

bool Distance::isCoincident() const
{
    return (isHorizontal() && isVertical());
}

bool Distance::isHorizontal() const
{
    return a->y.samePointer(b->y);
}

bool Distance::isVertical() const
{
    return a->x.samePointer(b->x);
}

} // namespace NamedSketcher::GCS
