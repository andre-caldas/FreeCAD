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

#ifndef APP_Accessor_AccessorProperty_H
#define APP_Accessor_AccessorProperty_H

#include <FCGlobal.h>

#include "Accessor.h"

namespace App {
class Property;
class PropertyContainer;
}

namespace App::Accessor
{

template<typename GuardType = App::Property>
class AppExport AccessorProperty : AccessorVar<App::Property, GuardType>
{
public:
    using AccessorVar<App::PropertyContainer, GuardType>::AccessorVar;

protected:
    std::any _getAny(std::monostate) const override;
    std::unique_ptr<Accessor> _getAccessor(std::monostate) const;
};


template<typename GuardType = App::Property>
class AppExport AccessorPropertyContainer : AccessorVar<App::PropertyContainer, GuardType>
{
public:
    using AccessorVar<App::PropertyContainer, GuardType>::AccessorVar;

protected:
    std::any _getAny(std::string_view key) const override;
    std::unique_ptr<Accessor> _getAccessor(std::string_view key) const;
};

} //namespace App::Accessor

#endif // APP_Accessor_AccessorProperty_H
