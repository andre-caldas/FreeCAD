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


#ifndef APP_ObjectPath_MapComponent_H
#define APP_ObjectPath_MapComponent_H

#include <sstream>
#include <FCConfig.h>

#include "String.h"
#include "Component.h"


namespace App::ObjectPath
{

class AppExport MapComponent : virtual public Component
{
public:
    void toString(std::ostream& ss, bool toPython=false) const override;
    bool isEqual(const Component& other) const override;
    virtual String getKey() const = 0;

    Py::Object get(const Py::Object& pyobj) const override;
    void set(Py::Object& pyobj, const Py::Object& value) const override;
    void del(Py::Object& pyobj) const override;
};

class AppExport MapComponentVar : public MapComponent
{
public:
    template<typename T>
    MapComponentVar(T&& key) : key(std::forward<T>(key)) {}
    MapComponentVar(const char * key) : key(key) {}

    String getKey() const override {return key;}

protected:
    String key;
};

} // namespace App::Path

#endif // APP_ObjectPath_MapComponent_H
