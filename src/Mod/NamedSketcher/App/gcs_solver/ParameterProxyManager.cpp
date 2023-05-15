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

#include <unordered_set>

#include "ProxiedParameter.h"
#include "ParameterGroup.h"
#include "ParameterProxyManager.h"

namespace NamedSketcher::GCS
{

void ParameterProxyManager::addGroup(ParameterGroup* group, double value)
{
    parameterGroups.insert(group);
    auto e = partition.emplace(group);
    assert(e.second); // Emplace succeeds.
    auto equivalents = e.first; // Emplaced EquivalentGroups.
    equivalents->value = value;

    for(auto parameter: *group)
    {
        processUnion(*equivalents, parameter);
    }
}

void ParameterProxyManager::informParameterAdditionToGroup(ParameterGroup* group, ProxiedParameter* parameter)
{
    for(auto& equivalents: partition)
    {
        if(equivalents.groups.count(group))
        {
            processUnion(equivalents, parameter);
            return;
        }
    }
    assert(false);
}

void ParameterProxyManager::processUnion(EquivalentGroups& equivalents, ProxiedParameter* parameter)
{
    for(auto it = partition.begin(); it != partition.end();)
    {
        // This is what a for range loop should do!
        // Increment just after assignment, so extraction does not invalidate it.
        EquivalentGroups& other_equivalent_groups = *it;
        ++it;

        if(&equivalents == &other_equivalent_groups)
        {
            continue;
        }

        for(ParameterGroup& grp: other_equivalent_groups)
        {
            if(grp.hasParameter(parameter))
            {
                auto extracted = partition.extract(other_equivalent_groups);
                equivalents << extracted.value();
                break;
            }
        }
    }
}

void ParameterProxyManager::removeGroup(ParameterGroup* group)
{
    for(auto& equivalents: partition)
    {
        if(equivalents.groups.count(group))
        {
            equivalents.groups.erase(group);
            splitDisjoints(equivalents);
            parameterGroups.erase(group);
            return;
        }
    }
    assert(false);
}

void informParameterRemovalFromGroup(ParameterGroup* group)
{
    for(auto& equivalents: partition)
    {
        if(equivalents.groups.count(group))
        {
            splitDisjoints(equivalents);
            return;
        }
    }
    assert(false);
}

void ParameterProxyManager::splitDisjoints(EquivalentGroups& equivalents)
{
    [[maybe_unused]] // We don't want equivalents to be destroyed!
    auto removed = partition.extract(equivalents);
    for(auto group: equivalents)
    {
        // Not very efficient, because we compare with all other sets,
        // which we know to be disjoint.
        addGroup(group, equivalents.value);
    }
}

ParameterProxyManager::EquivalentGroups::EquivalentGroups(ParameterGroup* group)
    : value(0)
{
    for(ProxiedParameter* parameter: group)
    {
        value += parameter->getValue() / group->size();
    }
    *this << group;
}

ParameterProxyManager::EquivalentGroups&
ParameterProxyManager::EquivalentGroups::operator<<(ParameterGroup* addedGroup)
{
    groups.emplace(addedGroup);
    addedGroup->setPointer(&value);
    return *this;
}

ParameterProxyManager::EquivalentGroups&
ParameterProxyManager::EquivalentGroups::operator<<(EquivalentGroups& addedGroups)
{
    for(ParameterGroup* group: addedGroups.groups)
    {
        *this << group;
    }
    addedGroups.clear();
    return *this;
}

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterProxyManager_H
