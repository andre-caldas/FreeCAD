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


#ifndef APP_ObjectPath_Component_H
#define APP_ObjectPath_Component_H

#include <utility>
#include <sstream>

#include <FCGlobal.h>

#include "String.h"

namespace Py {
class Object;
}
namespace App {
class Property;
}

namespace App::ObjectPath
{

/**
 * @brief A component is a part of a Path object, and is used to either
 * name a property or a field within a property. A component can be either
 * a single entry, and array, or a map to other sub-fields.
 */

class AppExport Component {
public:
    virtual ~Component() = default;

    virtual void toString(std::ostream &ss, bool toPython=false) const = 0;

    bool operator==(const Component & other) const;
    // You can assume typeid(*this) == typeid(other).
    // That is, just use static_cast on other.
    virtual bool isEqual(const Component & other) const = 0;

    virtual Py::Object get(const Py::Object& pyobj) const = 0;
    virtual void set(Py::Object& pyobj, const Py::Object& value) const = 0;
    virtual void del(Py::Object &pyobj) const = 0;

    virtual std::string getName() const {return _name_.getString();}
    virtual void setName(const String& name) {_name_ = name;}
    virtual void setName(String&& name) {_name_ = std::move(name);}
    // This is a temporary hack.
    // Do not create isArray, isRange, isXXX!!!
    // isSimple shall be removed or substituted by something
    // with more meaningful semantics:
    // - for example, this component could represent a "terminal component". Does it?
    virtual bool isSimple() const {return false;}

private:
    bool operator<(const Component & other) const;
    // Do not access _name_ directly!
    // Use getName() and setName().
    String _name_;
};

} // namespace App::Path

#endif // APP_ObjectPath_Component_H
