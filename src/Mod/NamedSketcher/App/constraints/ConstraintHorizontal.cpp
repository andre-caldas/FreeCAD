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

#include "ConstraintHorizontal.h"


namespace NamedSketcher
{

TYPESYSTEM_SOURCE(ConstraintHorizontal, ConstraintEqual)

ConstraintHorizontal::ConstraintHorizontal()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::ConstraintHorizontal should not be constructed without arguments.");
}

ConstraintHorizontal::ConstraintHorizontal(GCS::ParameterProxyManager& proxy_manager, const ref_type& start, const ref_type& end)
    : ConstraintEqual(proxy_manager, start.goFurther<double>("y"), end.goFurther<double>("y"))
{
}

void ConstraintHorizontal::appendParameterList(std::vector<double*>& parameters)
{
    THROW(Base::NotImplementedError);
}

void ConstraintHorizontal::Save (Base::Writer& writer) const
{
    THROW(Base::NotImplementedError);
}
void ConstraintHorizontal::Restore(Base::XMLReader& /*reader*/)
{
    THROW(Base::NotImplementedError);
}

} // namespace NamedSketcher
