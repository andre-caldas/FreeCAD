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
#include "Constant.h"

namespace NamedSketcher::GCS
{

void Constant::set(Parameter* x, Parameter* v)
{
    if(x == v)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    k = v;
}

double Constant::error(const ParameterProxyManager& manager) const
{
    const double A = manager.getOptimizedParameterValue(a);
    const double K = manager.getOptimizedParameterValue(k);
    return A - K;
}

ParameterVector Constant::differentialNonOptimized() const
{
    ParameterVector result;
    result.set(a, 1);
    return result;
}

OptimizedVector Constant::differentialOptimized(const ParameterProxyManager& manager) const
{
    return manager.optimizeVector(differentialNonOptimized());
}

void Constant::setProxies(ParameterProxyManager& manager) const
{
    manager.addParameter(a);
}

} // namespace NamedSketcher::GCS
