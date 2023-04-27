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

#ifndef BASE_Accessor_ReferencedObject_H
#define BASE_Accessor_ReferencedObject_H

#include <memory>
#include <string>

#include <FCGlobal.h>

#include "Types.h"

namespace Base::Accessor
{

/**
 * \brief A ReferencedObject is an object that can be queried
 * to resolve the next step in a path.
 */
class BaseExport ReferencedObject
{
public:
    virtual ~ReferencedObject() = default;
};

template<typename T>
class BaseExport IExportPointer
{
public:
    using export_type = T*;

    // TODO: Use range in C++20.
    virtual export_type resolve(token_iterator& start, token_iterator end);
};

template<typename T>
class BaseExport IExportShared
{
public:
    using export_type = std::shared_ptr<T>;

    // TODO: Use range in C++20.
    virtual export_type resolve(token_iterator& start, token_iterator end);
};

class BaseExport Chainable
        : public ReferencedObject
        , public IExportShared<ReferencedObject>
{
};

} //namespace Base::Accessor

#include "ReferencedObject.inl"

#endif // BASE_Accessor_ReferencedObject_H
