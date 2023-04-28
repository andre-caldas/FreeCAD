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

#include "NameAndTag.h"


namespace Base::Accessor {

Tag::Tag()
    : tag(boost::uuids::random_generator()()) {}

NameAndTag::NameAndTag() {}

NameAndTag::NameAndTag(std::string name_or_tag)
{
    setText(name_or_tag);
}

void NameAndTag::setText(std::string name_or_tag, bool overwrite_tag)
{
    if(overwrite_tag)
    {
        // Can't we simply check if name_or_tag is a tag?
        // It would be less noisy.
        try {
            tag = boost::uuids::string_generator()(name_or_tag);
            name.clear();
            return;
        } catch (std::runtime_error&) {
        }
    }
    name = std::move(name_or_tag);
}

bool NameAndTag::pointsToMe(NameAndTag& other) const
{
     if (tag == other.tag) return true;
     if ((!name.empty()) && (name == other.name)) return true;
     return false;
}

bool NameAndTag::pointsToMe(std::string_view other) const
{
    if(name == other)
    {
        return true;
    }

    // Can't we simply check if name_or_tag is a tag?
    // It would be less noisy.
    try {
        auto tag = boost::uuids::string_generator()(other.cbegin(), other.cend());
        return pointsToMe(tag);
    } catch (std::runtime_error&) {
    }
    return false;
}

bool NameAndTag::pointsToMe(boost::uuids::uuid other) const
{
     return (tag == other);
}

} // namespace Base
