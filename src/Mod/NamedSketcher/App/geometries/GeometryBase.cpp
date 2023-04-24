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
#include <Base/Persistence.h>
#endif // _PreComp_

#include "GeometryFactory.h"
#include "GeometryBase.h"

namespace App::NamedSketcher {

TYPESYSTEM_SOURCE_ABSTRACT(GeometryBase, Base::Persistence)

GeometryBase::GeometryBase(std::unique_ptr<Part::Geometry> geo)
    : geometry(std::move(geo))
{
}

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

static std::unique_ptr<GeometryBase> GeometryBase::factory(Base::XMLReader& reader)
{
    return GeometryFactory(reader);
}

void GeometryBase::Restore(Base::XMLReader& /*reader*/)
{
    FC_THROWM(Base::NotImplementedError, "Restore is provided by the GeometryBase::factory");
}

template<typename GeoClass>
GeoClass& GeometryBaseT<GeoClass>::getGeometry(void)
{
    if(!geometry)
    {
        FC_THROWM(Base::RuntimeError, "Geometry not initialized. GeometryBase::geometry is null.");
    }
    return static_cast<GeoClass&>(*geometry);
}

} // namespace NamedSketcher
