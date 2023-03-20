/***************************************************************************
 *   Copyright (c) 2015 Eivind Kvedalen <eivind@kvedalen.name>             *
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

//#include "PreCompiled.h"

//#ifndef _PreComp_
# include <cassert>
//#endif

#include <string>
#include <map>

#include <Base/Console.h>

#include "DocumentMapper.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

namespace App::ObjectPath {

DocumentMapper::DocumentMapper(const map_type &map)
{
    assert(!_DocumentMap);
    _DocumentMap = &map;
}

DocumentMapper::~DocumentMapper()
{
    _DocumentMap = nullptr;
}

bool DocumentMapper::hasMap()
{
    return !_DocumentMap;
}

DocumentMapper::map_type::const_iterator DocumentMapper::find(const std::string& name)
{
    assert(_DocumentMap);
    return _DocumentMap->find(name);
}

DocumentMapper::map_type::const_iterator DocumentMapper::end()
{
    assert(_DocumentMap);
    return _DocumentMap->end();
}

const std::map<std::string,std::string> *DocumentMapper::_DocumentMap;

} // namespace App::ObjectPath
