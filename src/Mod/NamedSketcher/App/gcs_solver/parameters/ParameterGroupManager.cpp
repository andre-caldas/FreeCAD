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

#include <Base/Exception.h>
#include <Base/Console.h>

#include <initializer_list>
#include <unordered_set>

#include "Parameter.h"
#include "../Vector.h"
#include "ParameterGroup.h"
#include "ParameterGroupManager.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace NamedSketcher::GCS
{

void ParameterGroupManager::addParameter(Parameter* a)
{
    assert(parameter2Group.count(a) == 0);
    assert(orderedNonConstantGroups.empty());

    auto group = std::make_unique<ParameterGroup>(a);
    parameter2Group.emplace(a, group.get());
    parameterGroups.emplace(std::move(group));
}

void ParameterGroupManager::addEquation(Equation* eq)
{
    assert(equationIndexes.count(eq) == 0);
    orderedEquations.emplace_back(eq);
    equationIndexes.emplace(eq, equationIndexes.size());
}

bool ParameterGroupManager::setParameterEqual(Parameter* a, Parameter* b)
{
    assert(parameter2Group.count(a) == 0);
    ParameterGroup* group_a = getParameterGroup(a);
    ParameterGroup* group_b = getParameterGroup(b);

    if(group_a == group_b)
    {
        return false;
    }

    for(Parameter* p: *group_b)
    {
        assert(parameter2Group.at(p) == group_b);
        parameter2Group.at(p) = group_a;
    }
    *group_a << std::move(*group_b);
    parameterGroups.extract(std::find_if(parameterGroups.begin(), parameterGroups.end(),
                                         [group_b](auto& g){return g.get() == group_b;}));
    return true;
}

bool ParameterGroupManager::areParametersEqual(Parameter* a, Parameter* b) const
{
    if(parameter2Group.count(a) == 0 || parameter2Group.count(b) == 0)
    {
        FC_THROWM(Base::RuntimeError, "Equating parameters that are not managed.");
    }

    return (parameter2Group.at(a) == parameter2Group.at(b));
}

ParameterGroup* ParameterGroupManager::getParameterGroup(Parameter* parameter) const
{
    return parameter2Group.at(parameter);
}

bool ParameterGroupManager::setParameterConstant(Parameter* k)
{
    assert(orderedNonConstantGroups.empty());
    return getParameterGroup(k)->setConstant(k);
}

void ParameterGroupManager::setOptimizedParameterIndexes()
{
    assert(orderedNonConstantGroups.empty());
    assert(nonConstantGroupIndexes.empty());
    assert(optimizedParameter2NonConstantGroup.empty());
    for(auto& group: parameterGroups)
    {
        if(!group->isConstant())
        {
            orderedNonConstantGroups.emplace_back(group.get());
            nonConstantGroupIndexes.emplace(group.get(), nonConstantGroupIndexes.size());
            optimizedParameter2NonConstantGroup.emplace(group->getValuePtr(), group.get());
        }
    }
}

size_t ParameterGroupManager::getOptimizedParameterIndex(OptimizedParameter* parameter) const
{
    ParameterGroup* group = optimizedParameter2NonConstantGroup.at(parameter);
    return getNonConstantGroupIndex(group);
}

size_t ParameterGroupManager::getNonConstantGroupIndex(ParameterGroup* group) const
{
    assert(!group->isConstant());
    return nonConstantGroupIndexes.at(group);
}

OptimizedParameter* ParameterGroupManager::getOptimizedParameter(Parameter* parameter) const
{
    ParameterGroup* group = getParameterGroup(parameter);
    assert(!group->isConstant());
    return group->getValuePtr();
}

double ParameterGroupManager::getOptimizedParameterValue(Parameter* parameter) const
{
    ParameterGroup* group = getParameterGroup(parameter);
    return group->getValue();
}

OptimizedVector ParameterGroupManager::getOptimizedParameterValues() const
{
    OptimizedVector result;
    for(ParameterGroup* group: orderedNonConstantGroups)
    {
        result.set(group->getValuePtr(), group->getValue());
    }
    return result;
}

void ParameterGroupManager::setOptimizedParameterValues(const OptimizedVector& vals) const
{
    for(ParameterGroup* group: orderedNonConstantGroups)
    {
        group->setValue(vals[group->getValuePtr()]);
    }
}

OptimizedVector ParameterGroupManager::optimizeVector(const ParameterVector& v) const
{
    OptimizedVector result;
    for(auto [parameter, value]: v.values)
    {
        ParameterGroup* group = getParameterGroup(parameter);
        if(group->isConstant())
        {
            continue;
        }

        if(result.hasKey(group->getValuePtr()))
        {
            FC_THROWM(Base::RuntimeError, "Cannot optimize vector. This is a bug!");
        }
        result.set(group->getValuePtr(), value);
    }
    return result;
}

size_t ParameterGroupManager::getGroupIndex(ParameterGroup* group) const
{
    return nonConstantGroupIndexes.at(group);
}

ParameterGroup* ParameterGroupManager::getGroup(size_t index) const
{
    return orderedNonConstantGroups.at(index);
}

size_t ParameterGroupManager::getEquationIndex(Equation* eq) const
{
    return equationIndexes.at(eq);
}

Equation* ParameterGroupManager::getEquation(size_t index) const
{
    return orderedEquations.at(index);
}

size_t ParameterGroupManager::inputSize() const
{
    assert(orderedNonConstantGroups.size() == nonConstantGroupIndexes.size());
    return orderedNonConstantGroups.size();
}

size_t ParameterGroupManager::outputSize() const
{
    assert(orderedEquations.size() == equationIndexes.size());
    return orderedEquations.size();
}

void ParameterGroupManager::commitParameters() const
{
    for(auto& group: parameterGroups)
    {
        group->commit();
    }
}

} // namespace NamedSketcher::GCS
