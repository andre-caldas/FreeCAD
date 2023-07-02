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


#ifndef NAMEDSKETCHER_ConstraintBase_H
#define NAMEDSKETCHER_ConstraintBase_H

#include "NamedSketcherGlobal.h"

#include <memory>
#include <string>

#include <Base/Accessor/NameAndTag.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "../gcs_solver/parameters/Parameter.h"
#include "ConstraintFactory.h"

namespace Base {
class Writer;
}

namespace NamedSketcher
{

class GeometryPoint;
class GeometryLineSegment;

namespace GCS
{
class Equation;
class ParameterGroupManager;
}

class NamedSketcherExport ConstraintBase
        : public Base::Accessor::NameAndTag
{
public:
    using ref_parameter = Base::Accessor::ReferenceTo<GCS::Parameter>;
    using ref_point = Base::Accessor::ReferenceTo<GeometryPoint>;
    using ref_line_segment = Base::Accessor::ReferenceTo<GeometryLineSegment>;

    using factory = ConstraintFactory;

    /**
     * @brief Vector of @class Equation, as used by the GCS solver
     * for under / over constraining evaluation.
     * @return A vector representing the minimum set of @class Equation
     * that constraints the geometric objects parameters when non-proxied.
     */
    virtual std::vector<GCS::Equation*> getEquations() = 0;

    /**
     * @brief Asks ReferenceTo<T>'s to be updated.
     * @return Has any reference changed?
     */
    virtual bool updateReferences() = 0;

    virtual unsigned int getMemSize () const = 0;
    virtual void Save (Base::Writer& writer) const = 0;

    void SaveHead(Base::Writer& writer) const;
    void SaveTail(Base::Writer& writer) const;
    virtual std::string xmlAttributes() const;
    virtual std::string_view xmlTagType() const = 0;
    std::string_view xmlTagName() const {return xmlTagNameStatic();}
    static constexpr const char* xmlTagNameStatic() {return "Constraint";}

    virtual ~ConstraintBase() {}
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintBase_H
