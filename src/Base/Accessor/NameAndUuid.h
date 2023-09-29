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

#ifndef BASE_Accessor_NameAndUuid_H
#define BASE_Accessor_NameAndUuid_H

#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace Base::Accessor
{

class Uuid
{
public:
    using uuid_type = boost::uuids::uuid;

    Uuid();
    Uuid(const Uuid&) = default;
    Uuid(uuid_type uuid) : uuid(uuid) {}
    Uuid(const std::string& uuid);
    Uuid(std::string_view uuid) : Uuid(std::string(uuid)){}
    virtual ~Uuid() = default;

    uuid_type getUuid () const {return uuid;}
    void setUuid(const std::string& uuid);
    std::string toString () const {return to_string(uuid);}
    operator std::string() const {return toString();}
    operator uuid_type() const {return getUuid();}

    static bool isUuid(const std::string& name_or_uuid);

protected:
    uuid_type uuid;
};

class NameAndUuid : public Uuid
{
public:
    NameAndUuid();
    NameAndUuid(std::string name_or_uuid);
    NameAndUuid(const char* name_or_uuid) : NameAndUuid(std::string(name_or_uuid)) {}
    NameAndUuid(uuid_type uuid) : Uuid(uuid) {}
    NameAndUuid(Uuid uuid) : Uuid(uuid) {}

    std::string getText() const {return name.empty() ? to_string(uuid) : name;}
    void setText(std::string name_or_uuid, bool overwrite_uuid = true);

    operator std::string() const {return getText();}
    bool operator == (std::string_view x) const {return (getText() == x);}
    bool pointsToMe(NameAndUuid& other) const;
    bool pointsToMe(std::string_view other) const;
    bool pointsToMe(uuid_type other) const;

    bool hasName() const {return !name.empty();}
    const std::string& onlyName() const {return name;}

protected:
    std::string name;
};

} //namespace Base::Accessor

#endif // BASE_Accessor_NameAndUuid_H
