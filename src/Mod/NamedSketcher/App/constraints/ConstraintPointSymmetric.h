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


#ifndef NAMEDSKETCHER_ConstraintPointSymmetric_H
#define NAMEDSKETCHER_ConstraintPointSymmetric_H

#include <type_traits>
#include <memory>

#include "../gcs_solver/equations/MediumParameter.h"
#include "ConstraintBase.h"

namespace NamedSketcher
{

/** Deals with constraints of type PointSymmetric.
 */
class NamedSketcherExport ConstraintPointSymmetric : public ConstraintBase
{
public:
    ref_point a;
    ref_point o;
    ref_point b;

    ConstraintPointSymmetric(ref_point a, ref_point o, ref_point b);

    std::vector<GCS::Equation*> getEquations() override;
    bool updateReferences() override;

    std::string_view xmlTagType() const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic() {return "PointSymmetric";}

    // Base::Persistence
    unsigned int getMemSize () const override;
    void Save (Base::Writer& writer) const override;
    static std::unique_ptr<ConstraintPointSymmetric> staticRestore(Base::XMLReader& reader);

    void report() const override;

private:
    GCS::MediumParameter equationX;
    GCS::MediumParameter equationY;
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintPointSymmetric_H
