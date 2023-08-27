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

#include "Parameter.h"
#include "ParameterGroup.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace NamedSketcher::GCS
{

ParameterGroup::ParameterGroup(Parameter* parameter)
    : value(parameter->name)
{
    append(parameter);
}

double ParameterGroup::getValue() const
{
    if(isConstant())
    {
        return *const_parameter;
    }
    return value;
}

void ParameterGroup::setValue(double val)
{
    if(isConstant())
    {
        FC_THROWM(Base::RuntimeError, "Attempt change value of constant parameter. This is a bug!");
    }
    value = val;
}

OptimizedParameter* ParameterGroup::getValuePtr()
{
    if(isConstant())
    {
        FC_THROWM(Base::RuntimeError, "Attempt to get pointer for constant parameter. This is a bug!");
    }
    return &value;
}

bool ParameterGroup::hasParameter(Parameter* parameter) const
{
    return parameters.count(parameter);
}

void ParameterGroup::append(Parameter* p, bool set_as_mean)
{
    parameters.insert(p);
    if(set_as_mean)
    {
        setAsMean();
    }
}

bool ParameterGroup::setConstant(Parameter* k)
{
    if(!hasParameter(k))
    {
        FC_THROWM(Base::RuntimeError, "Constant parameter must belong to group. This is a bug!");
    }
    if(const_parameter != nullptr && const_parameter != k)
    {
        // This is not supposed to happen,
        // because two constant parameters would be, at best, redundant.
        FC_THROWM(Base::RuntimeError, "Parameter group is already constant. This is a bug!");
    }
    if(const_parameter == k)
    {
        FC_WARN("Setting the same constant twice??? Better investigate...");
        return false;
    }
    const_parameter = k;
    value = (double)*k;
    return true;
}

bool ParameterGroup::isConstant() const
{
    return (const_parameter != nullptr);
}

void ParameterGroup::commit() const
{
    for(auto parameter: parameters)
    {
        std::cerr << "Group (" << this << ") commit: " << value << std::endl;
        *parameter = (double)value;
    }
}

ParameterGroup& ParameterGroup::operator<<(ParameterGroup&& other)
{
    if(other.const_parameter != nullptr)
    {
        if(const_parameter != nullptr && const_parameter != other.const_parameter)
        {
            // This is not supposed to happen,
            // because two constant parameters would be, at best, redundant.
            FC_THROWM(Base::RuntimeError, "Merged groups are both constant. This is a bug!");
        }
        const_parameter = other.const_parameter;
    }
    for(auto p: other.parameters)
    {
        append(p, false);
    }
    other.parameters.clear();
    setAsMean();
    return *this;
}

void ParameterGroup::setAsMean()
{
    if(isConstant())
    {
        value = (double)*const_parameter;
        return;
    }

    if(parameters.size() == 0)
    {
        assert(false);
        return;
    }

    value = 0.0;
    for(const auto& parameter: parameters)
    {
        value += (double)*parameter;
    }
    value /= parameters.size();
}


void ParameterGroup::report() const
{
    std::cerr << "(" << this << " = " << value << "): ";
    for(auto& p: parameters)
    {
        std::cerr << "(" << *p << ") ";
    }
    if(isConstant())
    {
        std::cerr << "-->constant<--";
    }
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
