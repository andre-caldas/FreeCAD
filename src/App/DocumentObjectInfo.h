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

#ifndef APP_DocumentObjectInfo_H
#define APP_DocumentObjectInfo_H

#include <memory>
#include <string>

#include <Base/Threads/ThreadSafeMultiIndex.h>

namespace App
{

class DocumentObject;

struct DocumentObjectInfo
{
    using object_t = std::shared_ptr<DocumentObject>;
    DocumentObjectInfo(object_t obj,
                       std::string name)
        : object(std::move(obj))
        , id(object->getID())
        , name(std::move(name))
    {}

    object_t object;
    long id;
    std::string name;
};

template<typename Record, auto ...LocalPointers>
using ThreadSafeMultiIndex = Base::Threads::ThreadSafeMultiIndex<Record, LocalPointers...>;

typedef ThreadSafeMultiIndex<DocumentObjectInfo,
                             &DocumentObjectInfo::object,
                             &DocumentObjectInfo::id,
                             &DocumentObjectInfo::name> object_info_list_t;

} //namespace App

#endif // APP_DocumentObjectInfo_H
