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

#if FC_DEBUG
#include <iostream>
#endif // FC_DEBUG
#include <Base/Vector3D.h>

namespace NamedSketcher::GCS
{

/*
 * Unfortunately, C++ does not allow us to derive from "double". :-(
 */
class ParameterBase
{
public:
    ParameterBase() = default;
    ParameterBase(std::string name, double v = 0);
    ParameterBase(double v) : value(v) {}
    ParameterBase& operator= (double v) {value = v; return *this;}

    operator double&() {return value;}
    operator double() const {return value;}
    ParameterBase& operator+= (double v) {value += v; return *this;}
    ParameterBase& operator-= (double v) {value -= v; return *this;}
    ParameterBase& operator*= (double v) {value *= v; return *this;}

private:
    double value;

#if FC_DEBUG
public:
    const std::string name;
#endif // FC_DEBUG
};

#if FC_DEBUG
inline ParameterBase::ParameterBase(std::string name, double v) : value(v), name(name) {}
inline std::ostream& operator<<(std::ostream& out, const ParameterBase& p)
{
    out << "(" << p.name << (p.name.empty()?"":": ") << (double)p << ")";
    return out;
}
#else
ParameterBase::ParameterBase(std::string, double v) : ParameterBase(v) {}
#endif // FC_DEBUG

class Parameter : public ParameterBase
{
public:
    using ParameterBase::ParameterBase;
    Parameter& operator= (double v) {*(ParameterBase*)this= v; return *this;} // Cargo cult.
};

class OptimizedParameter : public ParameterBase
{
public:
    using ParameterBase::ParameterBase;
    OptimizedParameter& operator= (double v) {*(ParameterBase*)this= v; return *this;} // Cargo cult.
};


class Point
{
public:
    Parameter x = 0.0;
    Parameter y = 0.0;

    Point() = default;
    Point(std::string name, double x = 0, double y = 0);
    Point(double x, double y) : x(x), y(y) {}
    Point(const Base::Vector3d& p) : Point(p.x, p.y) {}
    Point(std::string name, const Base::Vector3d& p) : Point(name, p.x, p.y) {}

    Point& normalize();
    operator Base::Vector3d() const;

#if FC_DEBUG
public:
    const std::string name;
#endif // FC_DEBUG
};

#if FC_DEBUG
inline Point::Point(std::string name, double x, double y) : x(name + ".x",x), y(name + ".y",y), name(name) {}
inline std::ostream& operator<<(std::ostream& out, const Point& p)
{
    out << "(" << p.name << (p.name.empty()?"":": ") << (double)p.x << "," << (double)p.y << ")";
    return out;
}
#else
Point::Point(std::string) {}
#endif // FC_DEBUG

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Parameter_H
