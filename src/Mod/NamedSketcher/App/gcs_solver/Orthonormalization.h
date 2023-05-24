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


#ifndef NAMEDSKETCHER_GCS_Orthonormalization_H
#define NAMEDSKETCHER_GCS_Orthonormalization_H

#include <unordered_map>
#include <vector>

#include "Types.h"
#include "../NamedSketcherGlobal.h"

namespace NamedSketcher::GCS
{

class NamedSketcherExport Orthonormalization
{
public:
    using Functional = Equation;

    Orthonormalization() = default;

    using functional_list = std::vector<Functional*>;
    using iterator = functional_list::iterator;
    using const_iterator = functional_list::const_iterator;

    void pushBack(Functional* functional, Vector&& vector);
    void popBack(Functional* functional);
    void remove(Functional* functional);

    void moveForward(Functional* functional);
    void moveBackward(Functional* functional);

    Vector projection(const Matrix& vectors);
    Vector orthogonalComponent(const Matrix& vectors);
    Vector normalizedOrthogonalComponent(const Matrix& vectors);

    int size() const {return duals.size();}

    class Iterator;
    class RedundantIterator;
    class NonRedundantIterator;

    RedundantIterator begin() const {return Iterator(functionals.begin(), functionals.end());}
    RedundantIterator end() const {return Iterator(functionals.end(), functionals.end());}
    RedundantIterator beginRedundants() const {return RedundantIterator(functionals.begin(), functionals.end());}
    RedundantIterator endRedundants() const {return RedundantIterator(functionals.end(), functionals.end());}
    RedundantIterator beginNonRedundants() const {return NonRedundantIterator(functionals.begin(), functionals.end());}
    RedundantIterator endNonRedundants() const {return NonRedundantIterator(functionals.end(), functionals.end());}

    Functional* operator[](int j) const {return functionals.at(j);}

private:
    /**
     * @brief Each vector of the matrix is a dual vector
     * representing the gradient for the corresponding @class Functional.
     */
    Matrix duals;

    /**
     * @brief The gradients can be orthonormalized.
     * The rows of gradientsQ are the orthonormalized gradients.
     */
    Matrix dualsQ;

    /**
     * @brief Ordering for the functionals.
     * The order is needed by the Gram-Schmidt process.
     */
    std::vector<Functional*> functionals;

    int getIndex(Functional* functional) const;
    void remove(int index);
    void moveForward(int index);
    void moveBackward(int index);
};

class NamedSketcherExport Orthonormalization::Iterator
{
public:
    using functional_iterator = std::vector<Functional*>::iterator;

    Iterator(functional_iterator it, functional_iterator end)
        : it(it)
        , end(end)
    {}

    Functional* get() const {return *it;}
    bool operator==(const RedundantIterator& other) const {return it == other.it;}
    bool operator!=(const RedundantIterator& other) const {return it != other.it;}
    Iterator& operator++(void) {++it; return *this;}
    Functional& operator*(void) const {return *get();}
    Functional* operator->(void) const {return get();}

private:
    functional_iterator it;
    functional_iterator end;
};

class NamedSketcherExport Orthonormalization::RedundantIterator
        : public Orthonormalization::Iterator
{
public:
    RedundantIterator(functional_iterator it, functional_iterator end);
    RedundantIterator& operator++(void);
};

class NamedSketcherExport Orthonormalization::NonRedundantIterator
        : public Orthonormalization::Iterator
{
public:
    NonRedundantIterator(functional_iterator it, functional_iterator end);
    NonRedundantIterator& operator++(void);
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Orthonormalization_H
