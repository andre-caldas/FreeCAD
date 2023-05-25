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


#ifndef NAMEDSKETCHER_GCS_ProxiedParameter_H
#define NAMEDSKETCHER_GCS_ProxiedParameter_H

#include <Base/Vector3D.h>

#include "NamedSketcherGlobal.h"

namespace NamedSketcher::GCS
{

/**
 * @brief The ProxiedParameter class allow us to treat, outside the solver,
 * parameters that are constrained to be equal.
 *
 * The geometric object's parameters are supposed to be \class ProxiedParameter.
 * When a geometry joins a \class ConstraintEqual, its proxy is redirected (\a setProxy)
 * to a value internal to \class ConstraintEqual.
 */
class NamedSketcherExport ProxiedParameter
{
public:
    ProxiedParameter(double value) : value(value) {}

    double getValue() const {return *proxy;}
    double getPointer() const {return proxy;}
    bool samePointer(ProxiedParameter* other) const {return proxy == other->proxy;}
    bool samePointer(ProxiedParameter& other) const {return proxy == other.proxy;}
    bool hasProxy() const {return proxy != &value;}
    void setProxy(double* parameter_address) {proxy = parameter_address;}
    void resetProxy(bool shallUpdate = true);
    void update() {if(proxy != &value) value = *proxy;}

private:
    double value;
    double* proxy = &value;

public: // :-(
    ProxiedParameter() = default;
};

class NamedSketcherExport ProxiedPoint
{
public:
    ProxiedPoint(double x, double y) : x(x), y(y) {}
    ProxiedParameter x;
    ProxiedParameter y;

    operator Base::Vector3d (void) const;

public: // :-(
    ProxiedPoint() = default;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ProxiedParameter_H
