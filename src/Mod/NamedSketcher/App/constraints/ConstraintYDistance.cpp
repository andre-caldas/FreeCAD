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

#include <Base/Writer.h>
#include <Base/Exception.h>

#include "../geometries/GeometryPoint.h"
#include "ConstraintYDistance.h"


namespace NamedSketcher
{

ConstraintYDistance::ConstraintYDistance(ref_point start, ref_point end, double distance)
    : start(std::move(start))
    , end(std::move(end))
    , distance(distance)
{
}

ConstraintYDistance::ConstraintYDistance(const Base::Accessor::PathToObject& p, double distance)
    : ConstraintYDistance(p + "start", p + "end", distance)
{
}

std::vector<GCS::Equation*> ConstraintYDistance::getEquations()
{
    if(!start.isLocked())
    {
        start.refreshLock();
    }
    if(!end.isLocked())
    {
        end.refreshLock();
    }
    if(!start.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << start.pathString() << ").");
    }
    if(!end.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << end.pathString() << ").");
    }

    equation.set(&start.get()->y, &end.get()->y, &distance);
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintYDistance::updateReferences()
{
    start.refreshLock();
    end.refreshLock();
    if(!start.hasChanged() && !end.hasChanged())
    {
        return false;
    }
    equation.set(&start.get()->y, &end.get()->y, &distance);
    return true;
}


unsigned int ConstraintYDistance::getMemSize () const
{
    return sizeof(ConstraintYDistance) + 100/*start.memSize() + end.memSize()*/;
}

void ConstraintYDistance::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintYDistance>
ConstraintYDistance::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintYDistance::report() const
{
    try
    {
        std::cerr << "Distance Y-direction: ";
        std::cerr << *start.get();
        std::cerr << " --> ";
        std::cerr << *end.get();
        std::cerr << ", distance = " << distance;
        std::cerr << std::endl;

        std::cerr << "\tError: (";
        std::cerr << (end.get()->y - start.get()->y - distance);
        std::cerr << ")" << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
