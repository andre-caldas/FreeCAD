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

#include "Parameter.h"

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

    ParameterValueMapper(const ParameterValueMapper& pvm)
        : parent_mapper(&pvm) {}

    virtual double getValue(const Parameter* p) const = 0;
    double operator()(const Parameter* p) const {return getValue(p);}
    double operator()(const Parameter& p) const {return (*this)(&p);}

protected:
    double _getValue(const Parameter* p) const;

private:
    const ParameterValueMapper* parent_mapper = nullptr;
};

inline double ParameterValueMapper::_getValue(const Parameter* p) const
{
    if(parent_mapper)
    {
        return (*parent_mapper)(p);
    }
    return *p;
}

class ParameterValueMapperDumb
    : public ParameterValueMapper
{
    double getValue(const Parameter* p) const override {return _getValue(p);}
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterValueMapper_H
