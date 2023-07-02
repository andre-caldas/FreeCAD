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

#ifndef NAMEDSKETCHER_NamedSketchPybind_H
#define NAMEDSKETCHER_NamedSketchPybind_H

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <memory>
#include "NamedSketch.h"

namespace NamedSketcher
{

class GeometryBase;

void init_NamedSketch(py::module& m);

/*
 * Temporary trampolim class to deal with heterogeneous
 * pybind and non-pybind python bindings.
 */
class NamedSketchPy : public NamedSketch
{
public:
    PropertyGeometryList::item_reference addGeometry(py::object* geo);
};

} //namespace NamedSketcher

#endif // NAMEDSKETCHER_NamedSketchPybind_H
