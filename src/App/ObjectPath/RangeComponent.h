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


#ifndef APP_ObjectPath_RangeComponent_H
#define APP_ObjectPath_RangeComponent_H

#include <FCConfig.h>

#include "Component.h"


namespace App::ObjectPath
{

class AppExport RangeComponent : virtual public Component
{
public:
    void toString(std::ostream& ss, bool toPython=false) const override;
    bool isEqual(const Component& other) const override;

    Py::Object get(const Py::Object& pyobj) const override;
    void set(Py::Object& pyobj, const Py::Object& value) const override;
    void del(Py::Object& pyobj) const override;

    virtual int getBegin() const = 0;
    virtual int getEnd() const = 0;
    virtual int getStep() const = 0;
};

class AppExport RangeComponentVar : public RangeComponent
{
public:
    RangeComponentVar(int begin, int end, int step = 1)
        : begin(begin)
        , end(end)
        , step(step)
    {}

    int getBegin() const override {return begin;}
    int getEnd() const override {return end;}
    int getStep() const override {return step;}

protected:
    int begin;
    int end;
    int step;
};

} // namespace App::Path

#endif // APP_ObjectPath_RangeRangeComponent_H
