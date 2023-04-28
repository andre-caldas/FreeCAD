// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
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


#ifndef NAMEDSKETCHER_ConstraintCoincident_H
#define NAMEDSKETCHER_ConstraintCoincident_H

#include <vector>
#include <set>
#include <boost/uuid/uuid.hpp>

#include "ConstraintBase.h"

#include "NamedSketcherGlobal.h"

namespace Base {
class Vector3d;
}

namespace App {
class ReferenceTo<Base::Vector3d>;
}

namespace NamedSketcher
{

/** Deals with constraints of type Coincident.
 */
class NamedSketcherExport ConstraintCoincident : public ConstraintBase
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

    using ref_type = App::ReferenceTo<Base::Vector3d>;

public:
    std::vector<ref_type> references;

    template<typename ref,
             typename = std::enable_if_t<std::is_constructible<ref_type, ref>>>
    ConstraintCoincident& addPoint(ref&& reference);
    ConstraintCoincident& removePoint(boost::uuids::uuid tag);

public:
    // Base::Persistence
    unsigned int getMemSize () const override;
    void Save (Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;
};

} // namespace NamedSketcher

#endif // NAMEDSKETCHER_ConstraintCoincident_H
