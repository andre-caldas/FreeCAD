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


#ifndef SKETCHER_GeometryLineSegment_H
#define SKETCHER_GeometryLineSegment_H

#include <memory>

#include <Base/Vector3D.h>

#include "gcs_solver/parameters/Parameter.h"
#include "GeometryBase.h"

namespace Base {
class Reader;
class Writer;
}
namespace Part {
class GeomLineSegment;
}

namespace NamedSketcher
{

/**
 * @brief Sketcher geometry structure that represents one point.
 */
class NamedSketcherExport GeometryLineSegment
        : public GeometryBaseT<GeometryLineSegment, Part::GeomLineSegment>
{
public:
    GeometryLineSegment(std::unique_ptr<Part::GeomLineSegment>&& geo);

    void commitChanges() const override;

    // Base::Persistence
    unsigned int getMemSize () const override;
    std::string_view xmlTagType(void) const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic(void) {return "LineSegment";}

private:
    GCS::Point start;
    GCS::Point end;
};

} // namespace NamedSketcher

#endif // SKETCHER_GeometryLineSegment_H
