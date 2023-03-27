/***************************************************************************
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


#ifndef APP_ObjectPath_ArrayComponent_H
#define APP_ObjectPath_ArrayComponent_H

#include <utility>
#include <sstream>

#include <FCConfig.h>

#include "Component.h"


namespace App::ObjectPath
{

class AppExport ArrayComponent : virtual public Component
{
public:
    void toString(std::ostream& ss, bool toPython=false) const override;
    bool isEqual(const Component& other) const override;

    Py::Object get(const Py::Object& pyobj) const override;
    void set(Py::Object& pyobj, const Py::Object& value) const override;
    void del(Py::Object& pyobj) const override;

    // ArrayComponent has two different roles.
    // One when index >= 0, and one when index < 0.
    // Probably there should be two different classes.
    size_t getIndex(int count) const;

    virtual int getIndex() const = 0;
};

class AppExport ArrayComponentVar : public ArrayComponent
{
public:
    ArrayComponentVar(int index) : index(index) {}

    int getIndex() const override {return index;}

protected:
    // This can be negative or positive.
    // I don't understand the meaning of this variable, yet.
    // I guess that when it is positive, it is an index.
    // When it is negative, it is an index.
    int index;
};

} // namespace App::Path

#endif // APP_ObjectPath_ArrayComponent_H
