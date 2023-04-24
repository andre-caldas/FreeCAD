/***************************************************************************
 *   Copyright (c) 2010 Jürgen Riegel <juergen.riegel@web.de>              *
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

#include "GeometryBase.h"
#include "PropertyGeometryList.h"

using namespace App;
TYPESYSTEM_SOURCE(NamedSketcher::PropertyGeometryList, Base::Property)

namespace App::NamedSketcher
{

Property* PropertyGeometryList::Copy() const
{
    FC_THROWM(Base::NotImplementedError, "Are you sure you want to copy this?");
}

void PropertyGeometryList::Paste(const Property &from)
{
    FC_THROWM(Base::NotImplementedError, "Are you sure you want to paste this?");
}

PyObject* PropertyGeometryList::getPyObject()
{
    // If you really needs this implemented,
    // take a look at Sketcher::PropertyConstraintList.
    FC_THROWM(Base::NotImplementedError, "Who needs this!?");
    return nullptr;
}

void PropertyGeometryList::setPyObject(PyObject*)
{
    // If you really needs this implemented,
    // take a look at Sketcher::PropertyConstraintList.
    FC_THROWM(Base::NotImplementedError, "Who needs this!?");
}

} // namespace App::NamedSketcher
