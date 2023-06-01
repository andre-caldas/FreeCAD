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
    using functionals_t = std::vector<Functional*>;

    Orthonormalization() = default;

    using functional_list = std::vector<Functional*>;
    using iterator = functional_list::iterator;
    using const_iterator = functional_list::const_iterator;

    void pushBack(Functional* functional, ParameterVector&& vector);
    void popBack(Functional* functional);
    void remove(Functional* functional);

    void moveForward(Functional* functional);
    void moveBackward(Functional* functional);

    ParameterVector projection(const Matrix& vectors);
    ParameterVector orthogonalComponent(const Matrix& vectors);
    ParameterVector normalizedOrthogonalComponent(const Matrix& vectors);

    int size() const {return duals.size();}

    functionals_t::const_iterator begin() const {return functionals.cbegin();}
    functionals_t::const_iterator end() const {return functionals.cend();}
    functionals_t getRedundants() const;
    functionals_t getNonRedundants() const;

    Functional* operator[](int j) const {return functionals.at(j);}

    int getIndex(Functional* functional) const;
    void remove(int index);
    void moveForward(int index);
    void moveBackward(int index);

private:
    /**
     * @brief Each vector of the matrix is a dual vector
     * representing the gradient for the corresponding @class Functional.
     */
    ParameterMatrix duals;

    /**
     * @brief The gradients can be orthonormalized.
     * The rows of gradientsQ are the orthonormalized gradients.
     */
    ParameterMatrix dualsQ;

    /**
     * @brief Ordering for the functionals.
     * The order is needed by the Gram-Schmidt process.
     */
    std::vector<Functional*> functionals;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Orthonormalization_H
