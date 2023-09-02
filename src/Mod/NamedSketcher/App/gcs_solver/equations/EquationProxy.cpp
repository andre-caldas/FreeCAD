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

#include "EquationProxy.h"

namespace NamedSketcher::GCS {

void EquationProxy::set(Equation* eq)
{
    proxied_equation = eq;
}

void EquationProxy::reset()
{
    proxied_equation = nullptr;
}

double EquationProxy::error(const GCS::ParameterGroupManager& manager) const
{
    if(proxied_equation == nullptr)
    {
        return 0;
    }
    return proxied_equation->error(manager);
}

ParameterVector EquationProxy::differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const
{
    if(proxied_equation == nullptr)
    {
        return ParameterVector();
    }
    return proxied_equation->differentialNonOptimized(parameter_mapper);
}

OptimizedVector EquationProxy::differentialOptimized(const ParameterGroupManager& manager) const
{
    if(proxied_equation == nullptr)
    {
        return OptimizedVector();
    }
    return proxied_equation->differentialOptimized(manager);
}


void EquationProxy::declareParameters(ParameterGroupManager& manager) const
{
    if(proxied_equation == nullptr)
    {
        return;
    }
    proxied_equation->declareParameters(manager);
}

bool EquationProxy::optimizeParameters(ParameterGroupManager& manager) const
{
    if(proxied_equation == nullptr)
    {
        return false;
    }
    return proxied_equation->optimizeParameters(manager);
}

void EquationProxy::report() const
{
    if(proxied_equation == nullptr)
    {
        std::cerr << "Proxy not set!" << std::endl;
        return;
    }
    std::cerr << "Proxied - ";
    proxied_equation->report();
}


} // namespace NamedSketcher
