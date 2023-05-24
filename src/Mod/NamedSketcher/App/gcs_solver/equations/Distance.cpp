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

#include "Distance.h"

namespace NamedSketcher::GCS
{

// |a-b|^2 - distance^2 = 0
double Distance::error() const
{
    if(isCoincident())
    {
        return distance->getValue();
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

std::vector<GradientDuplet> Distance::differentialNonOptimized(Shaker& shake) const
{
    std::uniform_real_distribution rand(0.0, 0.1 * shake);

    double a1 = shake(a->x.getValue());
    double a2 = shake(a->y.getValue());
    double b1 = shake(b->x.getValue());
    double b2 = shake(b->y.getValue());

    std::vector<GradientDuplet> result;
    result.reserve(4);
    // TODO: remove comments when we start using C++20.
    result.emplace_back({/*.parameter =*/ &a->x, /*.value =*/ 2*(a1-b1)});
    result.emplace_back({/*.parameter =*/ &a->y, /*.value =*/ 2*(a2-b2)});
    result.emplace_back({/*.parameter =*/ &b->x, /*.value =*/ 2*(b1-a1)});
    result.emplace_back({/*.parameter =*/ &b->y, /*.value =*/ 2*(b2-a2)});
    return result;
}

std::vector<GradientDuplet> Distance::differentialOptimized(Shaker& shake) const
{
    if(isCoincident())
    {
        return std::vector<GradientDuplet>();
    }

    if(isHorizontal())
    {
        // ax - bx - distance = 0.
        double a1 = shake(a->x.getValue());
        double a2 = shake(a->y.getValue());
        double b1 = shake(b->x.getValue());
        double b2 = shake(b->y.getValue());

        std::vector<GradientDuplet> result;
        result.reserve(4);
        // TODO: remove comments when we start using C++20.
        result.emplace_back({/*.parameter =*/ &a->x, /*.value =*/ 2*(a1-b1)});
        result.emplace_back({/*.parameter =*/ &a->y, /*.value =*/ 2*(a2-b2)});
        result.emplace_back({/*.parameter =*/ &b->x, /*.value =*/ 2*(b1-a1)});
        result.emplace_back({/*.parameter =*/ &b->y, /*.value =*/ 2*(b2-a2)});
        return result;
    }

    if(isVertical())
    {
        // ay - by - distance = 0.
        return a->y.getValue() - b->y.getValue() - distance->getValue();
    }

    double a1 = shake(a->x.getValue());
    double a2 = shake(a->y.getValue());
    double b1 = shake(b->x.getValue());
    double b2 = shake(b->y.getValue());
    double d = distance->getValue();
    return (a1-b1)*(a1-b1) + (a2-b2)*(a2-b2) - d*d;

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

    return differentialNonOptimized(shake);
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
