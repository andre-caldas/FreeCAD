/***************************************************************************
 *   Copyright (c) 2008 Jürgen Riegel <juergen.riegel@web.de>              *
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

#include "PreCompiled.h"

#ifndef _PreComp_
#include <utility>
#include <boost/uuid/uuid_io.hpp>
#endif

#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Accessor/ReferenceToObject.h>

#include <Mod/Part/App/TopoShape.h>
#include <Mod/Part/App/Geometry.h>

#include "geometries/GeometryFactory.h"

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
        reference_changed |= constraint.updateReferences();
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
NamedSketch::addGeometry(std::unique_ptr<Part::Geometry> geo)
{
    auto uuid = geometryList.addElement(GeometryFactory(std::move(geo)));
    return PropertyGeometryList::item_reference(this, "geometries", uuid);
}

void NamedSketch::delGeometry(boost::uuids::uuid tag)
{
    geometryList.removeElement(tag);
}

PropertyConstraintList::item_reference
NamedSketch::addConstraint(std::unique_ptr<ConstraintBase> constraint)
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
    auto& constraint = constraintList.getElementReference(tag);
    auto equations = constraint.getEquations();
    for(auto equation: equations)
    {
        gcs.removeEquation(equation);
    }
    constraintList.removeElement(tag);
}

void NamedSketch::solve() {
    gcs.solve();
}

Base::Accessor::ReferencedObject
NamedSketch::resolve(Base::Accessor::token_iterator& start, const Base::Accessor::token_iterator& end)
{
    if(*start == "geometries")
    {
        ++start;
        Not shared ptr!!!;
        return geometryList;
    }
    if(*start == "constraints")
    {
        ++start;
        Not shared ptr!!!;
        return geometryList;
    }
    return std::shared_ptr<ReferencedObject>();
}


Part::TopoShape NamedSketch::toShape() const
{
    Part::TopoShape result;
    for (auto& geometry: geometryList)
    {
        if(!geometry.isConstruction)
        {
            TopoDS_Shape sh = geometry.toShape();
            // TODO: too many copying here. :-(
            result.setShape(result.fuse(sh));
        }
    }
    return result;
}

} // namespace NamedSketcher
