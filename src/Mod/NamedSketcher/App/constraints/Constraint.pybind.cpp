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
#include "ConstraintConstant.h"
#include "ConstraintCoincident.h"
#include "ConstraintHorizontal.h"
#include "ConstraintVertical.h"
#include "ConstraintBlockPoint.h"
#include "ConstraintPointSymmetric.h"
#include "ConstraintXDistance.h"
#include "ConstraintYDistance.h"
#include "ConstraintPointAlongCurve.h"
#include "ConstraintTangentCurves.h"

#include "ConstraintBase.h"
#include "Constraint.pybind.h"

using namespace Base::Accessor;

namespace NamedSketcher
{

using ref_parameter = ConstraintBase::ref_parameter;
using ref_point = ConstraintBase::ref_point;
using ref_geometry = ConstraintBase::ref_geometry;
using path = Base::Accessor::PathToObject;

void init_Constraint(py::module& m)
{
    py::class_<ConstraintBase, std::shared_ptr<ConstraintBase>>(m, "Constraint");

    py::class_<ConstraintEqual, std::shared_ptr<ConstraintEqual>, ConstraintBase>(m, "ConstraintEqual")
        .def(py::init<ref_parameter, ref_parameter>())
    ;

    py::class_<ConstraintConstant, std::shared_ptr<ConstraintConstant>, ConstraintBase>(m, "ConstraintConstant")
        .def(py::init<ref_parameter, double>())
    ;

    py::class_<ConstraintCoincident, std::shared_ptr<ConstraintCoincident>, ConstraintBase>(m, "ConstraintCoincident")
        .def(py::init<>())
        .def(py::init<ref_point, ref_point>())
        .def("addPoint", py::overload_cast<ref_point>(&ConstraintCoincident::addPoint))
        .def("removePoint", &ConstraintCoincident::removePoint)
    ;

    py::class_<ConstraintHorizontal, std::shared_ptr<ConstraintHorizontal>, ConstraintBase>(m, "ConstraintHorizontal")
        .def(py::init<ref_point, ref_point>())
        .def(py::init<const path&>())
    ;

    py::class_<ConstraintVertical, std::shared_ptr<ConstraintVertical>, ConstraintBase>(m, "ConstraintVertical")
        .def(py::init<ref_point, ref_point>())
        .def(py::init<const path&>())
    ;

    py::class_<ConstraintBlockPoint, std::shared_ptr<ConstraintBlockPoint>, ConstraintBase>(m, "ConstraintBlockPoint")
        .def(py::init<ref_point, double, double>())
    ;

    py::class_<ConstraintPointSymmetric, std::shared_ptr<ConstraintPointSymmetric>, ConstraintBase>(m, "ConstraintPointSymmetric")
        .def(py::init<ref_point, ref_point, ref_point>())
    ;

    py::class_<ConstraintXDistance, std::shared_ptr<ConstraintXDistance>, ConstraintBase>(m, "ConstraintXDistance")
        .def(py::init<ref_point, ref_point, double>())
        .def(py::init<const path&, double>())
    ;

    py::class_<ConstraintYDistance, std::shared_ptr<ConstraintYDistance>, ConstraintBase>(m, "ConstraintYDistance")
        .def(py::init<ref_point, ref_point, double>())
        .def(py::init<const path&, double>())
    ;

    py::class_<ConstraintPointAlongCurve, std::shared_ptr<ConstraintPointAlongCurve>, ConstraintBase>(m, "ConstraintPointAlongCurve")
            .def(py::init<ref_point, ref_geometry>())
    ;

    py::class_<ConstraintTangentCurves, std::shared_ptr<ConstraintTangentCurves>, ConstraintBase>(m, "ConstraintTangentCurves")
            .def(py::init<ref_geometry, ref_geometry>())
    ;
}

} //namespace NamedSketcher
