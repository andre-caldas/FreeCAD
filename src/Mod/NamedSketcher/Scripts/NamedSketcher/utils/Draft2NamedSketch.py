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
import Part

import NamedSketcher
from draftutils.translate import translate

from .Tolerance import Tolerance

class Draft2NamedSketch:
    def __init__(self, object_list, tolerance = Tolerance()):
        self.object_list = object_list
        self.tolerance = tolerance

    def generate_sketch(self):
        self.geometries_data = []
        self.all_points_data = []
        sketch = NamedSketcher.NamedSketch()
        self.create_geometries(sketch)
        self.generate_constraints(sketch)
        return sketch

    def create_geometries(self, sketch):
        self.check_normal()

        geo = []
        for obj in self.object_list:
            tp = utils.get_type(obj)
            print(f'New object of type {tp}')
            shape = obj.Shape.copy()
            if shape.Edges:
                edge = shape.Edges[0]
            else:
                point = shape.Point

            if  tp == "Point":
                geo.append(create_point(point))

            elif tp == "Circle":
                geo.append(create_circle(edge))

            elif tp == "Ellipse":
                geo.append(create_ellipse(edge))

            elif tp in ["Wire", "Rectangle", "Polygon"] and obj.FilletRadius.Value == 0:
                for edge in shape.Edges:
                    geo.append(create_linesegment(edge))

            elif tp == "BSpline":
                geo.append(create_bspline(edge))

            elif tp == "BezCurve":
                geo.append(create_bezcurve(edge))

            else:
                App.Console.PrintError(translate("draft",
                                       "Cannot convert curve of type {}").format(tp)+"\n")

        for g in geo:
            if g is None:
                continue
            geo_ref = sketch.addGeometry(g)
            geo_data = GeometryData(geo_ref, g)
            self.geometries_data.append(geo_data)
            self.all_points_data += geo_data.points.values()


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
                    z_level = v.Z
                if not self.tolerance.are_very_equal(z_level, v.Z):
                    raise TypeError("All shapes must have the same z-coordinate")


    def generate_constraints(self, sketch):
        self.pin_first_point(sketch)
        self.generate_coincident_constraints(sketch)
        self.generate_horizontal_constraints(sketch)
        self.generate_vertical_constraints(sketch)
        self.generate_point_along_curve_constraints(sketch)
        self.generate_tangent_curve_constraints(sketch)
        self.generate_normal_curve_constraints(sketch)
        self.generate_circle_radius_constraints(sketch)

    def pin_first_point(self, sketch):
        if self.all_points_data:
            pt_ref = self.all_points_data[0].ref
            pt_obj = self.all_points_data[0].obj
            sketch.addConstraint(NamedSketcher.ConstraintBlockPoint(pt_ref, pt_obj.x, pt_obj.y))

    def generate_coincident_constraints(self, sketch):
        coincident_groups = {p: set() for p in self.all_points_data}
        for p1, p2 in combinations(self.all_points_data, 2):
            if self.tolerance.are_coincident(p1.obj, p2.obj):
                print('Coincident!', p1.obj, p2.obj)
                coincident_groups[p1].add(p2)

        processed_points = set()
        def get_equivalent_class(p, result=None):
            if result is None:
                result = set()
            if p in processed_points:
                return result
            processed_points.add(p)
            result.add(p)
            for n in coincident_groups[p]:
                if not n in result:
                    get_equivalent_class(n, result)
            return result

        for p in self.all_points_data:
            if p in processed_points:
                continue
            equivalent_class = get_equivalent_class(p)

            if len(equivalent_class) <= 1:
                continue

            constraint = NamedSketcher.ConstraintCoincident()
            print('Coincident points...')
            for a in equivalent_class:
                print('Adding point:', a.obj)
                constraint.addPoint(a.ref)
            sketch.addConstraint(constraint)

    def generate_horizontal_constraints(self, sketch):
        for g in self.geometries_data:
            if self.tolerance.is_horizontal(g.ref):
                sketch.addConstraint(NamedSketcher.ConstraintHorizontal(g.ref))

    def generate_vertical_constraints(self, sketch):
        for g in self.geometries_data:
            if self.tolerance.is_vertical(g.ref):
                sketch.addConstraint(NamedSketcher.ConstraintVertical(g.ref))

    def generate_point_along_curve_constraints(self, sketch):
        for g1, g2 in permutations(self.geometries_data, 2):
            for p in g1.points.values():
                if self.tolerance.is_point_along_curve(p.obj, g2.obj):
                    sketch.addConstraint(NamedSketcher.ConstraintPointAlongCurve(p.ref, g2.ref))

    def generate_tangent_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometries_data, 2):
            if self.tolerance.are_curves_tangent(g1.obj, g2.obj):
                sketch.addConstraint(NamedSketcher.ConstraintTangentCurves(g1.ref, g2.ref))

    def generate_normal_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometries_data, 2):
            if self.tolerance.are_curves_normal(g1.obj, g2.obj):
                #sketch.addConstraint(NamedSketcher.ConstraintNormal(g1.ref, g2.ref))
                print('Implement ConstraintOrthogonalCurves.')

    def generate_circle_radius_constraints(self, sketch):
        for g in self.geometries_data:
            if self.is_radius_underconstrained(g.ref):
                # TODO: do not use geometryFactory, so we can access parameters directly.
                radius = (g.ref + "radius").resolveParameter().value
                sketch.addConstraint(NamedSketcher.ConstraintConstant(g.ref + "radius", radius))

    def is_radius_underconstrained(self, geo):
        try:
            radius = (geo + "radius").resolveParameter()
        except NamedSketcher.ExceptionCannotResolve:
            return False
        return True # TODO: implement underconstrainment check.


#
# Geometry data.
#

class PointData:
    def __init__(self, ref, obj):
        self.ref = ref
        self.obj = obj

class GeometryData:
    def __init__(self, ref, obj):
        self.ref = ref
        self.obj = obj
        self.points = {}
        for p_ref in obj.getReferencesToPoints():
            p_obj = p_ref.resolve()
            self.points[p_ref] = PointData(p_ref, p_obj)

#
# Geometry creation.
#

def create_point(point):
    return NamedSketcher.Point(point)

def create_circle(edge):
    part = Part.Circle(edge.Curve)
    print('Created circle?', part)
    return NamedSketcher.Geometry(part)

def create_ellipse(edge):
    return None

def create_linesegment(edge):
    part = Part.LineSegment(edge.Curve, edge.FirstParameter, edge.LastParameter)
    print('New line segment created')
    return NamedSketcher.Geometry(part)

def create_bspline(edge):
    return None

def create_bezcurve(edge):
    return None
