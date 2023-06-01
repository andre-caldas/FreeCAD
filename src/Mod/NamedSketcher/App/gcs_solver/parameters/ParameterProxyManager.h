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
    ~ParameterProxyManager();

    /**
     * @brief Adds a parameter to the system.
     * @param a: The @class ProxiedParameter to add to the system.
     * @attention Idempotent: this can be called twice for the same argument.
     * The second time, it does nothing.
     */
    void addParameter(ProxiedParameter* a);

    /**
     * @brief Adds an equation to the system.
     * @param eq: The @class Equation to add to the system.
     */
    void addEquation(Equation* eq);

    /**
     * @brief Makes both proxies represent the same parameter.
     * @param a: first @class ProxiedParameter.
     * @param b: second @class ProxiedParameter.
     * @return Returns false if they are already equal.
     * Otherwise, returns true.
     */
    bool setParameterEqual(ProxiedParameter* a, ProxiedParameter* b);

    /**
     * @brief Finished the optimization step,
     * this function needs to be called
     * to give an index number to each @class ParameterGroup.
     */
    void setParameterIndexes() const;
    int getParameterIndex(double* valuePointer) const;
    int getParamterIndex(ParameterGroup* group) const;
    ParameterGroup* getGroup(int index) const;

    int getEquationIndex(Equation* eq) const;
    int getEquation(int index) const;
    OptimizedVector getOptimizedParameterVector() const;
    OptimizedVector optimizeVector(const Vector& v);

    int size() const;
    void commitValues() const;

private:
    std::unordered_set<std::unique_ptr<ParameterGroup>> parameterGroups;
    std::vector<ParameterGroup*> indexedParameterGroups;
    std::map<Parameter*, int> parameterIndexes;
    std::vector<ProxiedParameter*> originalParameters;

    std::vector<Equation*> equations;
    std::map<Equation*, int> equationIndexes;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterProxyManager_H
