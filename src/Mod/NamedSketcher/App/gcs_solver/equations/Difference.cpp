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

#include "Difference.h"

namespace NamedSketcher::GCS
{

void Difference::set(ProxiedParameter* x, ProxiedParameter* y, ProxiedParameter* d)
{
    if(x == y || x == d || y == d)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.")
    }
    a = x;
    b = y;
    difference = d;
}

double Difference::error() const
{
    return b->getValue() - a->getValue() - difference->getValue();
}

ParameterVector Difference::differentialNonOptimized() const
{
    Vector result;
    result.set(a, -1);
    result.set(b, 1);
}

OptimizedVector Difference::differentialOptimized() const
{
    if(!a->samePointer(b))
    {
        return optimizeVector(differentialNonOptimized());
    }
    return OptimizedVector();
}

void Difference::setProxies(ParameterProxyManager& manager) const
{
    manager.add(a);
    manager.add(b);
}

} // namespace NamedSketcher::GCS
