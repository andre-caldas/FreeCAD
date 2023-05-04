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

#include <Base/Persistence.h>
#include <Base/Accessor/NameAndTag.h>

#include "ConstraintFactory.h"

namespace NamedSketcher
{

class NamedSketcherExport ConstraintBase
        : public Base::Persistence
        , public Base::Accessor::NameAndTag
{
    TYPESYSTEM_HEADER();

public:
    using factory = class ConstraintFactory;
    bool isDriving = true;
    bool isDriven = false;

    /*!
     * \brief vector of parameters, as used by the GCS solver.
     * \return a vector representing all points and
     * all parameters of this Constraint (e.g.: radius).
     */
    virtual void appendParameterList(std::vector<double*>& parameters) = 0;

    /*!
     * \brief Methods derived from \class GeometryBase shall not implement
     * Persistence::Restore. Restore is done by factory().
     * \param reader
     */
    void Restore(Base::XMLReader& reader) override;
    void SaveHead(Base::Writer& writer) const;
    void SaveTail(Base::Writer& writer) const;
    virtual std::string xmlAttributes() const;
    virtual std::string_view xmlTagType() const = 0;
    std::string_view xmlTagName() const {return xmlTagNameStatic();}
    static constexpr const char* xmlTagNameStatic() {return "Constraint";}
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintBase_H
