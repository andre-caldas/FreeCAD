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
#include <memory>

#include <Base/Writer.h>
#include <Base/Exception.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "../gcs_solver/equations/Equal.h"
#include "geometries/GeometryPoint.h"
#include "ConstraintCoincident.h"

namespace NamedSketcher
{

ConstraintCoincident::ConstraintCoincident(ref_parameter a, ref_parameter b)
{
    addPoint(std::move(a));
    addPoint(std::move(b));
}

void ConstraintCoincident::addPoint(ref_point reference)
{
    references.emplace_back(std::move(reference));
    if(references.size() > 1)
    {
        equations.emplace_back(std::make_unique<GCS::Equal>());
        equations.emplace_back(std::make_unique<GCS::Equal>());
    }
}

void ConstraintCoincident::removePoint(boost::uuids::uuid /*tag*/)
{
    THROW(Base::NotImplementedError);
}

std::vector<GCS::Equation*> ConstraintCoincident::getEquations()
{
    if(references.empty())
    {
        return std::vector<GCS::Equation*>();
    }

    auto& first = references.at(0);
    std::vector<GCS::Equation*> result;
    assert(references.size() == 0 || 2*references.size() == equations.size()+2);
    for(size_t i=0; i < references.size(); ++i)
    {
        auto& point_ref = references.at(i);
        point_ref.refreshLock();

        if(!point_ref.isLocked())
        {
            FC_THROWM(Base::NameError, "Could not resolve name (" << point_ref.pathString() << ").");
        }
        if(i > 0)
        {
            equations.at(2*i - 2)->set(&first.get()->x, &point_ref.get()->x);
            equations.at(2*i - 1)->set(&first.get()->y, &point_ref.get()->y);
            result.emplace_back(equations.at(2*1 - 2).get());
            result.emplace_back(equations.at(2*1 - 1).get());
        }
    }

    return result;
}

bool ConstraintCoincident::updateReferences()
{
    assert(references.size() > 0);
    if(references.size() == 0)
    {
        return false;
    }

    bool any_change = false;
    for(auto& ref: references)
    {
        ref.refreshLock();
        any_change |= ref.hasChanged();
    }
    if(!any_change)
    {
        return false;
    }
    auto& first = references.at(0);
    for(size_t i=1; i < references.size(); ++i)
    {
        auto& point_ref = references.at(i);
        equations.at(2*i - 2)->set(&first.get()->x, &point_ref.get()->x);
        equations.at(2*i - 1)->set(&first.get()->y, &point_ref.get()->y);
    }
    return true;
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
        ref_point reference = ref_point::unserialize(reader);
        result->addPoint(std::move(reference));
    }
    return result;
}

} // namespace NamedSketcher
