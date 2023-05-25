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

void ParameterProxyManager::setEqual(ProxiedParameter* a, ProxiedParameter* b)
{
    auto has_a = parameterGroups.end();
    auto has_b = parameterGroups.end();
    for(auto group = parameterGroups.begin(); group != parameterGroups.end(); ++group)
    {
        if((*group)->hasParameter(a))
        {
            has_a = group;
        }
        if((*group)->hasParameter(b))
        {
            has_b = group;
        }
        if(has_a && has_b)
        {
            break;
        }
    }

    if(!has_a && has_b)
    {
        (*has_b)->append(a);
        return
    }
    if(has_a && !has_b)
    {
        (*has_a)->append(b);
        return
    }
    if(!has_a && !has_b)
    {
        parameterGroups.emplace(std::make_unique<ParameterGroup>(a, b));
        return
    }

    assert(has_a && has_b);
    **has_a << std::move(**has_b);
    parameterGroups.extract(has_b);
}

} // namespace NamedSketcher::GCS
