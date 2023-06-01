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


#ifndef NAMEDSKETCHER_GCS_Parameter_H
#define NAMEDSKETCHER_GCS_Parameter_H

namespace NamedSketcher::GCS
{

/*
 * Unfortunately, C++ does not allow us to derive from "double". :-(
 */
class ParameterBase
{
public:
    explicit ParameterBase(double v) : value(value) {}
    explicit ParameterBase& operator= (double v) {value = v; return *this;}

    operator double&() {return value;}
    ParameterBase& operator+= (double v) {value += v; return *this;}
    ParameterBase& operator-= (double v) {value -= v; return *this;}
    ParameterBase& operator*= (double v) {value *= v; return *this;}

private:
    double value;
};

class Parameter : public ParameterBase {};
class OptimizedParameter : public ParameterBase {};

class Point
{
public:
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}
    Parameter x = 0.0;
    Parameter y = 0.0;
};


} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Parameter_H
