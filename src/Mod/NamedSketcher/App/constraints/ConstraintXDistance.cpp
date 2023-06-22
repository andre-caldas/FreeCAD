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
#include <initializer_list>

#include <Base/Persistence.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Exception.h>

#include "../geometries/GeometryPoint.h"
#include "ConstraintXDistance.h"


namespace NamedSketcher
{

TYPESYSTEM_SOURCE(ConstraintXDistance, ConstraintBase)

ConstraintXDistance::ConstraintXDistance()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::ConstraintXDistance should not be constructed without arguments.");
}

template<typename ref_pt, typename ref_par,
         std::enable_if_t<std::is_constructible_v<ConstraintXDistance::ref_point, ref_pt>>*,
         std::enable_if_t<std::is_constructible_v<ConstraintXDistance::ref_parameter, ref_par>>*>
ConstraintXDistance::ConstraintXDistance(ref_pt&& start, ref_pt&& end, ref_par&& distance)
    : start(std::forward(start))
    , end(std::forward(end))
    , distance(std::forward(distance))
{
}

std::vector<GCS::Equation*> ConstraintXDistance::getEquations()
{
    if(!start.isLocked())
    {
        start.refreshLock();
    }
    if(!end.isLocked())
    {
        end.refreshLock();
    }
    if(!distance.isLocked())
    {
        distance.refreshLock();
    }
    if(!start.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << start.pathString() << ").");
    }
    if(!end.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << end.pathString() << ").");
    }
    if(!distance.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << distance.pathString() << ").");
    }

    equation.set(&start.get()->x, &end.get()->x, distance.get());
    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintXDistance::updateReferences()
{
    start.refreshLock();
    end.refreshLock();
    distance.refreshLock();
    if(!start.hasChanged() && !end.hasChanged() && !distance.hasChanged())
    {
        return false;
    }
    equation.set(&start.get()->x, &end.get()->x, distance.get());
    return true;
}


unsigned int ConstraintXDistance::getMemSize () const
{
    return sizeof(ConstraintXDistance) + 100/*start.memSize() + end.memSize()*/;
}

void ConstraintXDistance::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}
void ConstraintXDistance::Restore(Base::XMLReader& /*reader*/)
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintXDistance>
ConstraintXDistance::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}

} // namespace NamedSketcher
