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
#include <boost/format.hpp>

#include <Mod/Sketcher/App/planegcs/GCS.h>

#include "geometries/GeometryFactory.h"

#include "NamedSketch.h"


FC_LOG_LEVEL_INIT("NamedSketch",true,true)
PROPERTY_SOURCE(NamedSketcher::NamedSketch, Part::Part2DObject)

namespace App::NamedSketcher
{

NamedSketch::NamedSketch()
{
    ADD_PROPERTY_TYPE_NO_DEFAULT(geometryList, "NamedSketch", PropertyGeometryList, "List of geometries used in this sketch");
    ADD_PROPERTY_TYPE_NO_DEFAULT(constraintList, "NamedSketch", PropertyConstraintList, "List of constraints used in this sketch");

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
#if 0
//    ??????
#endif
    return App::DocumentObject::StdReturn;
}


bool NamedSketch::isSupportedGeometry(const Part::Geometry& geo) const
{
    if (geo.getTypeId() == Part::GeomPoint::getClassTypeId() ||
        geo->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
        return true;
    }
/*
    if (geo->getTypeId() == Part::GeomPoint::getClassTypeId() ||
        geo->getTypeId() == Part::GeomCircle::getClassTypeId() ||
        geo->getTypeId() == Part::GeomEllipse::getClassTypeId() ||
        geo->getTypeId() == Part::GeomArcOfCircle::getClassTypeId() ||
        geo->getTypeId() == Part::GeomArcOfEllipse::getClassTypeId() ||
        geo->getTypeId() == Part::GeomArcOfHyperbola::getClassTypeId() ||
        geo->getTypeId() == Part::GeomArcOfParabola::getClassTypeId() ||
        geo->getTypeId() == Part::GeomBSplineCurve::getClassTypeId() ||
        geo->getTypeId() == Part::GeomLineSegment::getClassTypeId()) {
        return true;
    }
    if (geo->getTypeId() == Part::GeomTrimmedCurve::getClassTypeId()) {
        Handle(Geom_TrimmedCurve) trim = Handle(Geom_TrimmedCurve)::DownCast(geo->handle());
        Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(trim->BasisCurve());
        Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(trim->BasisCurve());
        if (!circle.IsNull() || !ellipse.IsNull()) {
            return true;
        }
    }
*/
    return false;
}

// TODO: in the future, return an ObjectPath-like "reference".
ReferenceToObjectT<GeometryBase> NamedSketch::addGeometry(std::unique_ptr<Part::Geometry> geo)
{
    auto uuid = geometryList.addValue(GeometryFactory(std::move(geo)));
    return ReferenceToObjectT<GeometryBase>(this, "geometries", uuid);
}

void NamedSketch::delGeometry(boost::uuids::uuid tag)
{
    // TODO: look for constraints.
    geometryList.removeValue(tag);
}

ReferenceToObjectT<ConstraintBase> NamedSketch::addConstraint(std::unique_ptr<ConstraintBase> constraint)
{
    auto uuid = constraintList.addValue(std::move(constraint));
    return ReferenceToObjectT<ConstraintBase>(this, "constraints", uuid);
}

void NamedSketch::delConstraint(boost::uuids::uuid tag)
{
    constraintList.removeValue(tag);
}

void NamedSketch::solve() {
    std::vector<double*> Parameters;    // with memory allocation
    std::vector<double*> DrivenParameters;    // with memory allocation
    std::vector<double*> FixParameters; // with memory allocation
    std::vector<double> MoveParameters, InitParameters;

    GCS::System GCSsys;
    GCSsys.declareUnknowns(Parameters);
    GCSsys.declareDrivenParams(DrivenParameters);
    GCSsys.initSolution(defaultSolverRedundant);
}

} // namespace App::NamedSketcher
