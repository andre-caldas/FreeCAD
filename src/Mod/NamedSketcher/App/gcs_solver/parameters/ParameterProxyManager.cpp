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

#include "ProxiedParameter.h"
#include "ParameterVector.h"
#include "ParameterGroup.h"
#include "ParameterProxyManager.h"

namespace NamedSketcher::GCS
{

ParameterProxyManager::~ParameterProxyManager()
{
    for(auto parameter: originalParameters)
    {
        parameter->resetProxy();
    }
}

void ParameterProxyManager::addParameter(ProxiedParameter* a)
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

void ParameterProxyManager::setParameterEqual(ProxiedParameter* a, ProxiedParameter* b)
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
    **has_a << std::move(**has_b);
    parameterGroups.extract(has_b);
    return true;
}

void ParameterProxyManager::setParameterIndexes() const
{
    for(auto& group: parameterGroups)
    {
        parameterIndexes.emplace({&group->value, parameterIndexes.size()});
        indexedParameterGroups.emplace_back(group.get());
        assert(parameterIndexes.size() == indexedParameterGroups.size());
    }
}

int ParameterProxyManager::getParameterIndex(double* valuePointer) const
{
    return parameterIndexes.at(valuePointer);
}

int ParameterProxyManager::getParameterIndex(ParameterGroup* group) const
{
    return getParameterIndex(&group->value);
}

OptimizedVector ParameterProxyManager::getOptimizedParameterVector() const
{
    OptimizedVector result;
    for(Group* group: indexedParameterGroups)
    {
        result.set(&group->value, group->value)
    }
}

OptimizedVector ParameterProxyManager::optimizeVector(const Vector& v) const
{
    OptimizedVector result;
    for(auto [parameter, value]: v.values)
    {
        if(result.hasKey(getProxyPointer(parameter)))
        {
            FC_THROWM(Base::RuntimeError, "Cannot optimize vector.");
        }
        result.set(getProxyPointer(parameter), value);
    }
    return result;
}

ParameterGroup* ParameterProxyManager::getGroup(int index) const
{
    if(index >= indexedParameterGroups.size())
    {
        FC_THROWM(Base::RuntimeError, "Index for proxied parameter group does not exist. This is a bug!");
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

int ParameterProxyManager::size() const
{
    assert(indexedParameterGroups.size() == parameterIndexes.size());
    assert(indexedParameterGroups.size() == parameterGroups.size());
    return indexedParameterGroups.size();
}

void ParameterProxyManager::commitValues() const
{
    for(auto parameter: originalParameters)
    {
        parameter->commit();
    }
}

} // namespace NamedSketcher::GCS
