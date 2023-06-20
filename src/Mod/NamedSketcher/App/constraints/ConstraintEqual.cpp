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

#include <Base/Persistence.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Exception.h>

#include "../gcs_solver/equations/Equal.h"
#include "ConstraintEqual.h"


namespace NamedSketcher
{

TYPESYSTEM_SOURCE(ConstraintEqual, ConstraintBase)

ConstraintEqual::ConstraintEqual()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::ConstraintEqual should not be constructed without arguments.");
}

template<typename ref,
         std::enable_if_t<std::is_constructible_v<ConstraintEqual::ref_parameter, ref>>*>
ConstraintEqual::ConstraintEqual(ref&& a, ref&& b)
    : a(std::forward(a))
    , b(std::forward(b))
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
void ConstraintEqual::Restore(Base::XMLReader& /*reader*/)
{
    THROW(Base::NotImplementedError);
}

} // namespace NamedSketcher
