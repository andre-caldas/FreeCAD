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

#include <Base/Writer.h>

#include "ConstraintFactory.h"
#include "ConstraintBase.h"

namespace NamedSketcher {

TYPESYSTEM_SOURCE_ABSTRACT(ConstraintBase, Base::Persistence)

void ConstraintBase::Restore(Base::XMLReader& /*reader*/)
{
    FC_THROWM(Base::NotImplementedError, "Restore is provided by the ConstraintBase::factory");
}

void ConstraintBase::SaveHead(Base::Writer& writer) const
{
    writer.Stream() << writer.ind() << "<" << xmlTagName()
                    << " type=\"" << xmlTagType() << "\""
                    << xmlAttributes() << ">" << std::endl;
    writer.incInd();
}

void ConstraintBase::SaveTail(Base::Writer& writer) const
{
    writer.decInd();
    writer.Stream() << writer.ind() << "</" << xmlTagName() << ">" << std::endl;
}

} // namespace NamedSketcher
