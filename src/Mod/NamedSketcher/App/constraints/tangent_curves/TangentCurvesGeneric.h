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


#ifndef NAMEDSKETCHER_TangentCurves_Generic_H
#define NAMEDSKETCHER_TangentCurves_Generic_H

#include <memory>
#include <vector>

#include "../../gcs_solver/equations/ConcurrentCurves.h"
#include "../../gcs_solver/equations/ParallelCurves.h"

#include "TangentCurvesBase.h"

namespace NamedSketcher::GCS {
class EquationProxy;
}

namespace NamedSketcher::Specialization
{

/** This is the generic specialization (!?!?) for TangentCurves.
 */
class TangentCurvesGeneric
    : public TangentCurvesBase
{
public:
    TangentCurvesGeneric(GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2,
                         GeometryBase* c1, GeometryBase* c2,
                         GCS::Parameter* parameter_t1, GCS::Parameter* parameter_t2);

    void preprocessParameters() override;
    void setEquations() override;
    void report() const override;

private:
    GeometryBase* curve1;
    GeometryBase* curve2;
    GCS::Parameter* parameter_t1;
    GCS::Parameter* parameter_t2;
    GCS::ConcurrentCurves equationConcurrent;
    GCS::ParallelCurves equationParallel;
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_TangentCurves_Generic_H
