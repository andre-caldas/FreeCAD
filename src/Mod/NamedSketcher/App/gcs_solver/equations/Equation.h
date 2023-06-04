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
class ParameterProxyManager;

class NamedSketcherExport Equation
{
public:
    virtual double error() const = 0;
    virtual ParameterVector differentialNonOptimized() const = 0;
    virtual OptimizedVector differentialOptimized(ParameterProxyManager& manager) const = 0;
    virtual bool isLinear() const = 0;

    /**
     * @brief Informs the @a manager which proxied parameters are used by the system.
     * @param manager: object that manages the group of proxied parameters.
     */
    virtual void setProxies(ParameterProxyManager& manager) const = 0;

    /**
     * @brief Informs the @a manager which proxies to optimize (set equal or fixed value).
     * @param manager: object that manages the group of proxied parameters.
     * @return Returns true when some new proxy was optimized and false otherwise.
     */
    virtual bool optimizeProxies(ParameterProxyManager& /*manager*/) const {return false;}
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
