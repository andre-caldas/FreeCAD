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


#ifndef NAMEDSKETCHER_GCS_ParameterShaker_H
#define NAMEDSKETCHER_GCS_ParameterShaker_H

#include <random>
#include <unordered_map>

#include "ParameterValueMapper.h"

namespace NamedSketcher::GCS
{

class ParameterShaker
    : public ParameterValueMapper
{
public:
    ParameterShaker(double epsilon)
        : data(std::make_unique<RandomData>(epsilon))
    {}

    double getValue(const Parameter* p) const override;

private:
    // We use a pointer to a structure, because "getValue()" is const.
    struct RandomData
    {
        RandomData(double epsilon)
            : random_generator(std::random_device()())
            , distribution(0.0, epsilon)
        {}
        std::mt19937 random_generator;
        std::uniform_real_distribution<> distribution;
        std::unordered_map<const Parameter*, double> choosen_random;
    };
    std::unique_ptr<RandomData> data;
};

inline double ParameterShaker::getValue(const Parameter* p) const
{
    if(!data->choosen_random.count(p))
    {
        data->choosen_random[p] = data->distribution(data->random_generator);
    }
    double current_value = _getValue(p);
    return current_value + data->choosen_random[p];
}

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ParameterShaker_H
