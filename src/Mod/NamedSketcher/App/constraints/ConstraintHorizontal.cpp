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
#include "ConstraintHorizontal.h"


namespace NamedSketcher
{

ConstraintHorizontal::ConstraintHorizontal(ref_point start, ref_point end)
    : start(std::move(start))
    , end(std::move(end))
{
}

ConstraintHorizontal::ConstraintHorizontal(const Base::Accessor::PathToObject& p)
    : ConstraintHorizontal(p + "start", p + "end")
{
}

std::vector<GCS::Equation*> ConstraintHorizontal::getEquations()
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

    equation.set(&start.get()->y, &end.get()->y);
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintHorizontal::updateReferences()
{
    start.refreshLock();
    end.refreshLock();
    if(!start.hasChanged() && !end.hasChanged())
    {
        return false;
    }
    equation.set(&start.get()->y, &end.get()->y);
    return true;
}


unsigned int ConstraintHorizontal::getMemSize () const
{
    return sizeof(ConstraintHorizontal) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintHorizontal::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintHorizontal>
ConstraintHorizontal::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintHorizontal::report() const
{
    try
    {
        std::cout << "Horizontal (" << this << "): ";
        std::cout << "(" << start.get()->x << ", " << start.get()->y << ")";
        std::cout << " --> ";
        std::cout << "(" << end.get()->x << ", " << end.get()->y << ")";
        std::cout << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
