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

#ifndef BASE_Accessor_NameAndTag_H
#define BASE_Accessor_NameAndTag_H

#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace Base::Accessor
{

class Tag
{
public:
    using tag_type = boost::uuids::uuid;

    Tag();
    Tag(const Tag&) = default;
    Tag(tag_type tag) : tag(tag) {}
    Tag(const std::string& tag);
    Tag(std::string_view tag) : Tag(std::string(tag)){}

    tag_type getTag () const {return tag;}
    void setTag(const std::string& tag);
    std::string toString () const {return to_string(tag);}
    operator std::string() const {return toString();}
    operator tag_type() const {return getTag();}

protected:
    tag_type tag;
};

class NameAndTag : public Tag
{
public:
    NameAndTag();
    NameAndTag(std::string name_or_tag);
    NameAndTag(const char* name_or_tag) : NameAndTag(std::string(name_or_tag)) {}
    NameAndTag(tag_type tag) : Tag(tag) {}
    NameAndTag(Tag tag) : Tag(tag) {}

    std::string getText() const {return name.empty() ? to_string(tag) : name;}
    void setText(std::string name_or_tag, bool overwrite_tag = true);

    operator std::string() const {return getText();}
    bool operator == (std::string_view x) const {return (getText() == x);}
    bool pointsToMe(NameAndTag& other) const;
    bool pointsToMe(std::string_view other) const;
    bool pointsToMe(tag_type other) const;

    bool hasName() const {return !name.empty();}
    const std::string& onlyName() const {return name;}

protected:
    std::string name;
};

} //namespace Base::Accessor

#endif // BASE_Accessor_NameAndTag_H
