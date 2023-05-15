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


#ifndef NAMEDSKETCHER_ConstraintXDistance_H
#define NAMEDSKETCHER_ConstraintXDistance_H

#include <type_traits>
#include <vector>
#include <set>
#include <boost/uuid/uuid.hpp>

#include <Base/Vector3D.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "gcs_solver/ParameterGroup.h"

#include "ConstraintBase.h"

namespace NamedSketcher
{

/** Deals with constraints of type XDistance.
 */
class NamedSketcherExport ConstraintXDistance : public ConstraintBase
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

    using ref_type = Base::Accessor::ReferenceTo<Base::Vector3d>;

public:
    ref_type start;
    ref_type end;

    template<typename ref,
             std::enable_if_t<std::is_constructible_v<ref_type, ref>>* = nullptr>
    ConstraintXDistance(GCS::ParameterProxyManager& proxy_manager, ref&& start, ref&& end);

public:
    std::string_view xmlTagType() const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic() {return "XDistance";}

    void appendParameterList(std::vector<double*>& parameters) override;

    // Base::Persistence
    unsigned int getMemSize () const override;
    void Save (Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
    static std::unique_ptr<ConstraintXDistance> staticRestore(Base::XMLReader& reader);

private:
    GCS::ParameterGroup parameterGroup;

private:
    ConstraintXDistance();
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintXDistance_H
