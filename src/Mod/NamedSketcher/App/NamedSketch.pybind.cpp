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

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <Mod/Part/App/Geometry.h>
#include "geometries/Geometry.pybind.h"

#include "NamedSketch.h"
#include "NamedSketch.pybind.h"

namespace NamedSketcher
{

PropertyGeometryList::item_reference
addPart(NamedSketch& sketch, py::object* geo)
{
    return sketch.addPart(pyObjectToPartGeometry(geo));
}

void init_NamedSketch(py::module& m)
{
    py::class_<NamedSketch>(m, "NamedSketch")
        .def(py::init<>())
        .def("addPart", &addPart)
        .def("addGeometry", &NamedSketch::addGeometry)
        .def("delGeometry", &NamedSketch::delGeometry)
        .def("addConstraint", &NamedSketch::addConstraint)
        .def("delConstraint", &NamedSketch::delConstraint)
        .def("solve", &NamedSketch::solve)
        .def("report", &NamedSketch::report)
    ;
}

} //namespace NamedSketcher
