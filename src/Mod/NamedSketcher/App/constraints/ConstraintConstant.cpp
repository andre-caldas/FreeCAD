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

#include "../gcs_solver/equations/Constant.h"
#include "ConstraintConstant.h"


namespace NamedSketcher
{

ConstraintConstant::ConstraintConstant(ref_parameter a, double value)
    : a(std::move(a))
    , k(value)
{
}

std::vector<GCS::Equation*> ConstraintConstant::getEquations()
{
    if(!a.isLocked())
    {
        a.refreshLock();
    }
    if(!a.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << a.pathString() << ").");
    }

    equation.set(a.get(), &k);
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintConstant::updateReferences()
{
    a.refreshLock();
    if(!a.hasChanged())
    {
        return false;
    }
    equation.set(a.get(), &k);
    return true;
}


unsigned int ConstraintConstant::getMemSize () const
{
    return sizeof(ConstraintConstant) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintConstant::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintConstant>
ConstraintConstant::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintConstant::report() const
{
    try
    {
        std::cout << "Constant (" << this << "): ";
        std::cout << "(constant: " << k << ")";
        std::cout << " --> ";
        std::cout << "(" << *a.get() << ")";
        std::cout << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
