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

#include "ParameterVector.h"
#include "LinearTransform.h"

namespace NamedSketcher::GCS
{

template<typename OutType, typename InType>
void LinearTransform<OutType, InType>::addDual(OutType key, dual_t&& dual)
{
    duals.emplace(key, dual);
}

template<typename OutType, typename InType>
ParameterVector<OutType>
LinearTransform<OutType, InType>::apply(const dual_t& vector) const
{
    ParameterVector<OutType> result;
    for(auto& [k,f]: duals)
    {
        result.values.try_emplace(k, f.dot(vector));
    }
    return result;
}

template<typename OutType, typename InType>
ParameterVector<InType>
LinearTransform<OutType, InType>::project(const dual_t& vector) const
{
    ParameterVector<InType> result;
    for(auto& [k,f]: duals)
    {
        result += f.dot(vector) * f;
    }
    return result;
}

} // namespace NamedSketcher::GCS
