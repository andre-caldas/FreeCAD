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


#ifndef NAMEDSKETCHER_GCS_Types_H
#define NAMEDSKETCHER_GCS_Types_H

#include <vector>

#include "LinearTransform.h"
#include "parameters/Parameter.h"
#include "parameters/Vector.h"

namespace NamedSketcher::GCS
{

class Equation;
class ParameterGroup;

using OutputVector = Vector<Equation*>;
using ParameterVector = Vector<Parameter*>;
using OptimizedVector = Vector<OptimizedParameter*>;

using ParameterMatrix = LinearTransform<Equation*, Parameter*>;
using OptimizedMatrix = LinearTransform<Equation*, OptimizedParameter*>;

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Types_H
