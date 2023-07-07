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


#ifndef NAMEDSKETCHER_ConstraintConstant_H
#define NAMEDSKETCHER_ConstraintConstant_H

#include <type_traits>
#include <memory>

#include "../gcs_solver/equations/Constant.h"
#include "ConstraintBase.h"

namespace NamedSketcher
{

/** Deals with constraints of type Constant.
 */
class NamedSketcherExport ConstraintConstant : public ConstraintBase
{
public:
    ref_parameter a;

    ConstraintConstant(ref_parameter a, double value);

    std::vector<GCS::Equation*> getEquations() override;
    bool updateReferences() override;

    std::string_view xmlTagType() const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic() {return "Constant";}

    // Base::Persistence
    unsigned int getMemSize () const override;
    void Save (Base::Writer& writer) const override;
    static std::unique_ptr<ConstraintConstant> staticRestore(Base::XMLReader& reader);

    void report() const override;

private:
    GCS::Parameter k;
    GCS::Constant equation;
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintConstant_H
