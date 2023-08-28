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

#include <cassert>
#include <cmath>
#include <algorithm>

#include <Base/Exception.h>
#include <Base/Console.h>

#include "Orthonormalization.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace NamedSketcher::GCS
{

void Orthonormalization::pushBack(Functional* functional, ParameterVector&& vector)
{
    functionals.push_back(functional);
    auto q = orthogonalComponent(vector);
    duals.addDual(functional, std::move(vector));
    q.normalize();
    dualsQ.addDual(functional, std::move(q));
}


void Orthonormalization::popBack(Functional* functional)
{
    assert(functionals.back() == functional);
    remove(functionals.size()-1);
}

void Orthonormalization::remove(Functional* functional)
{
    remove(getIndex(functional));
}

void Orthonormalization::moveForward(Functional* functional)
{
    moveForward(getIndex(functional));
}

void Orthonormalization::moveBackward(Functional* functional)
{
    moveBackward(getIndex(functional));
}


ParameterVector Orthonormalization::projection(const ParameterVector& vec) const
{
    return dualsQ.project(vec);
}

ParameterVector Orthonormalization::orthogonalComponent(const ParameterVector& vec) const
{
    return ParameterVector().setAsLinearCombination(1, vec, -1, projection(vec));
}

ParameterVector Orthonormalization::normalizedOrthogonalComponent(const ParameterVector& vecs) const
{
    auto result = orthogonalComponent(vecs);
    double norm = result.norm();
    if(norm)
    {
        result *= 1/norm;
    }
    return result;
}

size_t Orthonormalization::getIndex(Functional* functional) const
{
    auto pos = std::find(functionals.begin(), functionals.end(), functional);
    if(pos == functionals.end())
    {
        FC_THROWM(Base::IndexError, "No index for provided Functional '" << functional << "'.");
    }
    return std::distance(functionals.begin(), pos);
}

void Orthonormalization::remove(size_t index)
{
    if(index >= functionals.size())
    {
        assert(false);
        return;
    }

    for(size_t j=index; j < functionals.size()-1; ++j)
    {
        moveBackward(j);
    }
    Functional* f = functionals.back();
    functionals.pop_back();
    duals.removeDual(f);
    dualsQ.removeDual(f);
    assert(functionals.size() == duals.size());
    assert(functionals.size() == dualsQ.size());
}

void Orthonormalization::moveForward(size_t index)
{
    if(index <= 0)
    {
        return;
    }

    moveBackward(index-1);
}

void Orthonormalization::moveBackward(const size_t index)
{
    if(index <= 0 || index >= functionals.size())
    {
        return;
    }

    Functional* eq1 = functionals[index];
    Functional* eq2 = functionals[index+1];
    // Write duals[index+1] as a linear combination of dualsQ[index] and dualsQ[index+1].
    double a = duals[eq2].dot(dualsQ[eq1]);
    if(a != 0)
    {
        double b = duals[eq2].dot(dualsQ[eq2]);
        double c = std::sqrt(a*a + b*b);
        a /= c;
        b /= c;

        ParameterVector movingQ = dualsQ[eq1];
        dualsQ[eq1].setAsLinearCombination(b, movingQ, -a, dualsQ[eq2]);
        dualsQ[eq2].setAsLinearCombination(a, movingQ, +b, dualsQ[eq2]);
    }

    functionals[index] = eq2;
    functionals[index+1] = eq1;
}

bool Orthonormalization::isRedundant(Functional* functional) const
{
    return dualsQ[functional].isZero();
}

Orthonormalization::functionals_t
Orthonormalization::getRedundants() const
{
    functionals_t result;
    std::copy_if(functionals.cbegin(),
                 functionals.cend(),
                 std::back_insert_iterator(result),
                 [this](Functional* f){return dualsQ[f].isZero();});
    return result;
}

Orthonormalization::functionals_t
Orthonormalization::getNonRedundants() const
{
    functionals_t result;
    std::copy_if(functionals.cbegin(),
                 functionals.cend(),
                 std::back_insert_iterator(result),
                 [this](Functional* f){return !dualsQ[f].isZero();});
    return result;
}

std::vector<Orthonormalization::Functional*> Orthonormalization::reset()
{
    duals.clear();
    dualsQ.clear();
    return std::move(functionals);
}

} // namespace NamedSketcher::GCS
