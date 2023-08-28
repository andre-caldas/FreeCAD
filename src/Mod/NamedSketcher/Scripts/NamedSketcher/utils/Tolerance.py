## SPDX-License-Identifier: LGPL-2.1-or-later
# ***************************************************************************
# *                                                                         *
# *  Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
# *                                                                         *
# *  This file is part of FreeCAD.                                          *
# *                                                                         *
# *  FreeCAD is free software: you can redistribute it and/or modify it     *
# *  under the terms of the GNU Lesser General Public License as            *
# *  published by the Free Software Foundation, either version 2.1 of the   *
# *  License, or (at your option) any later version.                        *
# *                                                                         *
# *  FreeCAD is distributed in the hope that it will be useful, but         *
# *  WITHOUT ANY WARRANTY; without even the implied warranty of             *
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
# *  Lesser General Public License for more details.                        *
# *                                                                         *
# *  You should have received a copy of the GNU Lesser General Public       *
# *  License along with FreeCAD. If not, see                                *
# *  <https://www.gnu.org/licenses/>.                                       *
# *                                                                         *
# **************************************************************************/
"""Provides functions to create NamedSketch objects from Draft objects."""

import NamedSketcher # For exception, while Base::Accessor does not use pybind.

#
# Geometric tolerance check.
#
class Tolerance:
    scale = 10

    def __init__(self, scale_tolerance=.03):
        self.scale_tolerance = scale_tolerance

    @property
    def distance_tolerance(self):
        return self.scale * self.scale_tolerance

    def are_very_equal(self, a, b):
        return abs(a - b) < 1e-6

    def is_zero(self, v):
        return abs(v) < self.distance_tolerance

    def are_equal(self, a, b):
        return self.is_zero(a - b)

    def are_coincident(self, p1, p2):
        return self.are_equal(p1.x, p2.x) and self.are_equal(p1.y, p2.y)

    def is_horizontal(self, geo):
        try:
            start = (geo + "start").resolvePoint()
            end = (geo + "end").resolvePoint()
        except NamedSketcher.ExceptionCannotResolve:
            return False
        return self.are_equal(start.y, end.y)

    def is_vertical(self, geo):
        try:
            start = (geo + "start").resolvePoint()
            end = (geo + "end").resolvePoint()
        except NamedSketcher.ExceptionCannotResolve:
            return False
        return self.are_equal(start.x, end.x)

    def is_point_along_curve(self, vertex, shape):
        dist, points, infos = vertex.distToShape(shape)
        if not infos or infos[0][3] == 'Vertex':
            # Coincident points.
            return False;
        return self.is_zero(dist)

    def are_curves_tangent(self, shape1, shape2):
        print('Are curves tangent???')
        print(shape1, shape2)
        dist, points, infos = shape1.distToShape(shape2)
        print('Info...', dist, points, infos)
        print(dist, points, infos)
        if self.is_zero(dist):
            for info in infos:
                if info[0] == 'Vertex' or info[3] == 'Vertex':
                    # Point along curve or coincident points.
                    # TODO: treat all at once! And also allow tangent point along curve.
                    continue
                print(info)
                tg1 = shape1.tangentAt(info[2])
                tg2 = shape2.tangentAt(info[5])
                if self.is_zero(tg1.cross(tg2).Length):
                    return True
        return False

    def are_curves_normal(self, geo1, geo2):
        return False
