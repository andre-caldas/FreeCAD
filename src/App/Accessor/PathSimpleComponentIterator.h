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

#ifndef APP_Accessor_PathSimpleComponentIterator_H
#define APP_Accessor_PathSimpleComponentIterator_H

#include <stack>

#include <FCGlobal.h>

#include "PathComponentIterator.h"

namespace App::Accessor
{

class Reference;

class PathSimpleComponentIterator : public PathComponentIterator
{
public:
    void reset(std::weak_ptr<ChainSolver> referenced_object) override;
    void resetToBegin() override;

    PathSimpleComponentIterator<AccessorType>& operator++() {increment(); return *this;}
    PathSimpleComponentIterator<AccessorType> operator++(int);

    AccessorType& operator*() const {return *accessor;}
    AccessorType* operator->() const {return accessor.get();}

protected:
    PathComponentIterator& increment() override;
    Reference* getAccessorPointer() const override {return accessor.get();}
    isEqual(const PathComponentIterator& other) const override;

private:
    std::unique_ptr<AccessorType> accessor;
    bool end = false;
};

} //namespace App::Accessor


#endif // APP_Accessor_PathSimpleComponentIterator_H
