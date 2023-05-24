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


#ifndef NAMEDSKETCHER_GCS_ParameterVector_H
#define NAMEDSKETCHER_GCS_ParameterVector_H

#include <unordered_map>

namespace NamedSketcher::GCS
{

template<typename ParameterType>
class ParameterVector
{
    using vector_t = ParameterVector<ParameterType>;

public:
    std::unordered_map<ParameterType, double> values;

    double operator[](ParameterType parameter) const;
    void set(ParameterType parameter, double value);

    vector_t& operator+=(double val);
    vector_t& operator*=(const ParameterVector<ParameterType>& other);
    vector_t& plusKVec(const ParameterVector<ParameterType>& other);
    void prune();
    bool isZero() const;
    double dot(vector_t& other) const;
    double norm2() const;
    double norm() const;
    vector_t& normalize();
    vector_t& setAsLinearCombination(double a, const vector_t& v, double b, const vector_t& w);

    friend vector_t operator*(double a, const vector_t& vector);
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterVector_H
