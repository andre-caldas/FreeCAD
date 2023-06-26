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

#ifndef APP_Accessor_PathMapComponentIterator_H
#define APP_Accessor_PathMapComponentIterator_H

#include <FCGlobal.h>

#include "PathComponentIterator.h"

namespace App::Accessor
{

/*!
 * \brief xxx.
 */
class PathMapComponentIterator : public PathComponentIterator
{
public:
    void resetToBegin() override;

    PathComponentIterator& operator++() override;
    Reference* operator->() const override;

protected:
    isEqual(const PathComponentIterator& other) const override;

private:
    bool end = false;
};

} //namespace App::Accessor


#endif // APP_Accessor_PathMapComponentIterator_H
