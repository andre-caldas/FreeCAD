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


#ifndef NAMEDSKETCHER_ConstraintHorizontal_H
#define NAMEDSKETCHER_ConstraintHorizontal_H

#include <memory>
#include <vector>

#include "../gcs_solver/equations/Equal.h"
#include "ConstraintBase.h"

namespace Base::Accessor {
template<typename T>
class ReferenceTo;
}

namespace NamedSketcher
{

class GeometryBase;

/** Deals with constraints of type Horizontal.
 */
class NamedSketcherExport ConstraintHorizontal : public ConstraintBase
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    ref_point start;
    ref_point end;

    template<class ref,
             std::enable_if_t<std::is_constructible_v<ref_point, ref>>* = nullptr>
    ConstraintHorizontal(ref&& a, ref&& b);

public:
    std::vector<GCS::Equation*> getEquations() override;

    std::string_view xmlTagType() const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic() {return "Horizontal";}

    // Base::Persistence
    unsigned int getMemSize () const override;
    void Save (Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
    static std::unique_ptr<ConstraintHorizontal> staticRestore(Base::XMLReader& reader);

private:
    GCS::Equal equation;

public: // :-(
    ConstraintHorizontal();
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintHorizontal_H
