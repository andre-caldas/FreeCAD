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


#ifndef SKETCHER_GeometryCircle_H
#define SKETCHER_GeometryCircle_H

#include <memory>

#include <Base/Vector3D.h>

#include "gcs_solver/parameters/Parameter.h"
#include "GeometryBase.h"

namespace Base {
class Reader;
class Writer;
}
namespace Part {
class GeomCircle;
}

namespace NamedSketcher
{

/**
 * @brief Sketcher geometry structure that represents one point.
 */
class NamedSketcherExport GeometryCircle
        : public GeometryBaseT<GeometryCircle, Part::GeomCircle>
        , public Base::Accessor::IExport<GCS::Point>
{
public:
    GeometryCircle(std::unique_ptr<Part::GeomCircle>&& geo);

    void commitChanges() const override;

    // Base::Persistence
    unsigned int getMemSize () const override;
    std::string_view xmlTagType(void) const override {return xmlTagTypeStatic();}
    static constexpr const char* xmlTagTypeStatic(void) {return "Circle";}

    std::vector<const GCS::Parameter*> getParameters() const override {return {&center.x,&center.y,&radius};}
    GCS::Point positionAtParameter(const GCS::ParameterValueMapper& value_mapper, const GCS::Parameter* t) const override;
    GCS::Point normalAtParameter(const GCS::ParameterValueMapper& value_mapper, const GCS::Parameter* t) const override;

    void report() const override;

private:
    GCS::Point center{"center"};
    GCS::Parameter radius{"radius"};

    using token_iterator = IExport<GCS::Point>::token_iterator;
    GCS::Point* resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Point*) override;
    GCS::Parameter* resolve_ptr(token_iterator& start, const token_iterator& end, GCS::Parameter*) override;
};

} // namespace NamedSketcher

#endif // SKETCHER_GeometryCircle_H
