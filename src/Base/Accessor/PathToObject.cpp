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

#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Exception.h>

#include "Exception.h"
#include "ReferencedObject.h"

#include "PathToObject.h"

namespace Base::Accessor {

PathToObject::PathToObject(std::shared_ptr<ReferencedObject> root, const token_list& path)
    : rootUuid(root->getUuid())
    , rootWeakPtr(root)
    , objectPath(path)
{
}

std::string PathToObject::pathString() const
{
    return pathString(objectPath.cbegin(), objectPath.cend());
}

std::string PathToObject::pathString(token_iterator start, const token_iterator end)
{
    std::string result;
    for(auto pos = start; pos != end; ++pos)
    {
        result += pos->getText();
    }
    return result;
}

PathToObject& PathToObject::operator+= (PathToObject extra_path)
{
    token_list&& tokens = std::move(extra_path.objectPath);
    objectPath.reserve(objectPath.size() + tokens.size() + 1);
    objectPath.push_back(NameAndUuid(extra_path.rootUuid));
    objectPath.insert(objectPath.end(), tokens.begin(), tokens.end());
    return *this;
}

PathToObject::lock_type PathToObject::getLock() const
{
    // TODO: Implement different "cache expire" policies.
    lock_type lock;

    // First we try the rootWeakPtr.
    lock.last_object = rootWeakPtr.lock();
    // If it is not valid, we try to find the uuid in the global dictionary.
    if(!lock.last_object)
    {
        lock.last_object = ReferencedObject::getWeakPtr(rootUuid).lock();
    }

    if(!lock.last_object)
    {
        FC_THROWM(ExceptionCannotResolve, "Root object (" << rootUuid.toString() << ") is not available. Path: '" << pathString() << "'.");
    }

    lock.remaining_tokens_start = objectPath.cbegin();
    lock.remaining_tokens_end = objectPath.cend();
    while(lock.remaining_tokens_start != lock.remaining_tokens_end)
    {
        auto ref_obj = dynamic_cast<IExport<ReferencedObject>*>(lock.last_object.get());
        if(!ref_obj)
        {
            // Current object is not chainable.
            return lock;
        }

        auto previous_it = lock.remaining_tokens_start;
        lock.last_object = ref_obj->resolve(lock.last_object, lock.remaining_tokens_start, objectPath.end());

        if(lock.remaining_tokens_start == previous_it)
        {
            FC_THROWM(Base::RuntimeError, "Object's path resolution is not consuming tokens. Path: '" << pathString() << "'. This is a BUG!");
        }
    }
    return lock;
}

void PathToObject::serialize(Base::Writer& writer) const
{
    writer.Stream() << writer.ind() << "<PathToObject>" << std::endl;
    writer.incInd();
    writer.Stream() << writer.ind() << "<RootUuid>";
    writer.Stream() << rootUuid.toString() << "</RootUuid>" << std::endl;
    for(auto token: objectPath)
    {
        // TODO: escape token.getText().
        writer.Stream() << writer.ind() << "<NameOrUuid>";
        writer.Stream() << token.getText() << "</NameOrUuid>" << std::endl;
    }
    writer.decInd();
    writer.Stream() << writer.ind() << "</PathToObject>" << std::endl;
}

PathToObject
PathToObject::unserialize(Base::XMLReader& reader)
{
    reader.readElement("PathToObject");
    reader.readElement("RootUuid");
    auto root = ReferencedObject::getWeakPtr(reader.getCharacters()).lock();
    if(!root)
    {
        FC_THROWM(ReferenceError, "Root element does not exist when unserializing RferenceTo: '" << reader.getCharacters() << "'");
    }
    PathToObject result{root};
    while(!reader.testEndElement("PathToObject"))
    {
        reader.readElement("NameOrUuid");
        result.objectPath.push_back(NameAndUuid(reader.getCharacters()));
    }
    return result;
}

} // namespace Base::Accessor
