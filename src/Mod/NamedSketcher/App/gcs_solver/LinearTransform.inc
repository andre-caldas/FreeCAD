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

#include "Vector.h"

namespace NamedSketcher::GCS
{

template<typename OutType, typename InType>
LinearTransform<OutType, InType>::LinearTransform(LinearTransform<OutType,InType>&& from)
{
    dualKeys = std::move(from.dualKeys);
    duals = std::move(from.duals);
}

template<typename OutType, typename InType>
void LinearTransform<OutType, InType>::addDual(OutType key, dual_t&& dual)
{
    duals.emplace(key, dual);
    dualKeys.emplace_back(key);
}

template<typename OutType, typename InType>
void LinearTransform<OutType, InType>::removeDual(OutType key)
{
    duals.erase(key);
    dualKeys.erase(std::remove(dualKeys.begin(), dualKeys.end(), key));
}


template<typename OutType, typename InType>
Vector<OutType>
LinearTransform<OutType, InType>::apply(const dual_t& vector) const
{
    Vector<OutType> result;
    for(auto& [k,f]: duals)
    {
        result.values.try_emplace(k, f.dot(vector));
    }
    return result;
}

template<typename OutType, typename InType>
Vector<InType>
LinearTransform<OutType, InType>::project(const dual_t& vector) const
{
    Vector<InType> result;
    for(auto& [k,f]: duals)
    {
        result.plusKVec(f.dot(vector), f);
    }
    return result;
}

template<typename OutType, typename InType>
void LinearTransform<OutType, InType>::replaceInputParameter(InType from, InType to)
{
    for(auto& vector: duals)
    {
        vector.replaceKey(from, to);
    }
}

} // namespace NamedSketcher::GCS
