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


#ifndef SKETCHER_GeometryFactory_H
#define SKETCHER_GeometryFactory_H

#include <memory>

#include <Base/ElementFactory.h>

namespace Part {
class Geometry;
}

namespace NamedSketcher
{

class GeometryBase;

std::unique_ptr<GeometryBase> geometryFactory(std::unique_ptr<Part::Geometry>&& geo);

class GeometryFactory : public Base::ElementFactory<GeometryBase>
{
protected:
    void getAttributes(Base::XMLReader& reader) override;
    void setAttributes(GeometryBase* p) override;

private:
    bool isConstruction;
    bool isBlocked;
};

} // namespace NamedSketcher

#endif // SKETCHER_GeometryFactory_H
