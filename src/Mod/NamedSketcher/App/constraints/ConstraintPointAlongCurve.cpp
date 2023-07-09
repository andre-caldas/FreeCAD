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

#include "PreCompiled.h"

#ifndef _PreComp_
#include <iostream>
#endif // _PreComp_
#include <cmath>

#include <Base/Writer.h>
#include <Base/Exception.h>

#include "../geometries/GeometryPoint.h"
#include "ConstraintPointAlongCurve.h"


namespace NamedSketcher
{

ConstraintPointAlongCurve::ConstraintPointAlongCurve(ref_point point, ref_geometry curve)
    : point(std::move(point))
    , curve(std::move(curve))
{
}

std::vector<GCS::Equation*> ConstraintPointAlongCurve::getEquations()
{
    if(!point.isLocked())
    {
        point.refreshLock();
    }
    if(!curve.isLocked())
    {
        curve.refreshLock();
    }
    if(!point.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << point.pathString() << ").");
    }
    if(!curve.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << curve.pathString() << ").");
    }

    equation.set(point.get(), curve.get(), &parameter_t);
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintPointAlongCurve::updateReferences()
{
    point.refreshLock();
    curve.refreshLock();
    if(!point.hasChanged() && !curve.hasChanged())
    {
        return false;
    }
    equation.set(point.get(), curve.get(), &parameter_t);
    return true;
}


unsigned int ConstraintPointAlongCurve::getMemSize () const
{
    return sizeof(ConstraintPointAlongCurve) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintPointAlongCurve::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintPointAlongCurve>
ConstraintPointAlongCurve::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintPointAlongCurve::report() const
{
    auto pt_curve = curve.get()->positionAtParameter({}, &parameter_t);
    try
    {
        std::cout << "Point along curve: ";
        std::cout << "candidate point " << *point.get();
        std::cout << " <-" << parameter_t << "-> ";
        std::cout << "curve(t) " << pt_curve;
        std::cout << std::endl;

        double diff_x = (point.get()->x - pt_curve.x);
        double diff_y = (point.get()->y - pt_curve.y);
        std::cout << "\tError: (";
        std::cout << std::sqrt(diff_x*diff_x + diff_y*diff_y);
        std::cout << ")" << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
