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

#include <unordered_set>

#include "Parameter.h"
#include "Vector.h"
#include "ParameterGroup.h"
#include "ParameterProxyManager.h"

namespace NamedSketcher::GCS
{

void ParameterProxyManager::addParameter(Parameter* a)
{
    originalParameters.emplace_back(a);
    parameterGroups.emplace(std::make_unique<ParameterGroup>(a));
}

void ParameterProxyManager::addEquation(Equation* eq)
{
    equationIndexes.emplace({eq, equationIndexes.size()});
    equations.emplace_back(eq);
    assert(equations.size() == equationIndexes.size());
}

void ParameterProxyManager::setParameterEqual(Parameter* a, Parameter* b)
{
    auto end = parameterGroups.end();
    auto has_a = end;
    auto has_b = end;
    for(auto group = parameterGroups.begin(); group != end; ++group)
    {
        if((*group)->hasParameter(a))
        {
            has_a = group;
        }
        if((*group)->hasParameter(b))
        {
            has_b = group;
        }
        if(has_a != end && has_b != end)
        {
            break;
        }
    }

    if(has_a == end || has_b == end)
    {
        FC_THROWM(Base::RuntimeError, "Optimizing a non managed proxy. This is a bug!")
    }

    if(has_a == has_b)
    {
        return false;
    }

    for(auto p: (*has_b)->parameters)
    {
        append(p);
    }
    optimizedParameterIndexes.extract(&(*has_b)->value);
    parameterGroups.extract(has_b);
    return true;
}

bool ParameterProxyManager::areParametersEqual(Parameter* a, Parameter* b) const
{
    for(auto group = parameterGroups.begin(); group != end; ++group)
    {
        if((*group)->hasParameter(a))
        {
            return (*group)->hasParameter(b);
        }
        if((*group)->hasParameter(b))
        {
            return false;
        }
    }
    FC_THROWM(Base::RuntimeError, "Comparing parameters that are not managed.");
}

void ParameterProxyManager::setOptimizedParameterIndexes() const
{
    for(auto& group: parameterGroups)
    {
        optimizedParameterIndexes.emplace({&group->value, optimizedParameterIndexes.size()});
        indexedParameterGroups.emplace_back(group.get());
        assert(optimizedParameterIndexes.size() == indexedParameterGroups.size());
    }
}

int ParameterProxyManager::getOptimizedParameterIndex(OptimizedParameter* parameter) const
{
    return optimizedParameterIndexes.at(parameter);
}

int ParameterProxyManager::getParameterIndex(ParameterGroup* group) const
{
    return getParameterIndex(&group->value);
}

OptimizedParameter* ParameterProxyManager::getOptimizedParameter(Parameter* parameter) const
{
    for(ParameterGroup* group: indexedParameterGroups)
    {
        if(group->hasParameter(parameter))
        {
            return &group->value;
        }
    }
    FC_THROWM(Base::RuntimeError, "Attempt to optimize parameter: parameter not found.");
}

OptimizedVector ParameterProxyManager::getOptimizedParameterValues() const
{
    OptimizedVector result;
    for(ParameterGroup* group: indexedParameterGroups)
    {
        result.set(&group->value, group->value);
    }
}

void ParameterProxyManager::setOptimizedParameterValues(const OptimizedVector& vals) const
{
    for(ParameterGroup* group: indexedParameterGroups)
    {
        group->value = vals[&group->value];
    }
}

OptimizedVector ParameterProxyManager::optimizeVector(const ParameterVector& v) const
{
    OptimizedVector result;
    for(auto [parameter, value]: v.values)
    {
        if(result.hasKey(getOptimizedPointer(parameter)))
        {
            FC_THROWM(Base::RuntimeError, "Cannot optimize vector. This is a bug!");
        }
        result.set(getOptimizedPointer(parameter), value);
    }
    return result;
}

ParameterGroup* ParameterProxyManager::getGroup(int index) const
{
    if(index >= indexedParameterGroups.size())
    {
        FC_THROWM(Base::RuntimeError, "Index for parameter group does not exist. This is a bug!");
    }
    return indexedParameterGroups.at(index);
}

int ParameterProxyManager::getEquationIndex(Equation* eq) const
{
    return equationIndexes.at(eq);
}

int ParameterProxyManager::getEquation(int index) const
{
    return equations.at(index);
}

int ParameterProxyManager::inputSize() const
{
    assert(indexedParameterGroups.size() == optimizedParameterIndexes.size());
    assert(indexedParameterGroups.size() == parameterGroups.size());
    return indexedParameterGroups.size();
}

int ParameterProxyManager::outputSize() const
{
    assert(equations.size() == equationIndexes.size());
    return equations.size();
}

void ParameterProxyManager::commitValues() const
{
    for(auto parameter: originalParameters)
    {
        parameter->commit();
    }
}

} // namespace NamedSketcher::GCS
