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

#include <Eigen/Cholesky>

#include "../parameters/ParameterGroupManager.h"

#include "SimpleSolver.h"

namespace NamedSketcher::GCS::LinearSolvers
{

SimpleSolver::SimpleSolver(ParameterGroupManager& manager, const OptimizedMatrix& gradients)
    : SolverBase(manager, gradients)
    , denseMatrix(manager.outputSize(), manager.inputSize())
{
}

void SimpleSolver::refactor()
{
    if(need_refactor)
    {
        denseMatrix = eigenMatrix;
    }
}

SimpleSolver::vector_t SimpleSolver::_solve(const vector_t& out)
{
    refactor();
    const auto& D = denseMatrix;
    std::cout << "Will solve using (SimpleSolver)..." << std::endl;
    std::cout << "Matrix:" << std::endl;
    std::cout << eigenMatrix << std::endl;
    std::cout << "M^t M:" << std::endl;
    std::cout << (D.transpose() * D) << std::endl;
    std::cout << "Target:" << std::endl;
    std::cout << out << std::endl;
    std::cout << "M^t Target:" << std::endl;
    std::cout << (D.transpose() * out) << std::endl;
    std::cout << "Solution:" << std::endl;
    std::cout << (D.transpose() * D).ldlt().solve(D.transpose() * out) << std::endl;
    return (D.transpose() * D).ldlt().solve(D.transpose() * out).eval();
}

} // namespace NamedSketcher::GCS::LinearSolvers
