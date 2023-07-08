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

#include <iostream>

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
    // No parameter can be added after we start optimizing them.
    assert(orderedNonConstantGroups.empty());

    if(hasParameter(a))
    {
        return;
    }

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

bool ParameterGroupManager::setParameterEqual(const Parameter* a, const Parameter* b)
{
    // No parameter can be added after we start optimizing them.
    assert(orderedNonConstantGroups.empty());

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

bool ParameterGroupManager::areParametersEqual(const Parameter* a, const Parameter* b) const
{
    if(parameter2Group.count(a) == 0 || parameter2Group.count(b) == 0)
    {
        FC_THROWM(Base::RuntimeError, "Equating parameters that are not managed.");
    }

    return (parameter2Group.at(a) == parameter2Group.at(b));
}

ParameterGroup* ParameterGroupManager::getParameterGroup(const Parameter* parameter) const
{
    return parameter2Group.at(parameter);
}

bool ParameterGroupManager::setParameterConstant(Parameter* k)
{
    // No parameter can be added after we start optimizing them.
    assert(orderedNonConstantGroups.empty());

    return getParameterGroup(k)->setConstant(k);
}

bool ParameterGroupManager::isParameterConstant(const Parameter* p)
{
    return getParameterGroup(p)->isConstant();
}

void ParameterGroupManager::finishOptimization()
{
    called_finish_optimization = true;
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

size_t ParameterGroupManager::getOptimizedParameterIndex(const OptimizedParameter* parameter) const
{
    // Did you call finishOptimization()?
    assert(called_finish_optimization);

    ParameterGroup* group = optimizedParameter2NonConstantGroup.at(parameter);
    return getNonConstantGroupIndex(group);
}

size_t ParameterGroupManager::getNonConstantGroupIndex(const ParameterGroup* group) const
{
    assert(!group->isConstant());
    return nonConstantGroupIndexes.at(group);
}

OptimizedParameter* ParameterGroupManager::getOptimizedParameter(const Parameter* parameter) const
{
    // Did you call finishOptimization()?
    assert(called_finish_optimization);

    ParameterGroup* group = getParameterGroup(parameter);
    assert(!group->isConstant());
    return group->getValuePtr();
}

double ParameterGroupManager::getOptimizedParameterValue(const Parameter* parameter) const
{
    // Did you call finishOptimization()?
    assert(called_finish_optimization);

    ParameterGroup* group = getParameterGroup(parameter);
    return group->getValue();
}

OptimizedVector ParameterGroupManager::getOptimizedParameterValues() const
{
    // Did you call finishOptimization()?
    assert(called_finish_optimization);

    OptimizedVector result;
    for(ParameterGroup* group: orderedNonConstantGroups)
    {
        result.set(group->getValuePtr(), group->getValue());
    }
    return result;
}

void ParameterGroupManager::setOptimizedParameterValues(const OptimizedVector& vals) const
{
    // Did you call finishOptimization()?
    assert(called_finish_optimization);

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

size_t ParameterGroupManager::getGroupIndex(const ParameterGroup* group) const
{
    return nonConstantGroupIndexes.at(group);
}

ParameterGroup* ParameterGroupManager::getGroup(size_t index) const
{
    return orderedNonConstantGroups.at(index);
}

bool ParameterGroupManager::hasEquation(const Equation* eq) const
{
    return (equationIndexes.count(eq) != 0);
}

size_t ParameterGroupManager::getEquationIndex(const Equation* eq) const
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


void ParameterGroupManager::print_vector(const OptimizedVector& v) const
{
    std::cout << "(";
    bool first = true;
    for(auto& group: orderedNonConstantGroups)
    {
        if(!first)
        {
            std::cout << ", ";
        }
        else
        {
            first = false;
        }
        std::cout << v[group->getValuePtr()];
    }
    std::cout << ")";
}

void ParameterGroupManager::report_position() const
{
    std::cout << "Current position:" << std::endl;
    for(auto& group: orderedNonConstantGroups)
    {
        group->report();
    }
}

void ParameterGroupManager::report() const
{
    std::cout << "Groups" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << std::endl;
    for(auto& group: parameterGroups)
    {
        group->report();
    }

    std::cout << std::endl;
    std::cout << "Equations" << std::endl;
    std::cout << "---------" << std::endl;
    std::cout << std::endl;
    for(auto& eq: orderedEquations)
    {
(void*)eq;//        eq->report();
    }
}

} // namespace NamedSketcher::GCS
