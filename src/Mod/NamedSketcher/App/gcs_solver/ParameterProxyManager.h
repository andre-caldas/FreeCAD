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
#include <set>

#include "NamedSketcherGlobal.h"

namespace NamedSketcher::GCS
{

class ProxiedParameter;
class ParameterGroup;

class NamedSketcherExport ParameterProxyManager
{
public:
    void addGroup(ParameterGroup* group, double value);
    void removeGroup(ParameterGroup* group);
    void informParameterAdditionToGroup(ParameterGroup*, ProxiedParameter*);
    void informParameterRemovalFromGroup(ParameterGroup*);

private:
    std::unordered_set<ParameterGroup*> parameterGroups;

    struct EquivalentGroups
    {
        EquivalentGroups(ParameterGroup* group);

        double value;
        std::unordered_set<ParameterGroup*> groups;

        EquivalentGroups& operator<<(ParameterGroup* addedGroup);
        EquivalentGroups& operator<<(EquivalentGroups addedGroups);

        /**
         * @brief Comparison for using std::set<EquivalentGroups>.
         */
        bool operator<(const EquivalentGroups& other) const {return &value < &other.value;}
        /**
         * @brief Comparison for using std::set<EquivalentGroups>.
         */
        bool operator==(const EquivalentGroups& other) const {return &value == &other.value;}

        EquivalentGroups(const EquivalentGroups&) = delete;
        EquivalentGroups& operator=(const EquivalentGroups&) = delete;
        EquivalentGroups(EquivalentGroups&&) = delete;
        EquivalentGroups& operator=(EquivalentGroups&&) = delete;
    };

    /**
     * @brief This is redundant information.
     * Each gathers ParameterGroups that have common elements.
     */
    std::set<EquivalentGroups> partition;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterProxyManager_H
