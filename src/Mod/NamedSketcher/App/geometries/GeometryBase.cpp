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
#include <Base/Writer.h>

#include "GeometryBase.h"

namespace NamedSketcher {

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

std::vector<GCS::Parameter*> GeometryBase::getParameters()
{
    std::vector<const GCS::Parameter*> parameters = const_cast<const GeometryBase*>(this)->getParameters();
    std::vector<GCS::Parameter*> result;
    result.reserve(parameters.size());
    for(auto parameter: parameters)
    {
        result.push_back(const_cast<GCS::Parameter*>(parameter));
    }
    return result;
}

GCS::Point GeometryBase::normalAtParameter(const GCS::ParameterValueMapper& value_mapper, const GCS::Parameter* t) const
{
    // TODO: move this magic number somewhere else.
    const double delta = 1.0 / (1024*1024*8);

    auto c0 = positionAtParameter({value_mapper, t, -delta/2}, t);
    auto c1 = positionAtParameter({value_mapper, t, -delta/2}, t);

    // Non-normalized tangent vector (speed).
    double dx = (c1.x - c0.x) / delta;
    double dy = (c1.y - c0.y) / delta;
    // Rotate clockwise.
    return GCS::Point(dy,-dx).normalize();
}

template<decltype(&GeometryBase::positionAtParameter) func>
void GeometryBase::partialDerivatives(const GCS::ParameterValueMapper& value_mapper, derivative_map& map, const GCS::Parameter* t) const
{
    // TODO: move this magic number somewhere else.
    const double delta = 1.0 / (1024*1024);

    auto parameters = getParameters();
    parameters.push_back(t);
    for(auto parameter: parameters)
    {
        if(map.count(parameter) == 0)
        {
            auto c0 = (this->*func)({value_mapper, parameter, -delta/2}, t);
            auto c1 = (this->*func)({value_mapper, parameter, delta/2}, t);
            double dx = (c1.x - c0.x) / delta;
            double dy = (c1.y - c0.y) / delta;
            map.try_emplace(parameter, dx, dy);
        }
    }
}

void GeometryBase::partialDerivativesPoint(const GCS::ParameterValueMapper& value_mapper, derivative_map& map, const GCS::Parameter* t) const
{
    partialDerivatives<&GeometryBase::positionAtParameter>(value_mapper, map, t);
}

void GeometryBase::partialDerivativesNormal(const GCS::ParameterValueMapper& value_mapper, derivative_map& map, const GCS::Parameter* t) const
{
    partialDerivatives<&GeometryBase::normalAtParameter>(value_mapper, map, t);
}

} // namespace NamedSketcher
