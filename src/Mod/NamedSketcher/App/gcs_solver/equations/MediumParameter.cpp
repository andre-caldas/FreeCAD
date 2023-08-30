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

#include "../parameters/ParameterGroupManager.h"
#include "../parameters/ParameterValueMapper.h"
#include "MediumParameter.h"

namespace NamedSketcher::GCS
{

void MediumParameter::set(Parameter* x, Parameter* m, Parameter* y)
{
    if(x == y || x == m || y == m)
    {
        FC_THROWM(Base::ReferenceError, "Different parameters must be passed.");
    }
    a = x;
    o = m;
    b = y;
}

double MediumParameter::error(const ParameterGroupManager& manager) const
{
    const double A = manager.getValue(a);
    const double B = manager.getValue(b);
    const double O = manager.getValue(o);
    return B + A - 2*O;
}

ParameterVector MediumParameter::differentialNonOptimized(const GCS::ParameterValueMapper& /*parameter_mapper*/) const
{
    ParameterVector result;
    result.set(a, 1);
    result.set(o, -2);
    result.set(b, 1);
    return result;
}

OptimizedVector MediumParameter::differentialOptimized(const ParameterGroupManager& manager) const
{
    bool a_const = manager.isParameterConstant(a);
    bool b_const = manager.isParameterConstant(b);
    bool o_const = manager.isParameterConstant(o);
    if(a_const && b_const && o_const)
    {
        return OptimizedVector();
    }

    if(manager.areParametersEqual(a, o) && manager.areParametersEqual(o, b))
    {
        return OptimizedVector();
    }
    assert(!manager.areParametersEqual(a, b));
    assert(!manager.areParametersEqual(a, o));
    assert(!manager.areParametersEqual(o, b));
    return manager.optimizeVector(differentialNonOptimized(manager));
}

void MediumParameter::declareParameters(ParameterGroupManager& manager) const
{
    manager.addParameter(a);
    manager.addParameter(o);
    manager.addParameter(b);
}

bool MediumParameter::optimizeParameters(ParameterGroupManager& manager) const
{
    bool ab_equal = manager.areParametersEqual(a, b);
    bool ao_equal = manager.areParametersEqual(a, o);
    bool ob_equal = manager.areParametersEqual(o, b);

    if(ab_equal && ao_equal && ob_equal)
    {
        return false;
    }

    if(ab_equal || ao_equal || ob_equal)
    {
        manager.setParameterEqual(a,o);
        manager.setParameterEqual(o,b);
        return true;
    }

    bool a_const = manager.isParameterConstant(a);
    bool b_const = manager.isParameterConstant(b);
    bool o_const = manager.isParameterConstant(o);

    double val_a = *manager.getOptimizedParameter(a, false);
    double val_b = *manager.getOptimizedParameter(b, false);
    double val_o = *manager.getOptimizedParameter(o, false);

    if(a_const && b_const && o_const)
    {
        return false;
    }

    if(a_const && b_const)
    {
        *o = (val_a + val_b)/2;
        manager.setParameterConstant(o);
        return true;
    }

    if(a_const && o_const)
    {
        *b = val_a + 2.0 * val_o;
        manager.setParameterConstant(b);
        return true;
    }

    if(b_const && o_const)
    {
        *a = val_b - 2.0 * val_o;
        manager.setParameterConstant(b);
        return true;
    }

    // TODO: Optimize when only one is const.
    // Doesn't make much difference, because it is already linear.

    return false;
}


void MediumParameter::report() const
{
    std::cerr << "Medium point";
    //    std::cerr << "PointAlongCurve: " << *point << " along " << *curve;
    std::cerr << std::endl;
}

} // namespace NamedSketcher::GCS
