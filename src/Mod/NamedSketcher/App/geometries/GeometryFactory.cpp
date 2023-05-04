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

#include <list>

#include <Mod/Part/App/Geometry.h>

#include "GeometryBase.h"
#include "GeometryPoint.h"
#include "GeometryLineSegment.h"

#include "GeometryFactory.h"

namespace NamedSketcher {

std::unique_ptr<GeometryBase> GeometryFactory(std::unique_ptr<Part::Geometry>&& geo)
{
    if(geo->getTypeId() == Part::GeomPoint::getClassTypeId()) {
        auto g = std::unique_ptr<Part::GeomPoint>(static_cast<Part::GeomPoint*>(geo.release()));
        return std::make_unique<GeometryPoint>(std::move(g));
    }
    if(geo->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
        auto g = std::unique_ptr<Part::GeomLineSegment>(static_cast<Part::GeomLineSegment*>(geo.release()));
        return std::make_unique<GeometryLineSegment>(std::move(g));
    }

    FC_THROWM(Base::NotImplementedError, "Type '" << geo->getTypeId().getName() << "' not supported by NamedSketcher, yet!");
}

void GeometryFactory::getAttributes(Base::XMLReader& reader)
{
    isConstruction =  reader.getAttributeAsBoolean("construction", false);
    isBlocked = reader.getAttributeAsBoolean("blocked", false);
}

void GeometryFactory::setAttributes(GeometryBase* p)
{
    p->isConstruction = isConstruction;
    p->isBlocked = isBlocked;
}

} // namespace NamedSketcher

using namespace NamedSketcher;
template<>
GeometryFactory::map_type Base::ElementFactory<GeometryBase>::factoryMap = {
    {
//        GeometryPoint::xmlTagTypeStatic(),
        "Point",
        [](Base::XMLReader& reader){return GeometryPoint::staticRestore(reader);}
    }
    ,{
//        GeometryLineSegment::xmlTagTypeStatic(),
        "LineSegment",
        [](Base::XMLReader& reader){return GeometryLineSegment::staticRestore(reader);}
    }
};
