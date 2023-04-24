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

#ifndef NAMEDSKETCHER_NamedSketch_H
#define NAMEDSKETCHER_NamedSketch_H

#include "NamedSketcherGlobal.h"

#include <string>
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>

#include <App/PropertyContainer.h>
#include <Mod/Part/App/Part2DObject.h>

#include <Mod/Sketcher/App/planegcs/GCS.h>

#include "geometries/PropertyGeometryList.h"
#include "constraints/PropertyConstraintList.h"

namespace App::NamedSketcher
{

class NamedSketcherExport NamedSketch
        : public App::PropertyContainer
        , public Part::Part2DObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(NamedSketcher::NamedSketch);

public:
    NamedSketch();

    PropertyGeometryList geometryList;
    PropertyConstraintList constraintList;

    /** @name methods override Feature */
    //@{
    short mustExecute() const override;
    /// recalculate the Feature (if no recompute is needed see also solve() and solverNeedsUpdate boolean)
    App::DocumentObjectExecReturn *execute() override;

    /// returns the type name of the ViewProvider
    const char* getViewProviderName() const override {
        return "NamedSketcherGui::ViewProviderNamedSketch";
    }
    //@}

    /*!
     \brief Adds a geometry to a sketch
     \param geo - std::unique_ptr to geometry to add
     \param construction - true for construction lines
     \retval reference to the newly created GeometryBase.
     */
    ReferenceToObjectT<GeometryBase> addGeometry(std::unique_ptr<Part::Geometry> geo);

    /*!
     \brief Deletes indicated geometry (by tag).
     \param tag - the geometry to delete
     */
    void delGeometry(boost::uuids::uuid tag);

    /*!
     \brief Adds a constraint to a sketch
     \param constraint - std::unique_ptr to constraint to add
     \retval tag of the added constraint.
     */
    ReferenceToObjectT<ConstraintBase> addConstraint(std::unique_ptr<ConstraintBase> constraint);

    /*!
     \brief Deletes indicated constraint.
     \param tag - the geometry to delete
     \retval int - 0 if successful
     */
    void delConstraint(boost::uuids::uuid tag);

    void solve();

    void Save(Base::Writer &/*writer*/) const override;
    void Restore(Base::XMLReader &/*reader*/) override;

private:
    GCS::System gcsSystem;
};

using NamedSketchPython = App::FeaturePythonT<NamedSketch>;

} //namespace NamedSketcher

#endif // NAMEDSKETCHER_NamedSketch_H
