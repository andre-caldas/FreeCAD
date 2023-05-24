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
#include <vector>

#include "Types.h"
#include "Shaker.h"
#include "Orthonormalization.h"
#include "ParameterProxyManager.h"

namespace NamedSketcher::GCS
{

class Equation;
class Equal;
class Constant;

class NamedSketcherExport System
{
public:
    void addEquation(Equation* equation);
    void removeEquation(Equation* equation);

    void addUserRedundantEquation(Equation* equation);
    void optimize();

private:
    ParameterProxyManager manager;

    /**
     * @brief Each equation generates a gradient.
     * Each row of this matrix is the gradient of an equation (function)
     * at a certain "good point".
     */
    Orthonormalization gradients;

    /**
     * @brief userRedundantEquations: constraints added by the user just for checking.
     * Not used for solving the sytem. Just for checking and alerting the user.
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
     * @brief Noise generator.
     */
    Shaker shaker;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_System_H
