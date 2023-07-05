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

#include <list>

#include <Base/Vector3D.h>
#include <Base/Accessor/ReferenceToObject.h>

#include "ConstraintBase.h"
#include "ConstraintCoincident.h"
#include "ConstraintEqual.h"
#include "ConstraintHorizontal.h"
#include "ConstraintXDistance.h"

#include "ConstraintFactory.h"

namespace NamedSketcher {

void ConstraintFactory::getAttributes(Base::XMLReader& /*reader*/)
{
//    isDriving = reader.getAttributeAsBoolean("driving", true);
//    isDriven =  reader.getAttributeAsBoolean("driven", false);
}

void ConstraintFactory::setAttributes(ConstraintBase* /*p*/)
{
//    p->isDriving = isDriving;
//    p->isDriven = isDriven;
}

} // namespace NamedSketcher

using namespace NamedSketcher;
template<>
ConstraintFactory::map_type Base::ElementFactory<ConstraintBase>::factoryMap = {
    {
        ConstraintCoincident::xmlTagTypeStatic(),
        [](Base::XMLReader& reader){return ConstraintCoincident::staticRestore(reader);}
    }
    ,{
        ConstraintEqual::xmlTagTypeStatic(),
        [](Base::XMLReader& reader){return ConstraintEqual::staticRestore(reader);}
    }
    ,{
        ConstraintHorizontal::xmlTagTypeStatic(),
        [](Base::XMLReader& reader){return ConstraintHorizontal::staticRestore(reader);}
    }
    ,{
        ConstraintXDistance::xmlTagTypeStatic(),
        [](Base::XMLReader& reader){return ConstraintXDistance::staticRestore(reader);}
    }
};
