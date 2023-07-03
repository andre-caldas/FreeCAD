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

#include "geometries/Geometry.pybind.h"
#include "constraints/Constraint.pybind.h"
#include "Reference.pybind.h"
#include "NamedSketch.pybind.h"
#include "NamedSketch.h"

namespace NamedSketcher
{

PYBIND11_MODULE(NamedSketcher, m)
{
    m.doc() = "A 2D sketcher that references objects by name";
    init_Reference(m); // Temporary hack.
    init_Geometry(m);
    init_Constraint(m);
    init_NamedSketch(m);

    NamedSketch::init();
}

} //namespace NamedSketcher
