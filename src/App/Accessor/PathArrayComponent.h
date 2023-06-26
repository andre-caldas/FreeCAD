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

#ifndef APP_Accessor_PathArrayComponent_H
#define APP_Accessor_PathArrayComponent_H

#include <memory>

#include <FCGlobal.h>

// The user shall not be required to include the header for the iterator.
#include "PathArrayComponentIterator.h"
#include "PathComponent.h"

namespace App {
//class NumberExpression;
}

namespace App::Accessor
{

// Temporary while I do not know how to use NumberExpression.
using NumberExpression = std::string;
class ArrayReference;

/*!
 * \brief xxx.
 */
class AppExport PathArrayComponent : public PathComponent
{
public:
    using iterator = PathArrayComponentIterator;
    friend class iterator;

    PathArrayComponent(std::string& name,
                       std::unique_ptr<NumberExpression>&& begin,
                       std::unique_ptr<NumberExpression>&& end,
                       std::unique_ptr<NumberExpression>&& step);

    PathComponentIterator begin() const override;
    PathComponentIterator end() const override;

protected:
    using getAccessor = PathComponent::getRoot<ArrayReference>;

private:
    std::unique_ptr<NumberExpression> begin;
    std::unique_ptr<NumberExpression> end;
    std::unique_ptr<NumberExpression> step;
};

} //namespace App::Accessor


#endif // APP_Accessor_PathArrayComponent_H
