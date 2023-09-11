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


#ifndef NAMEDSKETCHER_GCS_ParameterGroupManager_H
#define NAMEDSKETCHER_GCS_ParameterGroupManager_H

#include <functional>
#include <unordered_set>
#include <map>
#include <vector>
#include <memory>

#include "../Types.h"
#include "ParameterValueMapper.h"

namespace NamedSketcher::GCS
{

class ParameterGroupManager
    : public ParameterValueMapper
{
public:
    /**
     * @brief The value assigned to a @a parameter through this manager.
     * @param parameter
     * @return The value attributed to @a parameter.
     */
    double getValue(const Parameter* parameter) const override {return getOptimizedParameterValue(parameter);}

    /**
     * @brief Checks if @a parameter is already managed.
     * @param parameter: parameter to check.
     * @return Is it already managed?
     */
    bool hasParameter(const Parameter* parameter) const {return (parameter2Group.count(parameter) != 0);}

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
    bool setParameterEqual(const Parameter* a, const Parameter* b);

    /**
     * @brief Verify if two @class Parameter where optimized
     * to being the same @class OptimizedParameter.
     * @param a: first @class Parameter to compare.
     * @param b: second @class Parameter to compare.
     * @return True if both @class Parameter are proxied to the same @class OptimizedParameter.
     */
    bool areParametersEqual(const Parameter* a, const Parameter* b) const;

    /**
     * @brief Makes all @class OptimizedParameter that are equivalent to @a k
     * to be constant equal to @a k.
     * @param k: @class Parameter with the constant value.
     * @return Returns false if they are already constant equal to @a k.
     * Otherwise, returns true.
     * @throws If group already has a constant associated to it.
     */
    bool setParameterConstant(Parameter* k);

    /**
     * @brief Checks if parameter @a k is constant.
     * @param p: @class Parameter to check.
     * @return Iis constant?
     */
    bool isParameterConstant(const Parameter* p) const;

    ParameterGroup* getParameterGroup(const Parameter* parameter) const;

    /**
     * @brief Finished the optimization step,
     * this function needs to be called
     * to give an index number to each @class ParameterGroup.
     */
    void finishOptimization();
    size_t getOptimizedParameterIndex(const OptimizedParameter* parameter) const;
    size_t getNonConstantGroupIndex(const ParameterGroup* group) const;
    OptimizedParameter* getOptimizedParameter(const Parameter* parameter, bool finished_optimization=true) const;
    double getOptimizedParameterValue(const Parameter* parameter) const;
    OptimizedVector getOptimizedParameterValues() const;
    void setOptimizedParameterValues(const OptimizedVector& vals) const;

    size_t getGroupIndex(const ParameterGroup* group) const;
    ParameterGroup* getGroup(size_t index) const;

    bool hasEquation(const Equation* eq) const;
    size_t getEquationIndex(const Equation* eq) const;
    Equation* getEquation(size_t index) const;
    OptimizedVector optimizeVector(const ParameterVector& v) const;

    size_t inputSize() const;
    size_t outputSize() const;
    void commitParameters() const;

    OptimizedVector noise() const;

    void print_vector(const OptimizedVector& v) const;
    void report_position() const;
    void report() const;

private:
    /**
     * @brief Parameters are grouped in @class ParameterGroup.
     * This variable holds all groups.
     */
    std::unordered_set<std::unique_ptr<ParameterGroup>> parameterGroups;
    /**
     * @brief Reverse lookup @class Parameter --> @class ParameterGroup.
     * @attention Premature optimization. :-)
     */
    std::unordered_map<const Parameter*, ParameterGroup*> parameter2Group;

    /**
     * @brief Each non-constant @class ParameterGroup
     * "exports" one @class OptimizedParameter.
     * To each non-constant @class ParameterGroup we associate an index,
     * so we can use matrices.
     * @attention This is set only after calling setOptimizedParameterIndexes().
     */
    std::vector<ParameterGroup*> orderedNonConstantGroups;
    /**
     * @brief orderedNonConstantGroups inverse lookup.
     * @attention This is set only after calling setOptimizedParameterIndexes().
     */
    std::unordered_map<const ParameterGroup*, size_t> nonConstantGroupIndexes;
    /**
     * @brief Reverse lookup @class OptimizedParameter --> @class ParameterGroup.
     * @attention This is set only after calling setOptimizedParameterIndexes().
     * @attention Premature optimization. :-)
     */
    std::unordered_map<const OptimizedParameter*, ParameterGroup*> optimizedParameter2NonConstantGroup;

    /**
     * @brief Each @class Equation passed to the manager
     * (potentially conflicting constraints are not)
     * is associated to an index, so we can use matrices.
     */
    std::vector<Equation*> orderedEquations;
    /**
     * @brief orderedEquations inverse lookup.
     */
    std::unordered_map<const Equation*, size_t> equationIndexes;

    bool called_finish_optimization = false;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterGroupManager_H
