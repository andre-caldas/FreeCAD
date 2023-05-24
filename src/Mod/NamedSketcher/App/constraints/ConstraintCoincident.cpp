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
#include <Base/Vector3D.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "ConstraintCoincident.h"


namespace NamedSketcher
{

TYPESYSTEM_SOURCE_ABSTRACT(ConstraintCoincident, ConstraintBase)

ConstraintCoincident::ConstraintCoincident()
{
    // FreeCAD objects are not RAII. :-(
    FC_THROWM(Base::RuntimeError, "NamedSketcher::ConstraintCoincident should not be constructed without arguments.");
}

template<typename ref,
         std::enable_if_t<std::is_constructible_v<ConstraintCoincident::ref_type, ref>>*>
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

void ConstraintCoincident::appendParameterList(std::vector<GCS::ProxiedParameter*>&)
{
    THROW(Base::NotImplementedError);
}

unsigned int ConstraintCoincident::getMemSize () const
{
#if 0
    unsigned int size = 0;
    for(auto it = references.cbegin(); it != references.cend(); ++it)
    {
        size += it->memSize();
    }
    return size;
#endif
    return 15*references.size();
}

void ConstraintCoincident::Restore(Base::XMLReader& /*reader*/)
{
    THROW(Base::NotImplementedError);
}

void ConstraintCoincident::Save(Base::Writer& writer) const
{
    ConstraintBase::SaveHead(writer);
    for(auto& reference: references)
    {
        reference.serialize(writer);
    }
    ConstraintBase::SaveTail(writer);
}

std::unique_ptr<ConstraintCoincident>
ConstraintCoincident::staticRestore(Base::XMLReader& reader)
{
    auto result = std::make_unique<ConstraintCoincident>();
    while(reader.testEndElement(xmlTagNameStatic()))
    {
        ref_type reference = ref_type::unserialize(reader);
        result->addPoint(std::move(reference));
    }
    return result;
}

} // namespace NamedSketcher
