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
#include "Equal.h"

namespace NamedSketcher::GCS
{

void Equal::set(Parameter* x, Parameter* y)
{
    if(x == y)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    b = y;
}

double Equal::error(const ParameterProxyManager& manager) const
{
    const double A = manager.getOptimizedParameterValue(a);
    const double B = manager.getOptimizedParameterValue(b);
    return B - A;
}

ParameterVector Equal::differentialNonOptimized() const
{
    ParameterVector result;
    result.set(a, -1);
    result.set(b, 1);
    return result;
}

OptimizedVector Equal::differentialOptimized(const ParameterProxyManager& manager) const
{
    if(!manager.areParametersEqual(a,b))
    {
        return manager.optimizeVector(differentialNonOptimized());
    }
    return OptimizedVector();
}

bool Equal::optimizeProxies(ParameterProxyManager& manager) const
{
    return manager.setParameterEqual(a,b);
}

} // namespace NamedSketcher::GCS
