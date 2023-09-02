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

#include <Base/Writer.h>
#include <Base/Exception.h>

#include "ConstraintPointSymmetric.h"

namespace NamedSketcher
{

ConstraintPointSymmetric::ConstraintPointSymmetric(ref_point a, ref_point o, ref_point b)
    : a(std::move(a))
    , o(std::move(o))
    , b(std::move(b))
{
}

std::vector<GCS::Equation*> ConstraintPointSymmetric::getEquations()
{
    if(!a.isLocked())
    {
        a.refreshLock();
    }
    if(!o.isLocked())
    {
        o.refreshLock();
    }
    if(!b.isLocked())
    {
        b.refreshLock();
    }
    if(!a.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << a.pathString() << ").");
    }
    if(!o.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << o.pathString() << ").");
    }
    if(!b.isLocked())
    {
        FC_THROWM(Base::NameError, "Could not resolve name (" << b.pathString() << ").");
    }

    equationX.set(&a.get()->x, &o.get()->x, &b.get()->x);
    equationY.set(&a.get()->y, &o.get()->y, &b.get()->y);
    return std::vector<GCS::Equation*>{&equationX, &equationY};
}

bool ConstraintPointSymmetric::updateReferences()
{
    a.refreshLock();
    o.refreshLock();
    b.refreshLock();
    if(!a.hasChanged() && !o.hasChanged() && !b.hasChanged())
    {
        return false;
    }
    equationX.set(&a.get()->x, &o.get()->x, &b.get()->x);
    equationY.set(&a.get()->y, &o.get()->y, &b.get()->y);
    return true;
}


unsigned int ConstraintPointSymmetric::getMemSize () const
{
    return sizeof(ConstraintPointSymmetric) + 50/*a.memSize() + b.memSize()*/;
}

void ConstraintPointSymmetric::Save (Base::Writer& /*writer*/) const
{
    THROW(Base::NotImplementedError);
}

std::unique_ptr<ConstraintPointSymmetric>
ConstraintPointSymmetric::staticRestore(Base::XMLReader& /*reader*/)
{
    // SEE ConstraintCoincident.
    THROW(Base::NotImplementedError);
}


void ConstraintPointSymmetric::report() const
{
    try
    {
        std::cerr << "PointSymmetric: ";
        std::cerr << *a.get();
        std::cerr << " <<==" << *o.get() << "==>> ";
        std::cerr << *b.get();
        std::cerr << std::endl;

        // TODO: implement with and without manager.
        std::cerr << "\tError: (";
        std::cerr << (a.get()->x + b.get()->x - 2.0*o.get()->x);
        std::cerr << ", ";
        std::cerr << (a.get()->y + b.get()->y - 2.0*o.get()->y);
        std::cerr << ")" << std::endl;
    } catch (...) {}
}

} // namespace NamedSketcher
