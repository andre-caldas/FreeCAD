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


#ifndef NAMEDSKETCHER_TangentCurves_LineLine_H
#define NAMEDSKETCHER_TangentCurves_LineLine_H

#include <memory>
#include <vector>

#include "../../gcs_solver/equations/Colinear.h"

#include "TangentCurvesBase.h"

namespace NamedSketcher {
class GeometryLineSegment;
}
namespace NamedSketcher::GCS {
class EquationProxy;
}

namespace NamedSketcher::Specialization
{

/** This is the line-to-line specialization for TangentCurves.
 */
class TangentCurvesLineLine
    : public TangentCurvesBase
{
public:
    // We could simply subclass the "generic" class and make the parameters constant.
    TangentCurvesLineLine(GCS::EquationProxy& proxy1, GCS::EquationProxy& proxy2, GeometryLineSegment* l1, GeometryLineSegment* l2);

    void preprocessParameters() override;
    void setEquations() override;
    void report() const override;

private:
    GeometryLineSegment* line1;
    GeometryLineSegment* line2;
    GCS::Colinear equationColinearPoints1;
    GCS::Colinear equationColinearPoints2;
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_TangentCurves_LineLine_H
