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

#include <cmath>

#include "ImprovedMatrix.h"

#include "Orthonormalization.h"

namespace NamedSketcher::GCS
{

void Orthonormalization::pushBack(Functional* functional, Vector&& vector)
{
    functionals.push_back(functional);
    duals.addDual(functional, vector);
    auto q = orthogonalComponent(vector);
    q.normalize();
    dualsQ.addDual(functional, std::move(q));
}


void Orthonormalization::popBack(Functional* functional)
{
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


Matrix Orthonormalization::projection(const Matrix& vecs)
{
    return dualsQ * dualsQ.transpose() * vecs;
}

Matrix Orthonormalization::orthogonalComponent(const Matrix& vecs)
{
    return vecs - projection(vecs);
}

Matrix Orthonormalization::normalizedOrthogonalComponent(const Matrix& vecs)
{
    auto result = orthogonalComponent(vecs);
    normalizeduals(result);
    return result;
}

int Orthonormalization::getIndex(Functional* functional) const
{
    return functionals.find(functional);
}

void Orthonormalization::remove(int index)
{
    if(index >= functionals.size())
    {
        assert(false);
        return;
    }

    for(int j=index; j < functionals.size()-1; ++j)
    {
        moveBack(j);
    }
    Functional* f = functionals.back();
    functionals.pop_back();
    duals.remove(f);
    dualsQ.remove(f);
    assert(functionals.size() == duals.size());
    assert(functionals.size() == dualsQ.size());
}

void Orthonormalization::moveForward(int index)
{
    if(index <= 0)
    {
        return;
    }

    moveBack(index-1);
}

void Orthonormalization::moveBack(const int index)
{
    if(index <= 0 || index >= functionals.size())
    {
        return;
    }

    Functional* eq1 = functionals[index];
    Functional* eq2 = functionals[index+1];
    // Write duals[index+1] as a linear combination of dualsQ[index] and dualsQ[index+1].
    double a = duals[eq2].dot(dualsQ[eq1]);
    if(a == 0)
    {
        continue;
    }
    double b = duals[eq2].dot(dualsQ[eq2]);
    double c = std::sqrt(a*a + b*b);
    a /= c;
    b /= c;

    Vector movingQ = dualsQ[eq1];
    dualsQ[eq1].setAsLinearCombination(b, movingQ, -a, dualsQ[eq2]);
    dualsQ[eq2].setAsLinearCombination(a, movingQ, +b, dualsQ[eq2]);

    functionals[index] = eq2;
    functionals[index+1] = eq1;
}


Orthonormalization::RedundantIterator::RedundantIterator(functional_iterator it, functional_iterator end)
    : Iterator(begin, end)
{
    if(!dualsQ[*it].isZero())
    {
        ++*this;
    }
}

Orthonormalization::RedundantIterator&
Orthonormalization::RedundantIterator::operator++(void)
{
    while(it != end)
    {
        ++it;
        if(!dualsQ[*it].isZero())
        {
            break;
        }
    }
    return *this;
}


Orthonormalization::NonRedundantIterator::NonRedundantIterator(functional_iterator it, functional_iterator end)
    : Iterator(begin, end)
{
    if(dualsQ[*it].isZero())
    {
        ++*this;
    }
}

Orthonormalization::RedundantIterator&
Orthonormalization::RedundantIterator::operator++(void)
{
    while(it != end)
    {
        ++it;
        if(dualsQ[*it].isZero())
        {
            break;
        }
    }
    return *this;
}

} // namespace NamedSketcher::GCS
