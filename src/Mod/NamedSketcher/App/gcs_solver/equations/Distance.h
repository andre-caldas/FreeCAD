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


#ifndef NAMEDSKETCHER_GCS_Distance_H
#define NAMEDSKETCHER_GCS_Distance_H

#include <set>

#include "../ProxiedParameter.h""
#include "../Types.h"
#include "Equation.h"

namespace NamedSketcher::GCS
{

class NamedSketcherExport Distance : public Equation
{
public:
    Distance(ProxiedPoint* a, ProxiedPoint* b, ProxiedParameter* distance)
        : a(a)
        , b(b)
        , distance(distance)
    {assert(a != b);}

    double error() const override;
    std::vector<GradientDuplet> differentialNonOptimized(Shaker& shake) const override;
    std::vector<GradientDuplet> differentialOptimized(Shaker& shake) const override;

private:
    ProxiedPoint* a;
    ProxiedPoint* b;
    ProxiedParameter* distance;

    bool isConicident() const;
    bool isHorizontal() const;
    bool isVertical() const;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Distance_H
