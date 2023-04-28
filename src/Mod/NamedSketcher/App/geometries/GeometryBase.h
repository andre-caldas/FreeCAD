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


#ifndef NAMEDSKETCHER_GeometryBase_H
#define NAMEDSKETCHER_GeometryBase_H

#include "NamedSketcherGlobal.h"

#include <memory>
#include <string>

#include <Base/Persistence.h>
#include <Base/Accessor/NameAndTag.h>
#include <Base/Accessor/ReferenceToObject.h>

namespace Part {
class Geometry;
}

namespace NamedSketcher
{

class NamedSketcherExport GeometryBase
        : public Base::Persistence
        , public Base::Accessor::NameAndTag
{
    TYPESYSTEM_HEADER();

public:
    GeometryBase(std::unique_ptr<Part::Geometry> geo);

    static std::unique_ptr<GeometryBase> factory(Base::XMLReader& reader);

    bool isConstruction = false;
    bool isBlocked = false;

    virtual void commitChanges() const = 0;

    /*!
     * \brief vector of parameters, as used by the GCS solver.
     * \return a vector representing all points and
     * all parameters of this geometry (e.g.: radius).
     */
    virtual void appendParameterList(std::vector<double*>& parameters) = 0;

    /*!
     * \brief Methods derived from \class GeometryBase shall not implement
     * Persistence::Restore. Restore is done by factory().
     * \param reader
     */
    void Restore(Base::XMLReader& reader) override;
    void Save (Base::Writer& writer) const override;
    std::string xmlAttributes() const;
    virtual const char* xmlTagName() const = 0;

protected:
    std::shared_ptr<Part::Geometry> geometry;
};

template<typename GeoClass>
class NamedSketcherExport GeometryBaseT : public GeometryBase
{
    using reference_type = Base::Accessor::ReferenceTo<GeometryBaseT<GeoClass>>;

public:
    using GeometryBase::GeometryBase;
    GeoClass& getGeometry(void);
    reference_type getReference() const;
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_GeometryBase_H
