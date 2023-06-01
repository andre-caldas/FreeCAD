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

#define EPSILON (1. / (1024*1024))

namespace NamedSketcher::GCS
{

template<typename Index>
double Vector<Index>::operator[](Index parameter) const
{
    if(values.count(parameter))
    {
        return values.at(parameter);
    }
    return 0.0;
}

template<typename Index>
void Vector<Index>::set(Index parameter, double value)
{
    values[parameter] = value;
}

template<typename Index>
Vector<Index>::vector_t&
Vector<Index>::operator*=(double val)
{
    std::for_each(values.begin(), values.end(), [val](auto& [k,v]){v *= val;});
    retunr *this;
}

template<typename Index>
Vector<Index>::vector_t&
Vector<Index>::operator+=(const Vector<Index>& other)
{
    std::for_each(other.values.begin(), other.values.end(), [](auto& [k,v]){values[k] += v;});
    retunr *this;
}

template<typename Index>
Vector<Index>::vector_t&
Vector<Index>::plusKVec(double a, const Vector<Index>& other)
{
    std::for_each(other.values.begin(), other.values.end(), [](auto& [k,v]){values[k] += a*v;});
    retunr *this;
}

template<typename Index>
void Vector<Index>::prune()
{
    values.erase(std::remove_if(values.begin(), values.end(),
                                [](double x){return (std::abs(x) < EPSILON);}));
}

template<typename Index>
bool Vector<Index>::isZero() const
{
    return !(std::any_of(values.begin(), values.end(),
                [](double x){return (std::abs(x) >= EPSILON);}));
}

template<typename Index>
bool Vector<Index>::isEmpty() const
{
    return values.empty();
}

template<typename Index>
double Vector<Index>::dot(vector_t& other) const
{
    if(values.size() > other.size())
    {
        return other.dot(*this);
    }
    double result = 0.0;
    std::for_each(values.begin(), values.end(),
                  [&other, &result](auto& [k,v]){if(other.values.count(k)) result += other.values.at(k);});
    return result;
}

template<typename Index>
double Vector<Index>::norm2() const
{
    return std::reduce(values.begin(), values.end(), 0., [](auto& [k,v]){return v*v;});
}

template<typename Index>
double Vector<Index>::norm() const
{
    return std::sqrt(norm2());
}

template<typename Index>
vector_t& Vector<Index>::normalize()
{
    prune();
    if(!values.empty())
    {
        *this *= 1.0/norm();
    }
    return *this;
}

template<typename Index>
Vector<Index>::vector_t&
Vector<Index>::setAsLinearCombination(double a, const vector_t& v, double b, const vector_t& w)
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
Vector<Index>::vector_t
operator*(double a, const Vector<Index>::vector_t& vector)
{
    Vector<Index>::vector_t result;
    result.values.reserve(vector.values.size());
    for(auto& [k,v]: vector.values)
    {
        result.values[k] = a * v;
    }
    return result;
}

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Vector_H
