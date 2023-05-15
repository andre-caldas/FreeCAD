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
    ProxiedParameter() = default;

    double getValue() const {return *proxy;}
    void setProxy(double* parameter_address) : proxy(parameter_address) {}
    void resetProxy(bool shallUpdate = true);
    void update() {if(proxy != &value) value = *proxy;}

private:
    double value;
    double* proxy = &value;
};

class NamedSketcherExport ProxiedPoint
{
public:
    ProxiedParameter x;
    ProxiedParameter y;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ProxiedParameter_H
