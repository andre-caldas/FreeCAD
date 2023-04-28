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
#include <memory>
#include <utility>
#endif // _PreComp_

#include <Base/Exception.h>
#include <Mod/Part/App/Geometry.h>

#include "GeometryBase.h"
#include "GeometryPoint.h"
#include "GeometryLineSegment.h"


namespace NamedSketcher {

std::unique_ptr<GeometryBase> GeometryFactory(std::unique_ptr<Part::Geometry> geo)
{
    if(geo->getTypeId() == Part::GeomPoint::getClassTypeId()) {
        auto g = static_cast<Part::GeomPoint*>(geo.release());
        return std::unique_ptr<GeometryBase>(new GeometryPoint(std::unique_ptr(g)));
    }
    if(geo->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
        auto g = static_cast<Part::GeomLineSegment*>(geo);
        return std::unique_ptr<GeometryBase>(new GeometryLineSegment(std::unique_ptr(g)));
    }

    FC_THROWM(Base::NotImplementedError, "Type '" << geo->getTypeId().getName() << "' not supported by NamedSketcher, yet!");
}

std::unique_ptr<GeometryBase> GeometryFactory(Base::XMLReader& reader)
{
    std::unique_ptr<Part::Geometry> geo;
    std::unique_ptr<GeometryBase> result;

    reader.pickElement();
    bool isConstruction = reader.getAttributeAsBoolean("construction", false);
    bool isBlocked = reader.getAttributeAsBoolean("blocked", false);

    if(reader.testElement(GeometryPoint::xmlTagNameStatic()))
    {
        geo.reset(new Part::GeomPoint());
        geo->Restore(reader);
        result.reset(new GeometryPoint(std::move(geo)));
    }
    else if(reader.testElement(GeometryLineSegment::xmlTagNameStatic()))
    {
        geo.reset(new Part::GeomLineSegment());
        geo->Restore(reader);
        result.reset(new GeometryLineSegment(geo));
    }
    else
    {
        FC_THROWM(Base::NotImplementedError, "Type '" << reader.localName() << "' not supported by NamedSketcher, yet!");
    }

    result->isConstruction = isConstruction;
    result->isBlocked = isBlocked;
    return result;
}

} // namespace NamedSketcher
