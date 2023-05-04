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

#include <type_traits>
#include <vector>
#include <set>
#include <boost/uuid/uuid.hpp>

#include <Base/Vector3D.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "ConstraintEqual.h"

#include "NamedSketcherGlobal.h"

namespace NamedSketcher
{

/** Deals with constraints of type Horizontal.
 */
class NamedSketcherExport ConstraintHorizontal : public ConstraintEqual
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

    using ref_type = Base::Accessor::ReferenceTo<Base::Vector3d>;

public:
    ConstraintHorizontal(const ref_type& start, const ref_type& end);

public:
    std::string_view xmlTagType() const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic() {return "Horizontal";}

    void appendParameterList(std::vector<double*>& parameters) override;

    // Base::Persistence
    void Save (Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
    static std::unique_ptr<ConstraintHorizontal> staticRestore(Base::XMLReader& reader);

private:
    ConstraintHorizontal();
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintHorizontal_H
