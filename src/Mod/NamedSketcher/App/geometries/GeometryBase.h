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

#include <Mod/Part/App/TopoShape.h>

#include "GeometryFactory.h"

namespace NamedSketcher
{

class NamedSketcherExport GeometryBase
        : public Base::Persistence
        , public Base::Accessor::NameAndTag
{
    TYPESYSTEM_HEADER();

public:
    using factory = class GeometryFactory;

    bool isConstruction = false;
    bool isBlocked = false;

    virtual TopoDS_Shape toShape() = 0;
    virtual void commitChanges() const = 0;

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
    static constexpr const char* xmlTagNameStatic() {return "Geometry";}
};

/**
 * @brief Convenience template for GeometryBase subclasses.
 */
template<typename MySelf, typename GeoClass>
class NamedSketcherExport GeometryBaseT : public GeometryBase
{
public:
    using reference_type = Base::Accessor::ReferenceTo<MySelf>;

    GeometryBaseT(std::unique_ptr<GeoClass> geo);
    reference_type getReference() const;

    TopoDS_Shape toShape() override {return geometry->toShape();}

    void Save(Base::Writer& writer) const override;
    static std::unique_ptr<GeometryBase> staticRestore(Base::XMLReader& reader);

protected:
    std::shared_ptr<GeoClass> geometry;
    GeometryBaseT();
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_GeometryBase_H
