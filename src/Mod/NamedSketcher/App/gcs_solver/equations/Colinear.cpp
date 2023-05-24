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

#include "Colinear.h"

namespace NamedSketcher::GCS
{

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

std::vector<GradientDuplet> Colinear::differentialNonOptimized() const
{
    double a1 = a->x.getValue();
    double a2 = a->y.getValue();
    double b1 = b->x.getValue();
    double b2 = b->y.getValue();
    double c1 = c->x.getValue();
    double c2 = c->y.getValue();

    std::vector<GradientDuplet> result;
    result.reserve(6);
    // TODO: remove comments when we start using C++20.
    result.emplace_back({/*.parameter =*/ &a->x, /*.value =*/ b2 - c2});
    result.emplace_back({/*.parameter =*/ &a->y, /*.value =*/ c1 - b1});
    result.emplace_back({/*.parameter =*/ &b->x, /*.value =*/ c2 - a2});
    result.emplace_back({/*.parameter =*/ &b->y, /*.value =*/ a1 - c1});
    result.emplace_back({/*.parameter =*/ &c->x, /*.value =*/ a2 - b2});
    result.emplace_back({/*.parameter =*/ &c->y, /*.value =*/ b1 - a1});
    return result;
}

std::vector<GradientDuplet> Colinear::differentialOptimized() const
{
    if(isAlreadyColinear())
    {
        return std::vector<GradientDuplet>();
    }

    if(isHorizontal())
    {
        std::vector<GradientDuplet> result;
        result.reserve(2);
        if(a->y.samePointer(b->y))
        {
            // a2 - c2 = 0.
            result.emplace_back({/*.parameter =*/ &a->y, /*.value =*/ 1});
            result.emplace_back({/*.parameter =*/ &c->y, /*.value =*/ -1});
        } else {
            // a2 - b2 = 0.
            result.emplace_back({/*.parameter =*/ &a->y, /*.value =*/ 1});
            result.emplace_back({/*.parameter =*/ &b->y, /*.value =*/ -1});
        }
        return result;
    }

    if(isVertical())
    {
        std::vector<GradientDuplet> result;
        result.reserve(2);
        if(a->x.samePointer(b->x))
        {
            // a1 - c1 = 0.
            result.emplace_back({/*.parameter =*/ &a->x, /*.value =*/ 1});
            result.emplace_back({/*.parameter =*/ &c->x, /*.value =*/ -1});
        } else {
            // a1 - b1 = 0.
            result.emplace_back({/*.parameter =*/ &a->x, /*.value =*/ 1});
            result.emplace_back({/*.parameter =*/ &b->x, /*.value =*/ -1});
        }
        return result;
    }

    return differentialNonOptimized();
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

} // namespace NamedSketcher::GCS
