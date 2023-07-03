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

#ifndef NAMEDSKETCHER_NamedSketch_H
#define NAMEDSKETCHER_NamedSketch_H

#include "NamedSketcherGlobal.h"

#include <string>
#include <map>
#include <memory>
#include <boost/uuid/uuid.hpp>

#include <Base/Accessor/ReferencedObject.h>
#include <Mod/Part/App/Part2DObject.h>

#include "gcs_solver/System.h"
#include "geometries/PropertyGeometryList.h"
#include "constraints/PropertyConstraintList.h"

namespace Part {
class TopoShape;
}

namespace NamedSketcher
{

class NamedSketcherExport NamedSketch
    : public Part::Part2DObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(NamedSketcher::NamedSketch);

public:
    NamedSketch();

    PropertyGeometryList geometryList;
    PropertyConstraintList constraintList;

    /** @name methods override Feature */
    //@{
    short mustExecute() const override;
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
    PropertyGeometryList::item_reference
    addGeometry(std::unique_ptr<Part::Geometry>&& geo);

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
    PropertyConstraintList::item_reference
    addConstraint(std::shared_ptr<ConstraintBase> constraint);

    /*!
     \brief Deletes indicated constraint.
     \param tag - the geometry to delete
     \retval int - 0 if successful
     */
    void delConstraint(boost::uuids::uuid tag);

    void solve();

    ReferencedObject* resolve_ptr(Base::Accessor::token_iterator& start, const Base::Accessor::token_iterator& end, ReferencedObject*) override;

    void Save(Base::Writer& /*writer*/) const override;
    void Restore(Base::XMLReader& /*reader*/) override;

    Part::TopoShape toShape() const;

    // Old python interface...
    // Document::addObject uses it. :-(
    PyObject* getPyObject();

private:
    GCS::System gcs;
};

using NamedSketchPython = App::FeaturePythonT<NamedSketch>;

} //namespace NamedSketcher

#endif // NAMEDSKETCHER_NamedSketch_H
