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

#
# Geometric tolerance check.
#
class Tolerance:
    # TODO: set distance_tolerance according to situation. For example:
    # - Size of the geometries.
    # - GUI zoom.
    def __init__(self, distance_tolerance=10):
        self.distance_tolerance = distance_tolerance

    def are_very_equal(self, a, b):
        return abs(a - b) < 1e-9

    def are_equal(self, a, b):
        return abs(a - b) < self.distance_tolerance

    def are_coincident(self, p1, p2):
        return self.are_equal(p1.x, p2.x) and self.are_equal(p1.x, p2.x)

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

    def is_point_over_curve(self, p, geo):
        return False

    def are_curves_tangent(self, geo1, geo2):
        return False

    def are_curves_normal(self, geo1, geo2):
        return False
