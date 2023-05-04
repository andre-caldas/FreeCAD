/***************************************************************************
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
#include <memory>
#endif // _PreComp_

#include <Base/Reader.h>
#include <Base/Writer.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryPoint.h"

namespace NamedSketcher
{

TYPESYSTEM_SOURCE(GeometryPoint, GeometryBaseT<Part::GeomPoint>)

GeometryPoint::GeometryPoint()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::GeometryPoint should not be constructed without arguments.");
}

GeometryPoint::GeometryPoint(std::unique_ptr<Part::GeomPoint>&& geo)
    : GeometryBaseT(std::move(geo))
    , point(geo->getPoint())
    , gcs_point(&point.x, &point.y)
{
}

void GeometryPoint::commitChanges() const
{
    geometry->setPoint(point);
}

void GeometryPoint::appendParameterList(std::vector<double*>& parameters)
{
    parameters.push_back(&point.x);
    parameters.push_back(&point.y);
}


unsigned int GeometryPoint::getMemSize() const
{
    return geometry->getMemSize() + sizeof(*this);
}

} // namespace NamedSketcher
