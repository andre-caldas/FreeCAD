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

#include "../PreCompiled.h"

#ifndef _PreComp_
#include <string>
#endif // _PreComp_

#include <Base/Console.h>
#include <Base/Exception.h>

#include "NameAndUuid.h"
#include "ReferencedObject.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace Base::Accessor {

Uuid::uuid_type ReferencedObject::getUuid () const
{
    return nameAndUuid.getUuid();
}

void ReferencedObject::registerUuid(const std::shared_ptr<ReferencedObject>& shared_ptr)
{
    std::lock_guard lock(mutex);
    auto uuid = shared_ptr->getUuid();
    if(!map.count(uuid))
    {
        FC_MSG("ReferencedObject already registered: '" << uuid << "'.");
        return;
    }
    map.insert({uuid, shared_ptr});
}

Uuid::uuid_type ReferencedObject::registerUuid(std::string deprecated)
{
    if(!weak_from_this().expired())
    {
        return getUuid();
    }

    if(deprecated != "I know it is deprecated")
    {
        FC_THROWM(Base::RuntimeError, "All registeredUuids must reference a shared_ptr'd resource.");
    }
    std::lock_guard lock(mutex);
    if(!deprecatedFakeSharePointers.count(this))
    {
        std::shared_ptr<ReferencedObject> shared_ptr(this, [](auto){}); // Fake shared_ptr.
        map.insert({getUuid(), shared_ptr});
        deprecatedFakeSharePointers.insert({this, shared_ptr});
        return getUuid();
    }
    return deprecatedFakeSharePointers.at(this)->getUuid();
}

std::weak_ptr<ReferencedObject>
ReferencedObject::getWeakPtr(std::string_view uuid)
{
    Uuid t(std::string{uuid});
    return getWeakPtr(t);
}

std::weak_ptr<ReferencedObject>
ReferencedObject::getWeakPtr(Uuid::uuid_type uuid)
{
    std::lock_guard lock(mutex);
    if(map.count(uuid) == 0)
    {
        return std::weak_ptr<ReferencedObject>();
    }
    return map.at(uuid);
}

std::map<Uuid::uuid_type, std::weak_ptr<ReferencedObject>> ReferencedObject::map;
std::map<const ReferencedObject*, std::shared_ptr<ReferencedObject>> ReferencedObject::deprecatedFakeSharePointers;
std::mutex ReferencedObject::mutex;

} // namespace Base::Accessor
