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

#include <Base/Exception.h>

#include "../parameters/ParameterProxyManager.h"
#include "Difference.h"

namespace NamedSketcher::GCS
{

void Difference::set(Parameter* x, Parameter* y, Parameter* d)
{
    if(x == y || x == d || y == d)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    b = y;
    difference = d;
}

double Difference::error(const ParameterProxyManager& manager) const
{
    const double A = manager.getOptimizedParameterValue(a);
    const double B = manager.getOptimizedParameterValue(b);
    const double DIFF = manager.getOptimizedParameterValue(difference);
    return B - A - DIFF;
}

ParameterVector Difference::differentialNonOptimized() const
{
    ParameterVector result;
    result.set(a, -1);
    result.set(b, 1);
    return result;
}

OptimizedVector Difference::differentialOptimized(const ParameterProxyManager& manager) const
{
    if(!manager.areParametersEqual(a, b))
    {
        return manager.optimizeVector(differentialNonOptimized());
    }
    return OptimizedVector();
}

void Difference::setProxies(ParameterProxyManager& manager) const
{
    manager.addParameter(a);
    manager.addParameter(b);
}

} // namespace NamedSketcher::GCS
