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


#ifndef NAMEDSKETCHER_GCS_Colinear_H
#define NAMEDSKETCHER_GCS_Colinear_H

#include <set>

#include "../ProxiedParameter.h""
#include "../Types.h"
#include "Equation.h"

namespace NamedSketcher::GCS
{

class NamedSketcherExport Colinear : public NonLinearEquation
{
public:
    Colinear() = default;
    void set(ProxiedPoint* a, ProxiedPoint* b, ProxiedPoint* c);

    double error() const override;
    Vector differentialNonOptimized() const override;
    OptimizedVector differentialOptimized() const override;

    bool setProxies(ParameterProxyManager* manager) const override;

private:
    ProxiedPoint* a;
    ProxiedPoint* b;
    ProxiedPoint* c;

    bool isAlreadyColinear() const;
    bool isHorizontal() const;
    bool isVertical() const;
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_Colinear_H
