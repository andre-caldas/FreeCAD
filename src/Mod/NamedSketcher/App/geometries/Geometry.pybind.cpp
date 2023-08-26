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
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

#include <stdexcept>

#include <Mod/Part/App/Geometry.h>
#include <Mod/Part/App/GeometryPy.h>

#include "GeometryBase.h"
#include "GeometryFactory.h"
#include "GeometryPoint.h"
#include "GeometryLineSegment.h"
#include "GeometryCircle.h"

#include "Reference.pybind.h"
#include "Geometry.pybind.h"

using vparam = std::vector<Base::Accessor::ReferenceTo<NamedSketcher::GCS::Parameter>>;
using vpoint = std::vector<Base::Accessor::ReferenceTo<NamedSketcher::GCS::Point>>;
PYBIND11_MAKE_OPAQUE(vparam);
PYBIND11_MAKE_OPAQUE(vpoint);

namespace NamedSketcher
{

std::unique_ptr<GeometryBase> geometryFactoryPy(py::object* geo)
{
    return geometryFactory(pyObjectToPartGeometry(geo));
}

std::unique_ptr<Part::Geometry> pyObjectToPartGeometry(py::object* geo)
{
    if (PyObject_TypeCheck(geo->ptr(), &(Part::GeometryPy::Type)))
    {
        const Part::Geometry* g = static_cast<Part::GeometryPy*>(geo->ptr())->getGeometryPtr();
        return std::unique_ptr<Part::Geometry>(g->copy());
    }
    else
    {
        throw std::invalid_argument("Argument must be a Part::Geometry.");
    }
}

void init_Geometry(py::module& m)
{
    py::bind_vector<vparam>(m, "VectorReferenceToParameter");
    py::bind_vector<vpoint>(m, "VectorReferenceToPoint");

    // Parameters
    py::class_<GCS::Parameter, std::shared_ptr<GCS::Parameter>>(m, "GCS_Parameter")
        .def(py::init<double>())
        .def("assign", [](GCS::Parameter& p, double v){return p = v;})
        // TOOD: automatica implicit conversion to float!
        .def_property("value", [](GCS::Parameter& p) -> double{return p;}, [](GCS::Parameter& p, double v){return p = v;})
        ;
//    py::implicitly_convertible<GCS::Parameter, py::float_>();

    py::class_<GCS::Point, std::shared_ptr<GCS::Point>>(m, "GCS_Point")
        .def(py::init<double, double>())
        // TODO: just use GCS::Parameter when above implicit conversion starts working.
        .def_property("x", [](GCS::Point& p) -> double{return p.x;}, [](GCS::Point& p, double v){return p.x = v;})
        .def_property("y", [](GCS::Point& p) -> double{return p.y;}, [](GCS::Point& p, double v){return p.y = v;})
        ;

    py::class_<GeometryBase, std::shared_ptr<GeometryBase>>(m, "Geometry")
        .def(py::init(&geometryFactoryPy))
        .def("getReferencesToParameters", [](GeometryBase& self){return self.getReferencesTo<GCS::Parameter>();})
        .def("getReferencesToPoints", [](GeometryBase& self){return self.getReferencesTo<GCS::Point>();})
        ;

    py::class_<GeometryPoint, std::shared_ptr<GeometryPoint>, GeometryBase>(m, "Point")
        .def(py::init<double, double>())
        ;

    py::class_<GeometryLineSegment, std::shared_ptr<GeometryLineSegment>, GeometryBase>(m, "LineSegment")
        .def(py::init<double, double, double, double>())
        ;

    py::class_<GeometryCircle, std::shared_ptr<GeometryCircle>, GeometryBase>(m, "Circle")
        .def(py::init<double, double, double>())
        ;
}

} //namespace NamedSketcher
