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

#include "../geometries/GeometryPoint.h"
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
    if(!curve1.isLocked())
    {
        curve1.refreshLock();
    }
    if(!curve2.isLocked())
    {
        curve2.refreshLock();
    }
    if(!curve1.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << curve1.pathString() << ").");
    }
    if(!curve2.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << curve2.pathString() << ").");
    }

    preprocessParameters();
    equationConcurrent.set(curve1.get(), &parameter_t1, curve2.get(), &parameter_t2);
    equationParallel.set(curve1.get(), &parameter_t1, curve2.get(), &parameter_t2);
    return std::vector<GCS::Equation*>{&equationConcurrent, &equationParallel};
}

bool ConstraintTangentCurves::updateReferences()
{
    curve1.refreshLock();
    curve2.refreshLock();
    if(!curve1.hasChanged() && !curve2.hasChanged())
    {
        return false;
    }

    preprocessParameters();
    equationConcurrent.set(curve1.get(), &parameter_t1, curve2.get(), &parameter_t2);
    equationParallel.set(curve1.get(), &parameter_t1, curve2.get(), &parameter_t2);
    return true;
}

void ConstraintTangentCurves::preprocessParameters()
{
    // TODO: improve this search.
    double min_det = 100000000.;
    for(GCS::Parameter p1(0); p1 <= 1.0; p1 += 0x1p-4)
    {
        for(GCS::Parameter p2(0); p2 <= 1.0; p2 += 0x1p-4)
        {
            auto n1 = curve1.get()->normalAtParameter({}, &p1);
            auto n2 = curve2.get()->normalAtParameter({}, &p2);
            double det = std::abs(n1.x*n2.y - n1.y*n2.x);
            if(det <= min_det)
            {
                min_det = det;
                parameter_t1 = p1;
                parameter_t2 = p2;
            }
        }
    }
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
    auto pt_curve1 = curve1.get()->positionAtParameter({}, &parameter_t1);
    auto pt_curve2 = curve2.get()->positionAtParameter({}, &parameter_t2);
    auto n1 = curve1.get()->normalAtParameter({}, &parameter_t1);
    auto n2 = curve2.get()->normalAtParameter({}, &parameter_t2);
    try
    {
        std::cerr << "Tangent curves: ";
        std::cerr << "* Curve 1 " << parameter_t1 << " -> " << pt_curve1 << ". ";
        std::cerr << "Normal 1 " << n1 << ".";
        std::cerr << std::endl;

        std::cerr << "* Curve 2 " << parameter_t2 << " -> " << pt_curve2 << ". ";
        std::cerr << "Normal 2 " << n2 << ".";
        std::cerr << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
