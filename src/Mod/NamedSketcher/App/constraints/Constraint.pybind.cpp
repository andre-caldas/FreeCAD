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

#include <Base/Accessor/PathToObject.h>

#include "ConstraintEqual.h"
#include "ConstraintCoincident.h"
#include "ConstraintHorizontal.h"
#include "ConstraintXDistance.h"

#include "ConstraintBase.h"
#include "Constraint.pybind.h"

using namespace Base::Accessor;

namespace NamedSketcher
{

using ref_parameter = ConstraintBase::ref_parameter;
using ref_point = ConstraintBase::ref_point;
using path = Base::Accessor::PathToObject;

void init_Constraint(py::module& m)
{
    py::class_<ConstraintBase, std::shared_ptr<ConstraintBase>>(m, "Constraint");

    py::class_<ConstraintEqual, std::shared_ptr<ConstraintEqual>, ConstraintBase>(m, "ConstraintEqual")
        .def(py::init<ref_parameter, ref_parameter>())
    ;

    py::class_<ConstraintCoincident, std::shared_ptr<ConstraintCoincident>, ConstraintBase>(m, "ConstraintCoincident")
        .def(py::init<>())
        .def("addPoint", py::overload_cast<const ref_point&>(&ConstraintCoincident::addPoint))
//        .def("addPoint", py::overload_cast<ref_point&&>(&ConstraintCoincident::addPoint))
        .def("removePoint", &ConstraintCoincident::removePoint)
    ;

    py::class_<ConstraintHorizontal, std::shared_ptr<ConstraintHorizontal>, ConstraintBase>(m, "ConstraintHorizontal")
        .def(py::init<ref_point, ref_point>())
        .def(py::init<const path&>())
    ;

    py::class_<ConstraintXDistance, std::shared_ptr<ConstraintXDistance>, ConstraintBase>(m, "ConstraintXDistance")
            .def(py::init<ref_point, ref_point, ref_parameter>())
    ;
}

} //namespace NamedSketcher
