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
#include "ParameterProxyManager.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace NamedSketcher::GCS
{

void ParameterProxyManager::addParameter(Parameter* a)
{
    assert(parameter2Group.count(a) == 0);
    assert(orderedNonConstantGroups.empty());

    auto group = std::make_unique<ParameterGroup>(a);
    parameter2Group.emplace(a, group.get());
    parameterGroups.emplace(std::move(group));
}

void ParameterProxyManager::addEquation(Equation* eq)
{
    assert(equationIndexes.count(eq) == 0);
    orderedEquations.emplace_back(eq);
    equationIndexes.emplace(eq, equationIndexes.size());
}

bool ParameterProxyManager::setParameterEqual(Parameter* a, Parameter* b)
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

bool ParameterProxyManager::areParametersEqual(Parameter* a, Parameter* b) const
{
    if(parameter2Group.count(a) == 0 || parameter2Group.count(b) == 0)
    {
        FC_THROWM(Base::RuntimeError, "Equating parameters that are not managed.");
    }

    return (parameter2Group.at(a) == parameter2Group.at(b));
}

ParameterGroup* ParameterProxyManager::getParameterGroup(Parameter* parameter) const
{
    return parameter2Group.at(parameter);
}

bool ParameterProxyManager::setParameterConstant(Parameter* k)
{
    assert(orderedNonConstantGroups.empty());
    getParameterGroup(k)->setConstant(k);
}

void ParameterProxyManager::setOptimizedParameterIndexes()
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

size_t ParameterProxyManager::getOptimizedParameterIndex(OptimizedParameter* parameter) const
{
    ParameterGroup* group = optimizedParameter2NonConstantGroup.at(parameter);
    return getNonConstantGroupIndex(group);
}

size_t ParameterProxyManager::getNonConstantGroupIndex(ParameterGroup* group) const
{
    assert(!group->isConstant());
    return nonConstantGroupIndexes.at(group);
}

OptimizedParameter* ParameterProxyManager::getOptimizedParameter(Parameter* parameter) const
{
    ParameterGroup* group = getParameterGroup(parameter);
    assert(!group->isConstant());
    return group->getValuePtr();
}

double ParameterProxyManager::getOptimizedParameterValue(Parameter* parameter) const
{
    ParameterGroup* group = getParameterGroup(parameter);
    return group->getValue();
}

OptimizedVector ParameterProxyManager::getOptimizedParameterValues() const
{
    OptimizedVector result;
    for(ParameterGroup* group: orderedNonConstantGroups)
    {
        result.set(group->getValuePtr(), group->getValue());
    }
    return result;
}

void ParameterProxyManager::setOptimizedParameterValues(const OptimizedVector& vals) const
{
    for(ParameterGroup* group: orderedNonConstantGroups)
    {
        group->setValue(vals[group->getValuePtr()]);
    }
}

OptimizedVector ParameterProxyManager::optimizeVector(const ParameterVector& v) const
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

size_t ParameterProxyManager::getGroupIndex(ParameterGroup* group) const
{
    return nonConstantGroupIndexes.at(group);
}

ParameterGroup* ParameterProxyManager::getGroup(size_t index) const
{
    return orderedNonConstantGroups.at(index);
}

size_t ParameterProxyManager::getEquationIndex(Equation* eq) const
{
    return equationIndexes.at(eq);
}

Equation* ParameterProxyManager::getEquation(size_t index) const
{
    return orderedEquations.at(index);
}

size_t ParameterProxyManager::inputSize() const
{
    assert(orderedNonConstantGroups.size() == nonConstantGroupIndexes.size());
    return orderedNonConstantGroups.size();
}

size_t ParameterProxyManager::outputSize() const
{
    assert(orderedEquations.size() == equationIndexes.size());
    return orderedEquations.size();
}

void ParameterProxyManager::commitParameters() const
{
    for(auto& group: parameterGroups)
    {
        group->commit();
    }
}

} // namespace NamedSketcher::GCS
