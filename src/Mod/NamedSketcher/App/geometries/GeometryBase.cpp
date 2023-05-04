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

#include "PreCompiled.h"

#ifndef _PreComp_
#include <string>
#include <Base/Exception.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Persistence.h>
#include <Mod/Part/App/Geometry.h>
#endif // _PreComp_

#include "GeometryFactory.h"
#include "GeometryBase.h"

namespace NamedSketcher {

TYPESYSTEM_SOURCE_ABSTRACT(GeometryBase, Base::Persistence)

std::string GeometryBase::xmlAttributes() const
{
    std::string result;
    if(isBlocked)
    {
        result += " blocked='true'";
    }
    if(isConstruction)
    {
        result += " construction='true'";
    }
    return result;
}

void GeometryBase::Restore(Base::XMLReader& /*reader*/)
{
    FC_THROWM(Base::NotImplementedError, "Restore is provided by the GeometryBase::factory");
}

void GeometryBase::SaveHead(Base::Writer& writer) const
{
    writer.Stream() << writer.ind() << "<" << xmlTagName()
                    << " type=\"" << xmlTagType() << "\""
                    << xmlAttributes() << ">" << std::endl;
    writer.incInd();
}

void GeometryBase::SaveTail(Base::Writer& writer) const
{
    writer.decInd();
    writer.Stream() << writer.ind() << "</" << xmlTagName() << ">" << std::endl;
}

template<typename MySelf, typename GeoClass>
GeometryBaseT<MySelf, GeoClass>::GeometryBaseT()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::Exception, "NamedSketcher::GeometryBaseT should not be constructed without arguments.");
}

template<typename MySelf, typename GeoClass>
GeometryBaseT<MySelf, GeoClass>::GeometryBaseT(std::unique_ptr<GeoClass> geo)
    : geometry(std::move(geo))
{
}

template<typename MySelf, typename GeoClass>
void GeometryBaseT<MySelf, GeoClass>::Save(Base::Writer& writer) const
{
    SaveHead(writer);
    geometry->Save(writer);
    SaveTail(writer);
}

template<typename MySelf, typename GeoClass>
std::unique_ptr<GeometryBase>
GeometryBaseT<MySelf, GeoClass>::staticRestore(Base::XMLReader& reader)
{
    auto geo = std::make_unique<GeoClass>(reader);
    geo->Restore(reader);
    return std::make_unique<MySelf>(geo);
}

} // namespace NamedSketcher
