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

#include "ArrayComponent.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

using namespace App::ObjectPath;


void ArrayComponent::toString(std::ostream& ss, bool /*toPython*/) const
{
    ss << "[" << getIndex() << "]";
}

bool ArrayComponent::isEqual(const Component& other) const
{
    return dynamic_cast<const ArrayComponent&>(other).getIndex() == getIndex();
}

Py::Object ArrayComponent::get(const Py::Object& pyobj) const
{
    auto _index = getIndex();
    Py::Object res;
    if(pyobj.isMapping())
        res = Py::Mapping(pyobj).getItem(Py::Int(_index));
    else
        res = Py::Sequence(pyobj).getItem(_index);

    if(!res.ptr())
        Base::PyException::ThrowException();
    if(PyModule_Check(res.ptr()) && !ExpressionParser::isModuleImported(res.ptr()))
        FC_THROWM(Base::RuntimeError, "Module '" /*<< getName() <<*/ "' access denied.");
    return res;
}

void ArrayComponent::set(Py::Object& pyobj, const Py::Object& value) const
{
    if(pyobj.isMapping())
        Py::Mapping(pyobj).setItem(Py::Int(getIndex()),value);
    else
        Py::Sequence(pyobj).setItem(getIndex(),value);
}

void ArrayComponent::del(Py::Object& pyobj) const
{
    if(pyobj.isMapping())
        Py::Mapping(pyobj).delItem(Py::Int(getIndex()));
    else
        PySequence_DelItem(pyobj.ptr(),getIndex());
}

size_t ArrayComponent::getIndex(int count) const {
    if(getIndex() >= 0) {
        if(getIndex() < count)
            return getIndex();
    } else {
        int idx = getIndex() + count;
        if(idx >= 0)
            return idx;
    }
    FC_THROWM(Base::IndexError, "Array range out of bound: " << getIndex() << ", " << count);
}
