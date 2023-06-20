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

#include <Eigen/Core>

#include "../equations/Equation.h"

#include "../parameters/ParameterProxyManager.h"
#include "../parameters/ParameterGroup.h"
#include "../Vector.h"
#include "../LinearTransform.h"
#include "../Types.h"

#include "SolverBase.h"

namespace NamedSketcher::GCS::LinearSolvers
{

SolverBase::SolverBase(ParameterProxyManager& manager, const OptimizedMatrix& _gradients)
    : manager(manager)
{
    int rows = _gradients.size();
    // Reserve 10 parameters per dual vector. Magic number. :-(
    gradients.reserve(10*rows);
    for(int row=0; row < rows; ++row)
    {
        for(auto [k,v]: _gradients[row].values)
        {
            int col = manager.getOptimizedParameterIndex(k);
            gradients.insert(row, col) = v;
        }
    }
    gradients.makeCompressed();
}

void SolverBase::updateGradient(Equation* equation)
{
    auto eq_index = manager.getEquationIndex(equation);

    OptimizedVector gradient = equation->differentialOptimized(manager);
    for(auto [k,v]: gradient.values)
    {
        gradients.coeffRef(eq_index, manager.getOptimizedParameterIndex(k)) = v;
    }
    need_refactor = true;
}

OptimizedVector SolverBase::solve()
{
    vector_t eigen_target(manager.outputSize());
    for(size_t i=0; i < manager.outputSize(); ++i)
    {
        Equation* eq = manager.getEquation(i);
        eigen_target[i] = -eq->error(manager);
    }

    vector_t solution = _solve(eigen_target);
    assert((size_t)solution.rows() == manager.outputSize());

    OptimizedVector result;
    for(size_t index = 0; index < manager.outputSize(); ++index)
    {
        auto group = manager.getGroup(index);
        result.set(group->getValuePtr(), group->getValue());
    }
    return result;
}

} // namespace NamedSketcher::GCS::LinearSolvers
