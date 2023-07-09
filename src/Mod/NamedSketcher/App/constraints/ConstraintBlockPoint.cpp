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
#include <utility>
#endif // _PreComp_

#include <iostream>

#include <Base/Writer.h>
#include <Base/Exception.h>

#include "../geometries/GeometryPoint.h"
#include "ConstraintBlockPoint.h"


namespace NamedSketcher
{

ConstraintBlockPoint::ConstraintBlockPoint(ref_point point, double x, double y)
    : point(std::move(point))
    , kX(x)
    , kY(y)
{
}

std::vector<GCS::Equation*> ConstraintBlockPoint::getEquations()
{
    if(!point.isLocked())
    {
        point.refreshLock();
    }
    if(!point.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << point.pathString() << ").");
    }

    equationConstantX.set(&point.get()->x, &kX);
    equationConstantY.set(&point.get()->y, &kY);
    return std::vector<GCS::Equation*>{&equationConstantX, &equationConstantY};
}

bool ConstraintBlockPoint::updateReferences()
{
    point.refreshLock();
    if(!point.hasChanged())
    {
        return false;
    }
    equationConstantX.set(&point.get()->x, &kX);
    equationConstantY.set(&point.get()->y, &kY);
    return true;
}


unsigned int ConstraintBlockPoint::getMemSize () const
{
    return sizeof(ConstraintBlockPoint) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintBlockPoint::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintBlockPoint>
ConstraintBlockPoint::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintBlockPoint::report() const
{
    try
    {
        std::cout << "BlockPoint: ";
        std::cout << *point.get();
        std::cout << " --> ";
        std::cout << "(" << kX << ", " << kY << ")";
        std::cout << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
