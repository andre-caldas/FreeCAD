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


#ifndef NAMEDSKETCHER_GCS_Equation_H
#define NAMEDSKETCHER_GCS_Equation_H

#include <vector>
#include <Eigen/SparseCore>

#include "../Types.h"
#include "NamedSketcherGlobal.h"

namespace NamedSketcher::GCS
{

class Point;
class ParameterGroupManager;
class ParameterValueMapper;

class NamedSketcherExport Equation
{
public:
    virtual double error(const GCS::ParameterGroupManager& manager) const = 0;
    virtual ParameterVector differentialNonOptimized(const GCS::ParameterValueMapper& parameter_mapper) const = 0;
    virtual OptimizedVector differentialOptimized(const ParameterGroupManager& manager) const = 0;
    virtual bool isLinear() const = 0;

    /**
     * @brief Informs the @a manager which proxied parameters are used by the system.
     * @param manager: object that manages the group of proxied parameters.
     */
    virtual void declareParameters(ParameterGroupManager& manager) const = 0;

    /**
     * @brief Informs the @a manager which proxies to optimize (set equal or fixed value).
     * @param manager: object that manages the group of proxied parameters.
     * @return Returns true when some new proxy was optimized and false otherwise.
     */
    virtual bool optimizeParameters(ParameterGroupManager& /*manager*/) const {return false;}

    /**
     * @brief When searching for a solution, the system "suggests" a step.
     * That is, a vector with the suggested changes in the OprimizedParameters.
     * The suggestion is a not very precise "long shot".
     * This function indicates how much the step needs to be reduced
     * in order to avoid "harmful changes".
     * @param step: the suggested step.
     * @return A number "a" in (0,1] such that "step *= a" will be a better step.
     */
    virtual double limitStep(const ParameterGroupManager& /*manager*/, const OptimizedVector& /*step*/) const {return 1.0;}

    virtual void report() const = 0;
};

class NamedSketcherExport LinearEquation : public Equation
{
    bool isLinear() const override {return true;}
};

class NamedSketcherExport NonLinearEquation : public Equation
{
    bool isLinear() const override {return false;}
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Equation_H
