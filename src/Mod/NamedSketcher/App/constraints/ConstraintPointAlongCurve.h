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


#ifndef NAMEDSKETCHER_ConstraintPointAlongCurve_H
#define NAMEDSKETCHER_ConstraintPointAlongCurve_H

#include <memory>
#include <vector>

#include "../gcs_solver/equations/PointAlongCurve.h"
#include "ConstraintBase.h"

namespace Base::Accessor {
template<typename T>
class ReferenceTo;
}

namespace NamedSketcher
{

class GeometryBase;

/** Deals with constraints of type PointAlongCurve.
 */
class NamedSketcherExport ConstraintPointAlongCurve : public ConstraintBase
{
public:
    ref_point point;
    ref_geometry curve;

    ConstraintPointAlongCurve(ref_point point, ref_geometry curve);

    std::vector<GCS::Equation*> getEquations() override;
    bool updateReferences() override;

    std::string_view xmlTagType() const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic() {return "PointAlongCurve";}

    // Base::Persistence
    unsigned int getMemSize () const override;
    void Save (Base::Writer& writer) const override;
    static std::unique_ptr<ConstraintPointAlongCurve> staticRestore(Base::XMLReader& reader);

    void report() const override;

private:
    // TODO: Use colinear for line segments.
    GCS::PointAlongCurve equation;
    GCS::Parameter parameter_t{"t", 0}; // parametrization: t --> c(t).
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintPointAlongCurve_H
