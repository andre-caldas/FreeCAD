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

#include <numeric>

#include <Base/Persistence.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Exception.h>

#include "ConstraintCoincident.h"


namespace App::NamedSketcher
{

TYPESYSTEM_SOURCE(ConstraintCoincident, Base::Persistence)

template<typename ref, typename>
ConstraintCoincident& ConstraintCoincident::addPoint(ref&& reference)
{
    references.emplace_back(reference);
    return *this;
}

ConstraintCoincident& ConstraintCoincident::removePoint(boost::uuids::uuid tag)
{
    THROW(Base::NotImplementedError);
    return *this;
}


unsigned int ConstraintCoincident::getMemSize () const
{
    return std::accumulate(references.cbegin(), references.cend(), 0, [](auto i){return i->memSize();});
}

void ConstraintCoincident::Save (Base::Writer& writer) const
{
    THROW(Base::NotImplementedError);
}
void ConstraintCoincident::Restore(Base::XMLReader& /*reader*/)
{
    THROW(Base::NotImplementedError);
}

} // namespace App::NamedSketcher
