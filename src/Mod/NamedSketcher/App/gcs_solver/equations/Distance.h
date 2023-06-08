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

#include "Equation.h"

namespace NamedSketcher::GCS
{

class NamedSketcherExport Distance : public NonLinearEquation
{
public:
    Distance() = default;
    void set (Point* a, Point* b, Parameter* distance);

    double error(const ParameterProxyManager& manager) const override;
    ParameterVector differentialNonOptimized() const override;
    OptimizedVector differentialOptimized(const ParameterProxyManager& manager) const override;

    void setProxies(ParameterProxyManager& manager) const override;

private:
    Point* a;
    Point* b;
    Parameter* distance;

    bool isCoincident(const ParameterProxyManager& manager) const;
    bool isHorizontal(const ParameterProxyManager& manager) const;
    bool isVertical(const ParameterProxyManager& manager) const;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Distance_H
