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

#include "../gcs_solver/equations/Equal.h"
#include "ConstraintEqual.h"


namespace NamedSketcher
{

ConstraintEqual::ConstraintEqual(ref_parameter a, ref_parameter b)
    : a(std::move(a))
    , b(std::move(b))
{
}

std::vector<GCS::Equation*> ConstraintEqual::getEquations()
{
    if(!a.isLocked())
    {
        a.refreshLock();
    }
    if(!b.isLocked())
    {
        b.refreshLock();
    }
    if(!a.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << a.pathString() << ").");
    }
    if(!b.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << b.pathString() << ").");
    }

    equation.set(a.get(), b.get());
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintEqual::updateReferences()
{
    a.refreshLock();
    b.refreshLock();
    if(!a.hasChanged() && !b.hasChanged())
    {
        return false;
    }
    equation.set(a.get(), b.get());
    return true;
}


unsigned int ConstraintEqual::getMemSize () const
{
    return sizeof(ConstraintEqual) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintEqual::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintEqual>
ConstraintEqual::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintEqual::report() const
{
    try
    {
        std::cout << "Equal (" << this << "): ";
        std::cout << "(" << *a.get() << ")";
        std::cout << " == ";
        std::cout << "(" << *b.get() << ")";
        std::cout << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
