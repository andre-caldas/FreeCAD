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

#include "BiCG.h"

namespace NamedSketcher::GCS::LinearSolvers
{

BiCG::BiCG(ParameterGroupManager& manager, const OptimizedMatrix& gradients)
    : SolverBase(manager, gradients)
{
    solver.setTolerance(0x1p-16);
    MtM = eigenMatrix.transpose() * eigenMatrix;
    solver.analyzePattern(MtM);
std::cerr << "Solver tolerance: " << solver.tolerance() << std::endl;
}

void BiCG::refactor()
{
    if(need_refactor)
    {
        MtM = eigenMatrix.transpose() * eigenMatrix;
        solver.factorize(MtM);
        need_refactor = false;
    }
}

BiCG::vector_t BiCG::_solve(const vector_t& out)
{
    refactor();
    return solver.solve(eigenMatrix.transpose() * out);
}

} // namespace NamedSketcher::GCS::LinearSolvers
