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

#include <Base/Console.h>
#include <CXX/Objects.hxx>
#include <Base/Exception.h>
#include <Base/Interpreter.h>
#include <App/ExpressionParser.h>

#include "SimpleComponent.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

using namespace App::ObjectPath;

/**
 * @brief Create a string representation of a component.
 * @return A string representing the component.
 */
void SimpleComponent::toString(std::ostream& ss, bool /*toPython*/) const
{
    ss << getName();
}

bool SimpleComponent::isEqual(const Component& other) const
{
    // We could compare just other.name and name.
    // We used dynamic_cast just because it is a bug if other
    // is not of type SimpleComponent.
    return dynamic_cast<const SimpleComponent&>(other).getName() == getName();
}

Py::Object SimpleComponent::get(const Py::Object& pyobj) const {
    if(!pyobj.hasAttr(getName()))
        FC_THROWM(Base::AttributeError, "No attribute named '" << getName() << "'");
    auto res = pyobj.getAttr(getName());
    if(!res.ptr())
        Base::PyException::ThrowException();
    if(PyModule_Check(res.ptr()) && !ExpressionParser::isModuleImported(res.ptr()))
        FC_THROWM(Base::RuntimeError, "Module '" << getName() << "' access denied.");
    return res;
}

void SimpleComponent::set(Py::Object& pyobj, const Py::Object& value) const {
    if(PyObject_SetAttrString(pyobj.ptr(), getName().c_str(), value.ptr()) == -1)
        Base::PyException::ThrowException();
}

void SimpleComponent::del(Py::Object& pyobj) const {
    pyobj.delAttr(getName());
}
