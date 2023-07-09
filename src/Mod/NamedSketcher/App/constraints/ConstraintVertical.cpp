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
#include "ConstraintVertical.h"


namespace NamedSketcher
{

ConstraintVertical::ConstraintVertical(ref_point start, ref_point end)
    : start(std::move(start))
    , end(std::move(end))
{
}

ConstraintVertical::ConstraintVertical(const Base::Accessor::PathToObject& p)
    : ConstraintVertical(p + "start", p + "end")
{
}

std::vector<GCS::Equation*> ConstraintVertical::getEquations()
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

    equation.set(&start.get()->x, &end.get()->x);
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintVertical::updateReferences()
{
    start.refreshLock();
    end.refreshLock();
    if(!start.hasChanged() && !end.hasChanged())
    {
        return false;
    }
    equation.set(&start.get()->x, &end.get()->x);
    return true;
}


unsigned int ConstraintVertical::getMemSize () const
{
    return sizeof(ConstraintVertical) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintVertical::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintVertical>
ConstraintVertical::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintVertical::report() const
{
    try
    {
        std::cout << "Vertical: ";
        std::cout << "(" << start.get()->x << ", " << start.get()->y << ")";
        std::cout << " --> ";
        std::cout << "(" << end.get()->x << ", " << end.get()->y << ")";
        std::cout << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
