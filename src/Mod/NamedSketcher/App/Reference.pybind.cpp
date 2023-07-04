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

/*
 * This is not supposed to be here.
 * These python bindings should be in Base::Accessor.
 */

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
namespace py = pybind11;

#include <Base/Accessor/PathToObject.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "gcs_solver/parameters/Parameter.h"
#include "geometries/GeometryPoint.h"
#include "geometries/GeometryLineSegment.h"
#include "constraints/ConstraintBase.h"

#include "Reference.pybind.h"

using namespace Base::Accessor;

namespace NamedSketcher
{

using ref_geometry_base = ReferenceTo<GeometryBase>;
using ref_parameter = ReferenceTo<GCS::Parameter>;
using ref_point = ReferenceTo<GCS::Point>;
using ref_constraint = ReferenceTo<ConstraintBase>;

void init_Reference(py::module& m)
{
    py::class_<PathToObject>(m, "PathToObject", py::module_local())
        .def(py::self + std::string())
    ;

    py::class_<ref_geometry_base, PathToObject>(m, "ReferenceToGeometryBase", py::module_local())
        .def(py::init<const PathToObject&>())
        .def(py::self + std::string())
    ;
    py::implicitly_convertible<PathToObject, ref_geometry_base>();

    py::class_<ref_parameter, PathToObject>(m, "ReferenceToParameter", py::module_local())
        .def(py::init<const PathToObject&>())
        .def(py::self + std::string())
    ;
    py::implicitly_convertible<PathToObject, ref_parameter>();

    py::class_<ref_point, PathToObject>(m, "ReferenceToPoint", py::module_local())
        .def(py::init<const PathToObject&>())
        .def(py::self + std::string())
    ;
    py::implicitly_convertible<PathToObject, ref_point>();

    py::class_<ref_constraint, PathToObject>(m, "ReferenceToConstraint", py::module_local());
}

} //namespace NamedSketcher
