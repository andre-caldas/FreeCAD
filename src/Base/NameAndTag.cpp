/***************************************************************************
 *   Copyright (c) 2008 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"

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


namespace Base {

Tag::Tag()
    : tag(boost::uuids::random_generator()()) {}

NameAndTag::NameAndTag()
    : name(to_string(tag)) {}

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
            name.reset();
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
        tag = boost::uuids::string_generator()(name_or_tag);
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
