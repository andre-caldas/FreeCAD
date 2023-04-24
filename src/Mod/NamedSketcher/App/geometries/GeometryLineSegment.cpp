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
#endif // _PreComp_

#include <Base/Persistence.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Exception.h>

#include <Mod/Part/App/Geometry.h>

#include "GeometryLineSegment.h"

namespace App::NamedSketcher
{

TYPESYSTEM_SOURCE(GeometryLineSegment, GeometryBaseT<Part::GeomLineSegment>)

GeometryLineSegment::GeometryLineSegment(std::shared_ptr<Part::GeomLineSegment> &geo)
    : GeometryBaseT(geo)
    , start(geo->getStartPoint())
    , end(geo->getEndPoint())
    , gcs_start(&start.x, &start.y)
    , gcs_end(&end.x, &end.y)
{
    gcs_line.p1 = gcs_start;
    gcs_line.p2 = gcs_end;
}

void GeometryLineSegment::commitChanges() const
{
    getGeometry().setPoints(start, end);
}

void GeometryLineSegment::appendParameterList(std::vector<double*>& parameters)
{
    auto& line = getGeometry();
    parameters.push_back(line.getStartPoint().x);
    parameters.push_back(line.getStartPoint().y);
    parameters.push_back(line.getEndPoint().x);
    parameters.push_back(line.getEndPoint().y);
}


unsigned int GeometryPoint::getMemSize () const
{
    return geometry->getMemSize() + sizeof(*this);
}

} // namespace App::NamedSketcher
