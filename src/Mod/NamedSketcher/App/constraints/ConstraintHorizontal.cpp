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

#include "../geometries/GeometryPoint.h"
#include "ConstraintHorizontal.h"


namespace NamedSketcher
{

TYPESYSTEM_SOURCE(ConstraintHorizontal, ConstraintHorizontal)

ConstraintHorizontal::ConstraintHorizontal()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::ConstraintHorizontal should not be constructed without arguments.");
}

template<class ref,
         std::enable_if_t<std::is_constructible_v<ConstraintHorizontal::ref_point, ref>>*>
ConstraintHorizontal::ConstraintHorizontal(ref&& start, ref&& end)
    : start(std::forward(start))
    , end(std::forward(end))
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
void ConstraintHorizontal::Restore(Base::XMLReader& /*reader*/)
{
    THROW(Base::NotImplementedError);
}

} // namespace NamedSketcher
