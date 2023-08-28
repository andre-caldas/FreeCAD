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

#include <algorithm>
#include <numeric>
#include <cmath>

#include "Vector.h"

#define EPSILON 0x1p-28

namespace NamedSketcher::GCS
{

template<typename Index>
double Vector<Index>::operator[](const Index parameter) const
{
    if(values.count(parameter))
    {
        return values.at(parameter);
    }
    return 0.0;
}

template<typename Index>
void Vector<Index>::set(const Index parameter, double value)
{
    values[parameter] = value;
}

template<typename Index>
Vector<Index>&
Vector<Index>::operator*=(double val)
{
    std::for_each(values.begin(), values.end(), [val](auto& pair){pair.second *= val;});
    return *this;
}

template<typename Index>
Vector<Index>&
Vector<Index>::operator+=(const Vector<Index>& other)
{
    std::for_each(other.values.begin(), other.values.end(), [this](auto& pair){values[pair.first] += pair.second;});
    return *this;
}

template<typename Index>
Vector<Index>&
Vector<Index>::plusKVec(double a, const Vector<Index>& other)
{
    std::for_each(other.values.begin(), other.values.end(), [this, a](auto& pair){values[pair.first] += a*pair.second;});
    return *this;
}

template<typename Index>
bool Vector<Index>::isZero() const
{
    return !(std::any_of(values.cbegin(), values.cend(),
                [](const auto& pair){return (std::abs(pair.second) >= EPSILON);}));
}

template<typename Index>
bool Vector<Index>::isEmpty() const
{
    return values.empty();
}

template<typename Index>
double Vector<Index>::dot(const Vector<Index>& other) const
{
    if(values.size() > other.values.size())
    {
        return other.dot(*this);
    }
    return std::accumulate(values.begin(), values.end(), 0.0,
                           [&other](double acc, auto& pair){if(other.values.count(pair.first)) return acc + other.values.at(pair.first); return acc;});
}

template<typename Index>
double Vector<Index>::norm2() const
{
    return std::accumulate(values.begin(), values.end(), 0.0,
                           [](double acc, auto& pair){return acc + (pair.second)*(pair.second);});
}

template<typename Index>
double Vector<Index>::norm() const
{
    return std::sqrt(norm2());
}

template<typename Index>
Vector<Index>& Vector<Index>::normalize()
{
    if(!isZero())
    {
        *this *= 1.0/norm();
    }
    return *this;
}

template<typename Index>
Vector<Index>&
Vector<Index>::setAsLinearCombination(double a, const Vector<Index>& v, double b, const Vector<Index>& w)
{
    values.clear();
    values.reserve(v.values.size() + w.values.size());
    for(const auto& [k,val] : v.values)
    {
        values.try_emplace(k, a*val + b*w[k]);
    }
    for(const auto& [k,val] : w.values)
    {
        values.try_emplace(k, b*val);
    }
    return *this;
}

template<typename Index>
Vector<Index> Vector<Index>::operator*(double a) const
{
    Vector<Index> result;
    result.values.reserve(values.size());
    for(auto& [k,v]: values)
    {
        result.values[k] = a * v;
    }
    return result;
}

} // namespace NamedSketcher::GCS
