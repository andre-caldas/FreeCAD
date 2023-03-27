/***************************************************************************
 *   Copyright (c) 2015 Eivind Kvedalen <eivind@kvedalen.name>             *
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

//#include "PreCompiled.h"

//#ifndef _PreComp_
# include <cassert>
//#endif

#include <sstream>

#include <Base/Console.h>
#include <CXX/Objects.hxx>
#include <Base/Exception.h>
#include <Base/Interpreter.h>
#include <App/ExpressionParser.h>

#include "MapComponent.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

using namespace App::ObjectPath;


void MapComponent::toString(std::ostream& ss, bool toPython) const
{
    ss << "[" << getKey().toString(toPython) << "]";
}

bool MapComponent::isEqual(const Component& other) const
{
    // We could compare just other.key and key.
    // We used static_cast just because it is a bug if other
    // is not of type MapComponent.
    return dynamic_cast<const MapComponent&>(other).getKey() == getKey();
}

Py::Object MapComponent::get(const Py::Object& pyobj) const {
    auto _key = getKey().getString();
    Py::Object res = Py::Mapping(pyobj).getItem(_key);
    if(!res.ptr())
        throw Py::Exception();
    return res;
}

void MapComponent::set(Py::Object& pyobj, const Py::Object& value) const {
    Py::Mapping(pyobj).setItem(getKey().getString(),value);
}

void MapComponent::del(Py::Object& pyobj) const {
    Py::Mapping(pyobj).delItem(getKey().getString());
}
