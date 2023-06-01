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

#include "ParameterProxy.h"
#include "ParameterGroup.h"

namespace NamedSketcher::GCS
{

ParameterGroup::ParameterGroup(ParameterProxy* a, ParameterProxy* b)
{
    append(a);
    append(b);
}

ParameterGroup::~ParameterGroup()
{
    for(auto p: parameters) p->resetProxy();
}

bool ParameterGroup::hasParameter(ParameterProxy* parameter) const
{
    return parameters.count(parameter);
}

void ParameterGroup::append(ParameterProxy* p)
{
    if(p->hasProxy())
    {
        FC_THROWM(Base::RuntimeError, "Cannot set proxy for parameter that already has a proxy.")
    }
    p->setProxy(&value);
    parameters.insert(p);
}

ParameterGroup& operator<<(ParameterGroup&& other)
{
    for(auto p: other.parameters)
    {
        p->resetProxy();
        append(p);
    }
    other.parameters.clear();
}

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterGroup_H
