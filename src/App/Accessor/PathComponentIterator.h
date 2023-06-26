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

#ifndef APP_Accessor_PathComponentIterator_H
#define APP_Accessor_PathComponentIterator_H

#include <memory>

#include <FCGlobal.h>

namespace App::Accessor
{

class Reference;
class ChainSolver;

/*!
 * \brief xxx.
 */
class AppExport PathComponentIterator
{
public:
    PathComponentIterator();

    using iterator_category = std::input_iterator_tag;
    using difference_type = void;
    using value_type = Reference;
    using reference = Reference&;
    using pointer = Reference*;

    /**
     * \brief A \class PathComponent cannot be deferenced unless it is associated
     * with some object (\class Reference).
     * Also, for it to be iterated, their \class Expression must resolve,
     * and arrays size might need to be known.
     * For that, one might need an object as well.
     *
     * By calling `reset`, the iterator is rewind back to `begin`,
     * and the \a referenced_object is set.
     * \param referenced_object A \class ChainSolver to be queried.
     */
    virtual void reset(std::weak_ptr<ChainSolver> referenced_object) = 0;
    virtual void resetToBegin() = 0;

    // Those two are not virtual,
    // because we want them to return a specialized version of PathComponentIterator.
    PathComponentIterator& operator++() {increment(); return *this;}
    PathComponentIterator operator++(int);
    virtual bool operator==(const PathComponentIterator& other) const = 0;
    bool operator!=(const PathComponentIterator& other) const {return !(*this == other);}

    // Those two are not virtual,
    // because we want them to return a specialized version of Reference.
    Reference& operator*() const {return *getAccessorPointer();}
    Reference* operator->() const {return getAccessorPointer();}

protected:
    /**
     * @brief Because we do not make operator++ virtual,
     * we use this virtual method as a "default implementation".
     * @return A generic \class PathComponentIterator.
     */
    virtual void increment() = 0;

    /**
     * @brief Because we do not make operator* and operator-> virtual,
     * we use this virtual method as a "default implementation".
     * @return A \class Reference (accessor) to the object represented
     * by this component.
     */
    virtual Reference* getAccessorPointer() const = 0;

    /**
     * @brief Helper to "operator==".
     * The main point is that you can assume *this and \a other
     * are of the same type, and cast \a other.
     * When "operator==" is called, it checks to see if the types are equal,
     * and calls "isEqual()" only when they are.
     * @param other The component being compared.
     */
    virtual isEqual(const PathComponentIterator& other) const = 0;
};

} //namespace App::Accessor


#endif // APP_Accessor_PathComponentIterator_H
