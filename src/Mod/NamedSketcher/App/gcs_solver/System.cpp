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

#include "equations/Equation.h"
#include "equations/Equal.h"
#include "equations/Constant.h"
#include "ParameterProxyManager.h"
#include "System.h"

namespace NamedSketcher::GCS
{

void System::addEquation(Equation* equation)
{
    equations.push_back(equation);
    gradients.pushBack(equation, equation->differentialNonOptimized());
}

void System::addUserRedundantEquation(Equation* equation)
{
    userRedundantEquations.push_back(equation);
}

void System::removeEquation(Equation* equation)
{
    gradients.remove(equation);
}

void System::optimize()
{
    int next_equal = 0;
    int next_constant = 0;
    int next_linear = 0;

    for(int j=0; j < gradients.size(); ++j)
    {
        if(dynamic_cast<Equal*>(gradients[j]))
        {
            if(j != next_equal)
            {
                for(int k=j; k != next_equal; --k)
                {
                    gradients.moveForward(k);
                }
            }
            ++next_equal;
            ++next_constant;
            ++next_linear;
            continue;
        }
        if(dynamic_cast<Constant*>(gradients[j]))
        {
            if(j != next_constant)
            {
                for(int k=j; k != next_constant; --k)
                {
                    gradients.moveForward(k);
                }
            }
            ++next_constant;
            ++next_linear;
            continue;
        }
        if(gradients[j]->isLinear())
        {
            if(j != next_linear)
            {
                for(int k=j; k != next_linear; --k)
                {
                    gradients.moveForward(k);
                }
            }
            ++next_linear;
            continue;
        }
    }
}

bool System::solve()
{
    ParameterProxyManager manager;
    Matrix gcs;
    OptimizedMatrix optimized_gcs;

    // First we set proxies.
    for(auto it = gradients.beginNonRedundants(); it != gradients.endNonRedundants(); ++it)
    {
        (*it)->setProxies(manager);
    }
    // Then we get the optimized vectors.
    for(auto it = gradients.beginNonRedundants(); it != gradients.endNonRedundants(); ++it)
    {
        xxx
    }
}

} // namespace NamedSketcher::GCS
