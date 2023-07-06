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


#ifndef NAMEDSKETCHER_GCS_ParameterValueMapper_H
#define NAMEDSKETCHER_GCS_ParameterValueMapper_H

#include "ParameterGroupManager.h"

namespace NamedSketcher::GCS
{

class Parameter;

/**
 * @brief Sometimes we have a @class ParameterGroupManager active.
 * That is, when we are solving the GCS.
 * Sometimes we do not have any of those.
 * In order to avoid having two versions of some methods that need
 * to access the GCS::Parameter value, we wrapped those two concepts here.
 *
 * Also,
 * when calculating partial derivatives, we need to disturb a little
 * (delta) just one "direction".
 * This class also helps with this task.
 */
class ParameterValueMapper
{
public:
    /**
     * @brief Maps a parameter to *parameter.
     */
    ParameterValueMapper() {}

    /**
     * @brief Uses manager::getValue to map parameter to value.
     */
    ParameterValueMapper(const ParameterGroupManager& manager)
        : manager(&manager) {}

    /**
     * @brief Used for partial derivatives.
     * When parameter == @a direction, it adds @a delta to the computed value.
     * @param direction: corresponds to the dimension where we want
     * to calculate the partial derivative.
     * @param delta: the "delta" ammout to add to *direction.
     */
    ParameterValueMapper(const ParameterValueMapper& pvm, const Parameter* direction = nullptr, double delta = 0)
        : parent_mapper(&pvm), delta(delta), direction(direction) {}

    inline double operator()(const Parameter* p) const;
    double operator()(const Parameter& p) const {return (*this)(&p);}

private:
    const ParameterValueMapper* parent_mapper = nullptr;
    const ParameterGroupManager* manager = nullptr;
    const double delta = 0;
    const Parameter* direction = nullptr;
};

inline double ParameterValueMapper::operator()(const Parameter* p) const
{
    double parent_value = *p;
    if(parent_mapper)
    {
        parent_value = (*parent_mapper)(p);
    }
    else if(manager)
    {
        parent_value = manager->getValue(p);
    }
    return (p != direction) ? parent_value : (parent_value + delta);
}

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterValueMapper_H
