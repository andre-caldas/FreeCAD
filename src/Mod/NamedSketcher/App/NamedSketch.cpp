// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2008 Jürgen Riegel <juergen.riegel@web.de>               *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
#include <utility>
#include <boost/uuid/uuid_io.hpp>
#endif

#include <iostream>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Accessor/ReferenceToObject.h>

#include <Mod/Part/App/TopoShape.h>
#include <Mod/Part/App/Geometry.h>

#include "geometries/GeometryBase.h"
#include "constraints/ConstraintBase.h"

#include "NamedSketch.h"


FC_LOG_LEVEL_INIT("NamedSketch",true,true)
PROPERTY_SOURCE(NamedSketcher::NamedSketch, Part::Part2DObject)

namespace NamedSketcher
{

NamedSketch::NamedSketch()
{
    ADD_PROPERTY_TYPE_NO_DEFAULT(geometryList, "NamedSketch", (App::PropertyType)(App::Prop_None), "List of geometries used in this sketch");
    ADD_PROPERTY_TYPE_NO_DEFAULT(constraintList, "NamedSketch", (App::PropertyType)(App::Prop_None), "List of constraints used in this sketch");

    // Add standard geometric objects: axis and origin.

    // Learn ObjectIdentifier from here.
    // ExpressionEngine.setValidator(boost::bind(&NamedSketcher::NamedSketch::validateExpression, this, bp::_1, bp::_2));

    // Learn those signals and use for geometry, either.
    // constraintsRemovedConn = Constraints.signalConstraintsRemoved.connect(boost::bind(&NamedSketcher::NamedSketch::constraintsRemoved, this, bp::_1));
    // constraintsRenamedConn = Constraints.signalConstraintsRenamed.connect(boost::bind(&NamedSketcher::NamedSketch::constraintsRenamed, this, bp::_1));

    // Check this later.
    // analyser = new NamedSketchAnalysis(this);
    // Why "new"??? Why a pointer???
}

short NamedSketch::mustExecute() const
{
/*
    if (Geometry.isTouched())
        return 1;
    if (Constraints.isTouched())
        return 1;
    if (ExternalGeometry.isTouched())
        return 1;
*/
    return Part2DObject::mustExecute();
}

App::DocumentObjectExecReturn *NamedSketch::execute()
{
    try {
        App::DocumentObjectExecReturn* rtn = Part2DObject::execute();// to positionBySupport
        if (rtn != App::DocumentObject::StdReturn)
        {
            // error
            return rtn;
        }
    } catch (const Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    bool reference_changed = false;
    for(auto& constraint: constraintList)
    {
        reference_changed |= constraint->updateReferences();
    }
    if(reference_changed)
    {
        gcs.updateGradients();
    }

    solve();

    Shape.setValue(toShape());

    return App::DocumentObject::StdReturn;
}

// TODO: in the future, return an ObjectPath-like "reference".
PropertyGeometryList::item_reference
NamedSketch::addGeometry(std::unique_ptr<Part::Geometry>&& geo)
{
    auto uuid = geometryList.addElement(geometryFactory(std::move(geo)));
    return PropertyGeometryList::item_reference(this, "geometries", uuid);
}

void NamedSketch::delGeometry(boost::uuids::uuid tag)
{
    geometryList.removeElement(tag);
}

PropertyConstraintList::item_reference
NamedSketch::addConstraint(std::shared_ptr<ConstraintBase> constraint)
{
    auto equations = constraint->getEquations();
    for(auto equation: equations)
    {
        gcs.addEquation(equation);
    }
    auto uuid = constraintList.addElement(std::move(constraint));
    return PropertyConstraintList::item_reference(this, "constraints", uuid);
}

void NamedSketch::delConstraint(boost::uuids::uuid tag)
{
    auto constraint = constraintList.getElement(tag);
    auto equations = constraint->getEquations();
    for(auto equation: equations)
    {
        gcs.removeEquation(equation);
    }
    constraintList.removeElement(tag);
}

void NamedSketch::solve() {
    gcs.solve();
}

Base::Accessor::ReferencedObject*
NamedSketch::resolve_ptr(Base::Accessor::token_iterator& start, const Base::Accessor::token_iterator& end, ReferencedObject*)
{
    assert(start != end);
    if(*start == "geometries")
    {
        ++start;
        return &geometryList;
    }
    if(*start == "constraints")
    {
        ++start;
        return &geometryList;
    }
    return nullptr;
}


Part::TopoShape NamedSketch::toShape() const
{
    Part::TopoShape result;
    for (auto& geometry: geometryList)
    {
        if(!geometry->isConstruction)
        {
            TopoDS_Shape sh = geometry->toShape();
            // TODO: too many copying here. :-(
            result.setShape(result.fuse(sh));
        }
    }
    return result;
}

void NamedSketch::Save(Base::Writer &/*writer*/) const
{
    FC_THROWM(Base::NotImplementedError, "NamedSketch::Save not implemented!");
}

void NamedSketch::Restore(Base::XMLReader &/*reader*/)
{
    FC_THROWM(Base::NotImplementedError, "NamedSketch::Restore not implemented!");
}

PyObject* NamedSketch::getPyObject()
{
    /*
     * TODO: I hope that in the near future this can just throw an exception.
     * This is used by Document::addObject.
     * I hope one day Document::addObject can simply take a python object.
     */
    if (!PythonObject.is(Py::_None())) {
        return Py::new_reference_to(PythonObject);
    }

    py::object py_object = py::cast(this);
    PythonObject = py_object.ptr();
    return Py::new_reference_to(PythonObject);
}

void NamedSketch::report() const
{
    std::cout << "Geometries" << std::endl;
    std::cout << "==========" << std::endl;
    for (auto& geometry: geometryList)
    {
        geometry->report();
    }
    std::cout << std::endl;

    std::cout << "Constraints" << std::endl;
    std::cout << "===========" << std::endl;
    for (auto& constraint: constraintList)
    {
        constraint->report();
    }
    std::cout << std::endl;
}

} // namespace NamedSketcher
