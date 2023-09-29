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

#ifdef _PreComp_
#include <string>
#include <sstream>
#endif // _PreComp_
#include <utility>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Base/Exception.h>

#include "NameAndUuid.h"


namespace Base::Accessor {

Uuid::Uuid()
    : uuid(boost::uuids::random_generator()())
{}

Uuid::Uuid(const std::string& uuid)
    : uuid(boost::uuids::string_generator()(uuid))
{}

void Uuid::setUuid(const std::string& t)
{
    uuid = boost::uuids::string_generator()(t);
}

bool Uuid::isUuid(const std::string& name_or_uuid)
{
    // Can't we simply check if name_or_uuid is a uuid?
    // It would be less noisy.
    try {
        boost::uuids::string_generator()(name_or_uuid);
        return true;
    } catch (std::runtime_error&) {
    }
    return false;
}

NameAndUuid::NameAndUuid() {}

NameAndUuid::NameAndUuid(std::string name_or_uuid)
{
    setText(name_or_uuid);
}

void NameAndUuid::setText(std::string name_or_uuid, bool overwrite_uuid)
{
    if(overwrite_uuid)
    {
        // Can't we simply check if name_or_uuid is a uuid?
        // It would be less noisy.
        try {
            uuid = boost::uuids::string_generator()(name_or_uuid);
            name.clear();
            return;
        } catch (std::runtime_error&) {
        }
    }
    name = std::move(name_or_uuid);
}

bool NameAndUuid::pointsToMe(NameAndUuid& other) const
{
     if (uuid == other.uuid) return true;
     if ((!name.empty()) && (name == other.name)) return true;
     return false;
}

bool NameAndUuid::pointsToMe(std::string_view other) const
{
    if(name == other)
    {
        return true;
    }

    // Can't we simply check if name_or_uuid is a uuid?
    // It would be less noisy.
    try {
        auto uuid = boost::uuids::string_generator()(other.cbegin(), other.cend());
        return pointsToMe(uuid);
    } catch (std::runtime_error&) {
    }
    return false;
}

bool NameAndUuid::pointsToMe(Uuid::uuid_type other) const
{
     return (uuid == other);
}

} // namespace Base
