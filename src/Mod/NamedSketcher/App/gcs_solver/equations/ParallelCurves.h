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


#ifndef NAMEDSKETCHER_GCS_ParallelCurves_H
#define NAMEDSKETCHER_GCS_ParallelCurves_H

#include <set>

#include "../Types.h"
#include "Equation.h"

namespace NamedSketcher {
class GeometryBase;
}

namespace NamedSketcher::GCS
{

class NamedSketcherExport ParallelCurves : public NonLinearEquation
{
public:
    ParallelCurves() = default;
    void set(GeometryBase* curve1, Parameter* t1, GeometryBase* curve2, Parameter* t2);

    double error(const ParameterGroupManager& manager) const override;
    ParameterVector differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const override;
    OptimizedVector differentialOptimized(const ParameterGroupManager& manager) const override;

    void declareParameters(ParameterGroupManager& manager) const override;

private:
    Parameter* parameter_t1; // parametrization: t --> c(t).
    GeometryBase* curve1;
    Parameter* parameter_t2; // parametrization: t --> c(t).
    GeometryBase* curve2;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParallelCurves_H
