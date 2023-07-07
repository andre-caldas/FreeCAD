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

#include <iostream>
#include <Eigen/Core>

#include "../equations/Equation.h"

#include "../parameters/ParameterGroupManager.h"
#include "../parameters/ParameterGroup.h"
#include "../Vector.h"
#include "../LinearTransform.h"
#include "../Types.h"

#include "SolverBase.h"

namespace NamedSketcher::GCS::LinearSolvers
{

SolverBase::SolverBase(ParameterGroupManager& manager, const OptimizedMatrix& optimizedMatrix)
    : manager(manager)
{
    assert(manager.outputSize() == optimizedMatrix.size());
    int rows = manager.outputSize();
    int cols = manager.inputSize();
    eigenMatrix.resize(rows,cols);
    // TODO: reserve non-zero spots.
    for(int row=0; row < rows; ++row)
    {
        for(auto [k,v]: optimizedMatrix[row].values)
        {
            int col = manager.getOptimizedParameterIndex(k);
            eigenMatrix.insert(row, col) = v;
        }
    }
    eigenMatrix.makeCompressed();
}

void SolverBase::updateGradient(Equation* equation)
{
    auto eq_index = manager.getEquationIndex(equation);

    OptimizedVector gradient = equation->differentialOptimized(manager);
    for(auto [k,v]: gradient.values)
    {
        eigenMatrix.coeffRef(eq_index, manager.getOptimizedParameterIndex(k)) = v;
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

    std::cout << "Linear target: " << eigen_target << std::endl;
    vector_t solution = _solve(eigen_target);
    std::cout << "Linear solution: " << solution << std::endl;
    assert((size_t)solution.rows() == manager.inputSize());

    OptimizedVector result;
    for(size_t index = 0; index < manager.inputSize(); ++index)
    {
        auto group = manager.getGroup(index);
        result.set(group->getValuePtr(), solution[index]);
    }
    return result;
}

} // namespace NamedSketcher::GCS::LinearSolvers
