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

#ifndef SKETCHER_GeometryPoint_H
#define SKETCHER_GeometryPoint_H

#include <memory>

#include <Base/Vector3D.h>
#include <Mod/Sketcher/App/planegcs/Geo.h>

#include "GeometryBase.h"

namespace Part {
class GeomPoint;
}

namespace NamedSketcher
{

/** Sketcher geometry structure that represents one point.
 */
class NamedSketcherExport GeometryPoint
        : public GeometryBaseT<Part::GeomPoint>
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    GeometryPoint(std::unique_ptr<Part::GeomPoint>&& geo);

    void commitChanges() const override;
    void appendParameterList(std::vector<double*>& parameters) override;

    // Base::Persistence
    unsigned int getMemSize () const override;

    const char* xmlTagName() const override {return xmlTagNameStatic();}
    static const char* xmlTagNameStatic() {return "GeometryPoint";}

private:
    Base::Vector3d point;
    GCS::Point gcs_point;
};

} // namespace NamedSketcher

#endif // SKETCHER_GeometryPoint_H
