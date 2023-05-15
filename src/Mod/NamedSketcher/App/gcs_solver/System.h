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

#include <vector>

#include <Eigen/SparseCore>

#include "NamedSketcherGlobal.h"

namespace NamedSketcher::GCS
{

class Equation;

class NamedSketcherExport System
{
public:
    void addEssentialEquation(Equation* equation);
    void addRedundantEquation(Equation* equation);
    void addAssertEquation(Equation* equation);

    void removeEssentialEquation(Equation* equation);
    void removeRedundantEquation(Equation* equation);
    void removeAssertEquation(Equation* equation);

private:
    /**
     * @brief Each equation generates a gradient.
     * Each column of this matrix is the gradient of an equation (function)
     * at a certain "good point". It is the transposed of the differential.
     */
    Eigen::SparseMatrix gradientsT;
    /**
     * @brief The gradientsT can be orthonormalized.
     * The columns of gradientsTQ are the orthonormalized gradients.
     */
    Eigen::SparseMatrix gradientsTQ;
    /**
     * @brief The gradientsT can be orthonormalized.
     * The columns of gradientsTR are the scalars used to write each
     * gradient as a linear combination of the orthonormalized vectors.
     */
    Eigen::SparseMatrix gradientsTR;

    std::vector<Equation*> essentialEquations;
    std::vector<Equation*> extraRedundantEquations;
    std::vector<Equation*> assertEquations;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_System_H
