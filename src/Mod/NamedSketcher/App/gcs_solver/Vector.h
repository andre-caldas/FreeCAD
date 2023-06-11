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


#ifndef NAMEDSKETCHER_GCS_Vector_H
#define NAMEDSKETCHER_GCS_Vector_H

#include <unordered_map>

namespace NamedSketcher::GCS
{

template<typename Index>
class Vector
{
public:
    std::unordered_map<Index, double> values;

    double operator[](Index parameter) const;
    void set(Index parameter, double value);
    bool hasKey(Index parameter) const {return values.count(parameter);}

    Vector<Index>& operator+=(const Vector<Index>& other);
    Vector<Index>& operator*=(double val);
    Vector<Index>& plusKVec(double a, const Vector<Index>& other);
    bool isZero() const;
    bool isEmpty() const;
    double dot(const Vector<Index>& other) const;
    double norm2() const;
    double norm() const;
    Vector<Index>& normalize();
    Vector<Index>& setAsLinearCombination(double a, const Vector<Index>& v, double b, const Vector<Index>& w);

    Vector<Index> operator*(double a) const;
};

template<typename Index>
Vector<Index> operator*(double a, const Vector<Index>& vector) { return vector * a; }

} // namespace NamedSketcher::GCS

#include "Vector.inc"

#endif // NAMEDSKETCHER_GCS_Vector_H
