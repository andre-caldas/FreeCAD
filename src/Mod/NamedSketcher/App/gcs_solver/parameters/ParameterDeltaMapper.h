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


#ifndef NAMEDSKETCHER_GCS_ParameterDeltaMapper_H
#define NAMEDSKETCHER_GCS_ParameterDeltaMapper_H

#include "ParameterValueMapper.h"

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
class ParameterDeltaMapper
    : public ParameterValueMapper
{
public:
    /**
     * @brief Used for partial derivatives.
     * When parameter == @a direction, it adds @a delta to the computed value.
     * @param direction: corresponds to the dimension where we want
     * to calculate the partial derivative.
     * @param delta: the "delta" ammout to add to *direction.
     */
    ParameterDeltaMapper(const ParameterValueMapper& pvm, const Parameter* direction, double delta)
        : ParameterValueMapper(pvm), direction(direction), delta(delta) {}

    double getValue(const Parameter* p) const override;

private:
    const Parameter* direction;
    double delta;
};

inline double ParameterDeltaMapper::getValue(const Parameter* p) const
{
    double current_value = _getValue(p);
    return (p != direction) ? current_value : (current_value + delta);
}

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterDeltaMapper_H
