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


#ifndef NAMEDSKETCHER_GCS_OrthogonalDisplacement_H
#define NAMEDSKETCHER_GCS_OrthogonalDisplacement_H

#include "Equation.h"

namespace NamedSketcher::GCS
{

class NamedSketcherExport OrthogonalDisplacement : public NonLinearEquation
{
public:
    OrthogonalDisplacement() = default;
    void set(Point* start, Point* end, Point* displaced_point, std::vector<std::pair<double,Parameter*>> displacement_combinations);
    void set(Point* start, Point* end, Point* displaced_point, Parameter* displacement);

    double error(const ParameterGroupManager& manager) const override;
    ParameterVector differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const override;
    OptimizedVector differentialOptimized(const ParameterGroupManager& manager) const override;

    void declareParameters(ParameterGroupManager& manager) const override;

    void report() const override;

private:
    Point* start;
    Point* end;
    Point* displaced_point;
    // Distance is the linear combination: x1 d1 + ... + xn dn.
    std::vector<std::pair<double,Parameter*>> displacement_combinations;

    bool isCoincident(const ParameterGroupManager& manager) const;
    bool isHorizontal(const ParameterGroupManager& manager) const;
    bool isVertical(const ParameterGroupManager& manager) const;

    double totalDisplacement(const ParameterValueMapper& parameter_mapper) const;
    void setDisplacementDifferentials(const ParameterGroupManager& manager, OptimizedVector& result, double factor=1.0) const;
    void setDisplacementDifferentials(const ParameterValueMapper& parameter_mapper, ParameterVector& result, double factor=1.0) const;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_OrthogonalDisplacement_H
