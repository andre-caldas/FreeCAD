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

#include <Base/Exception.h>
#include <Base/Reader.h>
#include <Base/Writer.h>

#include "GeometryFactory.h"
#include "GeometryBase.h"

namespace NamedSketcher {

template<typename MySelf, typename GeoClass>
GeometryBaseT<MySelf, GeoClass>::GeometryBaseT()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::GeometryBaseT should not be constructed without arguments. This is a bug!");
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
    auto geo = std::make_unique<GeoClass>();
    geo->Restore(reader);
    return std::make_unique<MySelf>(std::move(geo));
}

} // namespace NamedSketcher
