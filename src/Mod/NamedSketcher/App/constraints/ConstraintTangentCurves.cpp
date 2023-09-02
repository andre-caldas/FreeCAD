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
#include <memory>
#endif // _PreComp_
#include <cmath>

#include <Base/Writer.h>
#include <Base/Exception.h>

#include "../geometries/GeometryLineSegment.h"
#include "../geometries/GeometryCircle.h"

#include "tangent_curves/TangentCurvesGeneric.h"
#include "tangent_curves/TangentCurvesLineLine.h"
#include "tangent_curves/TangentCurvesLineCircle.h"
#include "tangent_curves/TangentCurvesCircleCircle.h"

#include "ConstraintTangentCurves.h"


namespace NamedSketcher
{

ConstraintTangentCurves::ConstraintTangentCurves(ref_geometry curve1, ref_geometry curve2)
    : curve1(std::move(curve1))
    , curve2(std::move(curve2))
{
}

std::vector<GCS::Equation*> ConstraintTangentCurves::getEquations()
{
    updateReferences(true);
    if(!curve1.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << curve1.pathString() << ").");
    }
    if(!curve2.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << curve2.pathString() << ").");
    }

    return std::vector<GCS::Equation*>{&equation1, &equation2};
}

bool ConstraintTangentCurves::updateReferences()
{
    return updateReferences(false);
}

bool ConstraintTangentCurves::updateReferences(bool only_unlocked)
{
    if(!only_unlocked || !curve1.isLocked())
    {
        curve1.refreshLock();
    }
    if(!only_unlocked || !curve2.isLocked())
    {
        curve2.refreshLock();
    }
    if(!curve1.hasChanged() && !curve2.hasChanged())
    {
        return false;
    }

    pickImplementation();
    assert(implementation);
    implementation->preprocessParameters();
    implementation->setEquations();
    return true;
}

void ConstraintTangentCurves::pickImplementation()
{
    // This method is private.
    // We assume all curves are locked.
    GeometryCircle* c1 = dynamic_cast<GeometryCircle*>(curve1.get());
    GeometryCircle* c2 = dynamic_cast<GeometryCircle*>(curve2.get());
    GeometryLineSegment* l1 = dynamic_cast<GeometryLineSegment*>(curve1.get());
    GeometryLineSegment* l2 = dynamic_cast<GeometryLineSegment*>(curve2.get());

    if(c1 && c2)
    {
        double in_limit = std::max(c1->radius, c2->radius);
        double in_limit_2 = in_limit * in_limit;
        double dx = (c2->center.x - c1->center.x);
        double dy = (c2->center.y - c1->center.y);
        double d_2 = dx*dx + dy*dy;

        implementation = std::make_unique<Specialization::TangentCurvesCircleCircle>(equation1, equation2, c1, c2, d_2 <= in_limit_2);
        return;
    }

    if(c1 && l2)
    {
        double px = (c1->center.x - l2->start.x);
        double py = (c1->center.y - l2->start.y);
        double vx = (l2->end.x - l2->start.x);
        double vy = (l2->end.y - l2->start.y);
        double det = px*vy - py*vx;

        implementation = std::make_unique<Specialization::TangentCurvesLineCircle>(equation1, equation2, c1, l2, det >= 0);
        return;
    }

    if(l1 && c2)
    {
        double px = (c2->center.x - l1->start.x);
        double py = (c2->center.y - l1->start.y);
        double vx = (l1->end.x - l1->start.x);
        double vy = (l1->end.y - l1->start.y);
        double det = px*vy - py*vx;

        implementation = std::make_unique<Specialization::TangentCurvesLineCircle>(equation1, equation2, l1, c2, det >= 0);
        return;
    }

    if(l1 && l2)
    {
        implementation = std::make_unique<Specialization::TangentCurvesLineLine>(equation1, equation2, l1, l2);
        return;
    }

    implementation = std::make_unique<Specialization::TangentCurvesGeneric>(equation1, equation2, curve1.get(), curve2.get(), &parameter_t1, &parameter_t2);
}

unsigned int ConstraintTangentCurves::getMemSize () const
{
    return sizeof(ConstraintTangentCurves) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintTangentCurves::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintTangentCurves>
ConstraintTangentCurves::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintTangentCurves::report() const
{
    std::cerr << "Tangent curves - ";
    implementation->report();
}

} // namespace NamedSketcher
