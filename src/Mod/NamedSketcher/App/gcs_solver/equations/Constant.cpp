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

#include "Constant.h"

namespace NamedSketcher::GCS
{

double Constant::error() const
{
    return a->getValue() - k->getValue();
}

std::vector<GradientDuplet> Constant::differentialNonOptimized(Shaker& /*shake*/) const
{
    std::vector<GradientDuplet> result;
    result.reserve(1);
    // TODO: remove comments when we start using C++20.
    result.emplace_back({/*.parameter =*/ a, /*.value =*/ 1});
    return result;
}

std::vector<GradientDuplet> Constant::differentialOptimized(Shaker& shake) const
{
    return differentialNonOptimized(shake);
}

} // namespace NamedSketcher::GCS
