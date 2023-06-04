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


#ifndef NAMEDSKETCHER_GCS_ParameterProxyManager_H
#define NAMEDSKETCHER_GCS_ParameterProxyManager_H

#include <unordered_set>
#include <map>
#include <vector>
#include <memory>

#include "../Types.h"
#include "NamedSketcherGlobal.h"

namespace NamedSketcher::GCS
{

class NamedSketcherExport ParameterProxyManager
{
public:
    /**
     * @brief Adds a parameter to the system.
     * @param a: The @class Parameter to add to the system.
     * @attention Idempotent: this can be called twice for the same argument.
     * The second time, it does nothing.
     */
    void addParameter(Parameter* a);

    /**
     * @brief Adds an equation to the system.
     * @param eq: The @class Equation to add to the system.
     */
    void addEquation(Equation* eq);

    /**
     * @brief Proxy two @class Parameter to mean the same @class OptimizedParameter.
     * @param a: first @class Parameter.
     * @param b: second @class Parameter.
     * @return Returns false if they are already equal.
     * Otherwise, returns true.
     */
    bool setParameterEqual(Parameter* a, Parameter* b);

    /**
     * @brief Verify if two @class Parameter where optimized
     * to being the same @class OptimizedParameter.
     * @param a: first @class Parameter to compare.
     * @param b: second @class Parameter to compare.
     * @return True if both @class Parameter are proxied to the same @class OptimizedParameter.
     */
    bool areParametersEqual(Parameter* a, Parameter* b) const;

    /**
     * @brief Finished the optimization step,
     * this function needs to be called
     * to give an index number to each @class ParameterGroup.
     */
    void setOptimizedParameterIndexes() const;
    int getOptimizedParameterIndex(OptimizedParameter* parameter) const;
    int getGroupIndex(ParameterGroup* group) const;
    ParameterGroup* getGroup(int index) const;

    int getEquationIndex(Equation* eq) const;
    int getEquation(int index) const;
    OptimizedParameter* getOptimizedParameter(Parameter* parameter) const;
    OptimizedVector getOptimizedParameterValues() const;
    OptimizedVector setOptimizedParameterValues(const OptimizedVector& vals);
    OptimizedVector optimizeVector(const ParameterVector& v);

    int inputSize() const;
    int outputSize() const;
    void commitValues() const;

private:
    std::unordered_set<std::unique_ptr<ParameterGroup>> parameterGroups;
    std::vector<ParameterGroup*> indexedParameterGroups;
    std::map<OptimizedParameter*, int> optimizedParameterIndexes;
    std::vector<Parameter*> originalParameters;

    std::vector<Equation*> equations;
    std::map<Equation*, int> equationIndexes;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterProxyManager_H
