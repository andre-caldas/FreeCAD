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

#include "../geometries/GeometryLineSegment.h"
#include "../geometries/GeometryCircle.h"

#include "point_along_curve/PointAlongCurveLine.h"
#include "point_along_curve/PointAlongCurveCircle.h"
#include "point_along_curve/PointAlongCurveGeneric.h"

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
    updateReferences(true);
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

    return std::vector<GCS::Equation*>{&equation};
}

bool ConstraintPointAlongCurve::updateReferences()
{
    return updateReferences(false);
}

bool ConstraintPointAlongCurve::updateReferences(bool only_unlocked)
{
    if(!only_unlocked || !point.isLocked())
    {
        point.refreshLock();
    }
    if(!only_unlocked || !curve.isLocked())
    {
        curve.refreshLock();
    }
    if(!point.hasChanged() && !curve.hasChanged())
    {
        return false;
    }

    pickImplementation();
    assert(implementation);
    implementation->preprocessParameters();
    implementation->setEquations();
    return true;
}

void ConstraintPointAlongCurve::pickImplementation()
{
    // This method is private.
    // We assume all curves are locked.
    GeometryLineSegment* line = dynamic_cast<GeometryLineSegment*>(curve.get());
    GeometryCircle* circle = dynamic_cast<GeometryCircle*>(curve.get());

    if(line)
    {
        implementation = std::make_unique<Specialization::PointAlongCurveLine>(equation, point.get(), line);
        return;
    }

    if(circle)
    {
        implementation = std::make_unique<Specialization::PointAlongCurveCircle>(equation, point.get(), circle);
        return;
    }

    implementation = std::make_unique<Specialization::PointAlongCurveGeneric>(equation, point.get(), curve.get(), &parameter_t);
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
    std::cerr << "Point along curve - ";
    implementation->report();
}

} // namespace NamedSketcher
