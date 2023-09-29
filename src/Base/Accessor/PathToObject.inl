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

#include <utility>

#include "ReferencedObject.h"
#include "PathToObject.h"
#include "ReferenceToObject.h"

namespace Base::Accessor {

class ReferencedObject;

template<typename... NameOrUuid,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrUuid, NameAndUuid>...
             >>*>
PathToObject::PathToObject(std::shared_ptr<ReferencedObject> root, NameOrUuid&&... obj_path)
    : rootUuid(root->getUuid())
    , rootWeakPtr(root)
    , objectPath(std::initializer_list<NameAndUuid>{std::forward(obj_path)...})
{
}

template<typename... NameOrUuid,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrUuid, NameAndUuid>...
             >>*>
PathToObject::PathToObject(ReferencedObject* root, NameOrUuid&&... obj_path)
    : rootUuid(root->registerUuid("I know it is deprecated"))
    , rootWeakPtr(root->WeakFromThis())
    , objectPath({NameAndUuid(std::forward<NameOrUuid>(obj_path))...})
{
}

template<typename... NameOrUuid,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrUuid, NameAndUuid>...
             >>*>
PathToObject PathToObject::goFurther(NameOrUuid&& ...furtherPath) const
{
    auto new_path = objectPath;
    new_path.insert(new_path.end(), {NameAndUuid(std::forward<NameOrUuid>(furtherPath))...});
    auto lock = rootWeakPtr.lock();
    if(lock)
    {
        return PathToObject(lock, new_path);
    }
    return PathToObject(rootUuid, new_path);
}

template<typename T>
std::shared_ptr<T> PathToObject::resolveType()
{
    return ReferenceTo<T>(*this).resolve();
}

} // namespace Base::Accessor
