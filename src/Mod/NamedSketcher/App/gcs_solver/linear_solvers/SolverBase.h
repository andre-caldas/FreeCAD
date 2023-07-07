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


#ifndef NAMEDSKETCHER_GCS_LinearSolvers_SolverBase_H
#define NAMEDSKETCHER_GCS_LinearSolvers_SolverBase_H

#include <Eigen/SparseCore>

#include "../Types.h"

namespace NamedSketcher::GCS {
class ParameterGroupManager;
}

namespace NamedSketcher::GCS::LinearSolvers
{

class SolverBase
{
public:
    using matrix_t = Eigen::SparseMatrix<double, Eigen::RowMajor>;
    using vector_t = Eigen::VectorXd;

    SolverBase(ParameterGroupManager& manager, const OptimizedMatrix& optimizedMatrix);

    void updateGradient(Equation* equation);
    OptimizedVector solve();

protected:
    ParameterGroupManager& manager;
    matrix_t eigenMatrix;
    bool need_refactor = true;

    virtual void refactor() = 0;
    virtual vector_t _solve(const vector_t& target) = 0;
};

} // namespace NamedSketcher::GCS::LinearSolvers

#endif // NAMEDSKETCHER_GCS_LinearSolver_SolverBase_H
