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

#include "RangeComponent.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

using namespace App::ObjectPath;


void RangeComponent::toString(std::ostream& ss, bool /* toPython */) const
{
    ss << '[';
//    if(getBegin() != INT_MAX)
        ss << getBegin();
    ss << ':';
//    if(getEnd() != INT_MAX)
        ss << getEnd();
    if(getStep() != 1)
        ss << ':' << getStep();
    ss << ']';
}

bool RangeComponent::isEqual(const Component& other) const
{
    auto& x = dynamic_cast<const RangeComponent&>(other);
    return getBegin() == x.getBegin() && getEnd() == x.getEnd() && getStep()==x.getStep();
}

Py::Object RangeComponent::get(const Py::Object& pyobj) const
{
    Py::Int v1(getBegin());
    Py::Int v2(getEnd());
    Py::Int v3(getStep());
    PyObject *s = PySlice_New(v1.ptr(), v2.ptr(), v3.ptr());
    if(!s)
        throw Py::Exception();
    Py::Object slice(s,true);
    PyObject *res = PyObject_GetItem(pyobj.ptr(),slice.ptr());
    if(!res)
        throw Py::Exception();
    return Py::asObject(res);
}

void RangeComponent::set(Py::Object& pyobj, const Py::Object& value) const
{
    if(pyobj.isMapping())
        Py::Mapping(pyobj).setItem(Py::Int(getBegin()),value);
    else
        Py::Sequence(pyobj).setItem(getBegin(),value);
}

void RangeComponent::del(Py::Object& pyobj) const
{
    if(pyobj.isMapping())
        Py::Mapping(pyobj).delItem(Py::Int(getBegin()));
    else
        PySequence_DelItem(pyobj.ptr(),getBegin());
}
