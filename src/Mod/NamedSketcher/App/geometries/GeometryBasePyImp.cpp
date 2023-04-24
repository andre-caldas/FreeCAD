/***************************************************************************
 *   Copyright (c) 2010 Jürgen Riegel <juergen.riegel@web.de>              *
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

#include <string>
#include <sstream>
#include <boost/uuid/uuid_io.hpp>

#include <Base/QuantityPy.h>

#include "GeometryBasePy.h"
#include "GeometryBasePy.cpp"


using namespace NamedSketcher;

// constructor method
int GeometryBasePy::PyInit(PyObject* /*args*/, PyObject* /*kwd*/)
{
    return 0;
}

// returns a string which represents the object e.g. when printed in python
std::string GeometryBasePy::representation() const
{
    std::stringstream result;
    result << "<Geometry "
           << getGeometryPointPtr()->geometry
           << ">";
    return result.str();
}

Py::String GeometryBasePy::getTag() const
{
    const std::string result = boost::uuids::to_string(getConstraintPtr()->getTag());
    return Py::String(result.c_str());
}

PyObject *GeometryBasePy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int GeometryBasePy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}
