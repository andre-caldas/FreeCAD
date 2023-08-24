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

from itertools import combinations, permutations, product

import FreeCAD as App
from draftutils import utils

import NamedSketcher
from draftutils.translate import translate


class Draft2NamedSketch:
    def __init__(self, objects_list, tolerance = Tolerance()):
        self.objects_list = objects_list
        self.tolerance = tolerance

    def generate_sketch(self):
        self.geometries_data = []
        self.all_points_data = []
        sketch = NamedSketcher.NamedSketch();
        create_geometries(sketch)
        generate_constraints(sketch)
        return sketch

    def create_geometries(self, sketch):
        self.check_normal()

        for obj in self.objects_list:
            tp = utils.get_type(obj)
            geo = None
            shape = obj.Shape.copy()
            if shape.Edges:
                curve = shape.Edges[0].Curve
            else:
                point = shape.Point

            if  tp == "Point":
                geo = create_point(point)

            elif tp == "Circle":
                geo = create_circle(curve):

            elif tp == "Ellipse":
                geo = create_ellipse(curve):

            elif tp in ["Wire", "Rectangle", "Polygon"] and obj.FilletRadius.Value == 0:
                for edge in shape.Edges:
                    geo = create_linesegment(curve)

            elif tp == "BSpline":
                geo = create_bspline(curve)

            elif tp == "BezCurve":
                geo = create_bezcurve(curve)

            else:
                App.Console.PrintError(translate("draft",
                                       "Cannot convert curve of type {}").format(tp)+"\n")

        if geo is not None:
            geo_ref = sketch.addGeometry(geo)
            geo_data = GeometryData(geo_ref, geo)
            self.gemetries_data.append(geo_data)
            self.all_points_data += geo_data.points


    def check_normal(self):
        z_level = None
        for obj in self.object_list:
            if isinstance(obj,Part.Shape):
                shape = obj
            elif hasattr(obj,'Shape'):
                shape = obj.Shape
            else:
                raise TypeError("No shape found")

            for v in shape.Vertexes:
                if z_level is None:
                    z_level = v.z
                if z_level != v.z:
                    raise TypeError("All shapes must have the same z-coordinate")


    def generate_constraints(self, sketch):
        pin_first_point(self, sketch)
        generate_coincident_constraints(self, sketch)
        generate_horizontal_constraints(self, sketch)
        generate_vertical_constraints(self, sketch)
        generate_point_over_curve_constraints(self, sketch)
        generate_tangent_curve_constraints(self, sketch)
        generate_normal_curve_constraints(self, sketch)
        generate_circle_radius_constraints(self, sketch)

    def pin_first_point(self, sketch):
        if self.all_points_data:
            pt_ref = self.all_points_data[0].ref
            sketch.addConstraint(NamedSketcher.ConstraintBlockPoint(pt_ref))

    def generate_coincident_constraints(self, sketch):
        coincident_groups = {p: set() for p in self.all_points_data}
        for p1, p2 in combinations(self.all_points_data, 2):
            if self.tolerance.are_coincident(p1.obj, p2.obj):
                coincident_groups[p1] add(p2)
        processed_points = set()
        partition = set()

        processed_points = set()
        def get_equivalent_class(p, result=set()):
            if p in processed_points:
                continue
            processed_points.add(p)
            result.add(p)
            for n in coincident_groups[p]:
                if not n in result:
                    get_equivalent_class(n, result)

        for p in self.all_points_data:
            if p in processed_points:
                continue
            equivalent_class = get_equivalent_class(p)

            if len(equivalent_class) <= 1:
                continue

            constraint = NamedSketcher.ConstraintCoincident()
            for a in equivalent_class:
                constraint.addPoint(a.ref)
            sketch.addConstraint(constraint)

    def generate_horizontal_constraints(self, sketch):
        for g in self.geometries_data:
            if self.tolerance.is_horizontal(g.ref):
                sketch.addConstraint(NamedCurve.ConstraintHorizontal(g.ref))

    def generate_vertical_constraints(self, sketch):
        for g in self.geometries_data:
            if self.tolerance.is_vertical(g.ref):
                sketch.addConstraint(NamedCurve.ConstraintVertical(g.ref))

    def generate_point_over_curve_constraints(self, sketch):
        for g1, g2 in permutations(self.geometries_data):
            for p in g1.points:
                self.tolerance.is_point_over_curve(p.obj, g2.obj):
                    sketch.addConstraint(NamedCurve.ConstraintPointOverCurve(p.ref, g2.ref))

    def generate_tangent_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometries_data, 2):
            if self.tolerance.are_curves_tangent(g1.obj, g2.obj):
                sketch.addConstraint(NamedCurve.ConstraintTangent(g1.ref, g2.ref))

    def generate_normal_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometries_data, 2):
            if self.tolerance.are_curves_normal(g1.obj, g2.obj):
                sketch.addConstraint(NamedCurve.ConstraintNormal(g1.ref, g2.ref))

    def generate_circle_radius_constraints(self, sketch):
        for g in self.geometries_data:
            if self.tolerance.is_radius_underconstrained(g.ref):
                sketch.addConstraint(NamedCurve.ConstraintConstant(g.ref + "radius"))


#
# Geometry data.
#

class PointData:
    def __init__(ref, obj):
        self.ref = ref
        self.obj = obj

class GeometryData:
    def __init__(ref, obj):
        self.ref = ref
        self.obj = obj
        self.points = []
        for p_ref in obj.getReferencesToPoints():
            p_obj = p_ref.resolve()
            self.points.append(PointData(p_ref, p_obj))

#
# Geometry creation.
#

def create_point(point):
    return NamedSketcher.Point(point)

def create_circle(curve):
    return NamedSketcher.Circle(curve)

def create_ellipse(curve):
    return None

def create_linesegment(curve):
    return NamedSketcher.LineSegment(curve)

def create_bspline(curve):
    return None

def create_bezcurve(curve):
    return None


#
# Geometric constraints check.
#
class Tolerance:
    # TODO: set distance_tolerance according to situation. For example:
    # - Size of the geometries.
    # - GUI zoom.
    def __init__(distance_tolerance=10):
        self.distance_tolerance = distance_tolerance

    def are_equal(a, b):
        return abs(a - b) < self.distance_tolerance

    def are_coincident(p1, p2):
        return self.are_equal(p1.x, p2.x) && self.are_equal(p1.x, p2.x)

    def is_horizontal(geo):
        try:
            start = (geo + "start").resolvePoint()
            end = (geo + "end").resolvePoint()
        except NamedSketcher.ExceptionCannotResolve:
            return false
        return self.are_equal(start.y, end.y)

    def is_vertical(geo):
        try:
            start = (geo + "start").resolvePoint()
            end = (geo + "end").resolvePoint()
        except NamedSketcher.ExceptionCannotResolve:
            return false
        return self.are_equal(start.x, end.x)

    def is_point_over_curve(p, geo):
        return false

    def are_curves_tangent(geo1, geo2):
        return false

    def are_curves_normal(geo1, geo2):
        return false

    def is_radius_underconstrained(geo):
        try:
            radius = (geo + "radius").resolveParameter()
        except NamedSketcher.ExceptionCannotResolve:
            return false
        return true # TODO: implement underconstrainment check.

