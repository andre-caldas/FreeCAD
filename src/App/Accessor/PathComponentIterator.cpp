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

#include <typeinfo>

#include "PathComponentIterator.h"
#include "PathSimpleComponentIterator.h"
#include "PathArrayComponentIterator.h"
#include "PathMapComponentIterator.h"

namespace App::Accessor
{

PathComponentIterator::PathComponentIterator();

PathComponentIterator PathComponentIterator::operator++(int)
{
    auto copy = *this;
    ++*this;
    return copy;
}

bool operator==(const PathComponentIterator& other) const
{
    if(typeid(*this) != typeid(other))
    {
        return false;
    }
    return isEqual(other);
}


void PathSimpleComponentIterator::resetToBegin()
{
}

PathComponentIterator& PathSimpleComponentIterator::operator++()
{
}

protected:
    isEqual(const PathComponentIterator& other) const override;

private:
    bool end = false;
};


class PathArrayComponentIterator : public PathComponentIterator
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


#endif // APP_Accessor_PathComponentIterator_H
