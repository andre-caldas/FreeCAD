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

#ifndef APP_Accessor_SimpleAccessor_H
#define APP_Accessor_SimpleAccessor_H

#include <FCGlobal.h>

#include "Accessor.h"

namespace App::Accessor
{

template<typename ReferencedType>
class AppExport SimpleAccessorT : public SimpleAccessor
{
public:
    virtual ReferencedType& get() const = 0;
    virtual set(const ReferencedType& value) const = 0;
    virtual set(ReferencedType&& value) const {set(value);}
};


/*!
 * \brief xxx.
 */
template<typename T>
class AppExport SimpleAccessorVar : public SimpleAccessorT<T>
{
public:
    template<typename Guard>
    SimpleAccessorVar(std::weak_ptr<Guard>&& guard, T& variable) : variable(variable) {}

    T& get() const override {return variable;}
    set(const T& value) const override {variable = value;}
    set(T&& value) const override {variable = value;}
};

} //namespace App::Accessor

#endif // APP_Accessor_SimpleAccessor_H
