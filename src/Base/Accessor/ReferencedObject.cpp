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

#include "NameAndTag.h"
#include "ReferencedObject.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace Base::Accessor {

void ReferencedObject::registerTag(const std::shared_ptr<ReferencedObject>& shared_ptr)
{
    std::lock_guard lock(mutex);
    auto tag = shared_ptr->getTag();
    if(!map.count(tag))
    {
        FC_MSG("ReferencedObject already registered: '" << tag << "'.");
        return;
    }
    map.insert({tag, shared_ptr});
}

Tag::tag_type ReferencedObject::registerTag(std::string deprecated)
{
    if(deprecated != "I know it is deprecated")
    {
        FC_THROWM(Base::RuntimeError, "All registeredTags must reference a shared_ptr'd resource.");
    }
    std::lock_guard lock(mutex);
    if(!deprecatedFakeSharePointers.count(this))
    {
        std::shared_ptr<ReferencedObject> shared_ptr(this, [](auto){}); // Fake shared_ptr.
        map.insert({getTag(), shared_ptr});
        deprecatedFakeSharePointers.insert({this, shared_ptr});
        return name_and_tag;
    }
    return deprecatedFakeSharePointers.at(this)->name_and_tag;
}

std::weak_ptr<ReferencedObject>
ReferencedObject::getWeakPtr(std::string_view tag)
{
    Tag t(std::string{tag});
    return getWeakPtr(t);
}

std::weak_ptr<ReferencedObject>
ReferencedObject::getWeakPtr(Tag::tag_type tag)
{
    std::lock_guard lock(mutex);
    if(map.count(tag) == 0)
    {
        return std::weak_ptr<ReferencedObject>();
    }
    return map.at(tag);
}

std::map<Tag::tag_type, std::weak_ptr<ReferencedObject>> ReferencedObject::map;
std::map<const ReferencedObject*, std::shared_ptr<ReferencedObject>> ReferencedObject::deprecatedFakeSharePointers;
std::mutex ReferencedObject::mutex;

} // namespace Base::Accessor
