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


#ifndef NAMEDSKETCHER_GCS_System_H
#define NAMEDSKETCHER_GCS_System_H

#include <memory>
#include <unordered_set>
#include <vector>

#include <Eigen/SparseCore>

#include "Types.h"
#include "Shaker.h"
#include "Orthonormalization.h"

namespace NamedSketcher::GCS
{

class Equation;
class Equal;
class Constant;
class ParameterGroupManager;

class NamedSketcherExport System
{
public:
    using differential_t = Eigen::SparseMatrix<double, Eigen::RowMajor>;
    using equation_value_t = Eigen::SparseVector<double>;

    void addEquation(Equation* equation);
    void removeEquation(Equation* equation);
    void updateGradients();

    void addUserRedundantEquation(Equation* equation);
    void optimize();
    bool solve() const;

    OutputVector error(const ParameterGroupManager& manager) const;
    equation_value_t minus_error(const ParameterGroupManager& manager) const;

private:
    /**
     * @brief Each equation generates a gradient.
     * Each row of this matrix is the gradient of an equation (function)
     * at a certain "good point".
     * @attention Does not include automatically generated extraRedundantEquations,
     * nor userRedundantEquations.
     */
    Orthonormalization gradients;

    /**
     * @brief userRedundantEquations: constraints added by the user just for checking.
     * Not used for solving the sytem. Just for checking and alerting the user.
     * @attention Those equations are not considered anywhere else.
     */
    std::vector<Equation*> userRedundantEquations;

    /**
     * @brief extraRedundantEquations: redundant equations that are automatically generated.
     * For example, A = B = C could be decomposed into two non-redundant equations:
     * A = B and B = C, and a third redundant equation: A = C.
     * Those are used for making the solver more stable.
     * They are added to the system being solved.
     */
    std::vector<Equation*> extraRedundantEquations;

    /**
     * @brief Searches for the "best point" between current point
     * and the approximate next step point determined by getStepTarget.
     * @param manager: @class ParameterGroupManager to translate coordinates.
     * @param target: where to aim.
     */
    void stepIntoTargetDirection(
            ParameterGroupManager& manager,
            const OptimizedVector& target) const;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_System_H
