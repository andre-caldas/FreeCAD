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

#include <Eigen/SparseCholesky>

#include "Ldlt.h"

namespace NamedSketcher::GCS::LinearSolvers
{

Ldlt::Ldlt(ParameterGroupManager& manager, const OptimizedMatrix& gradients)
    : SolverBase(manager, gradients)
{
    const auto& D = eigenMatrix;
    solver.analyzePattern(D.transpose() * D);
}

void Ldlt::refactor()
{
    if(need_refactor)
    {
        std::cerr << "Refactoring solver..." << std::endl;
        const auto& D = eigenMatrix;
        solver.factorize(D.transpose() * D);
        need_refactor = false;
    }
}

Ldlt::vector_t Ldlt::_solve(const vector_t& out)
{
    const auto& D = eigenMatrix;
    std::cerr << "Will solve using (Ldlt)..." << std::endl;
    std::cerr << "Matrix:" << std::endl;
    std::cerr << eigenMatrix << std::endl;
    std::cerr << "M^t M:" << std::endl;
    std::cerr << (D.transpose() * D) << std::endl;
    std::cerr << "Target:" << std::endl;
    std::cerr << out << std::endl;
    std::cerr << "M^t Target:" << std::endl;
    std::cerr << (D.transpose() * out) << std::endl;
    refactor();
    std::cerr << "Solution:" << std::endl;
    std::cerr << solver.solve(D.transpose() * out) << std::endl;
    return solver.solve(D.transpose() * out).eval();
}

} // namespace NamedSketcher::GCS::LinearSolvers
