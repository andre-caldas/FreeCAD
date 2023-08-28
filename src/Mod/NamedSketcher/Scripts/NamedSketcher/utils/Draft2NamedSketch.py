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

from math import sqrt
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

        # The greatest length of all edges.
        reference_length = 1.

        geo = []
        for obj in self.object_list:
            tp = utils.get_type(obj)
            print(f'New object of type {tp}')
            shape = obj.Shape.copy()
            if shape.Edges:
                reference_length = max(max([e.Length for e in shape.Edges]), reference_length)
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

        self.tolerance.scale = reference_length
        for geo_data in geo:
            if geo_data is None:
                continue
            geo_data.ref = sketch.addGeometry(geo_data.obj)
            self.geometries_data.append(geo_data)
            self.all_points_data += geo_data.points.values()


    def check_normal(self):
        self.z_level = None
        for obj in self.object_list:
            if isinstance(obj,Part.Shape):
                shape = obj
            elif hasattr(obj,'Shape'):
                shape = obj.Shape
            else:
                raise TypeError("No shape found")

            for v in shape.Vertexes:
                if self.z_level is None:
                    self.z_level = v.Z
                if not self.tolerance.are_very_equal(self.z_level, v.Z):
                    raise TypeError("All shapes must have the same z-coordinate")


    def generate_constraints(self, sketch):
        self.pin_first_point(sketch)
        self.generate_coincident_constraints(sketch)
        self.generate_horizontal_constraints(sketch)
        self.generate_vertical_constraints(sketch)
        self.generate_point_along_curve_constraints(sketch)
        self.generate_tangent_curve_constraints(sketch)
        self.generate_normal_curve_constraints(sketch)
        self.generate_extra_constraints(sketch)

    def pin_first_point(self, sketch):
        if self.all_points_data:
            pt_ref = self.all_points_data[0].ref
            pt_x = self.all_points_data[0].x
            pt_y = self.all_points_data[0].y
            sketch.addConstraint(NamedSketcher.ConstraintBlockPoint(pt_ref, pt_x, pt_y))

    def generate_coincident_constraints(self, sketch):
        coincident_groups = {p: set() for p in self.all_points_data}
        for p1, p2 in combinations(self.all_points_data, 2):
            if self.tolerance.are_coincident(p1.gcs_point, p2.gcs_point):
                print('Coincident!', p1.gcs_point, p2.gcs_point)
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
                print('Adding point:', a.gcs_point)
                constraint.addPoint(a.ref)
            sketch.addConstraint(constraint)

    def generate_horizontal_constraints(self, sketch):
        for g in self.geometries_data:
            if self.tolerance.is_horizontal(g.ref):
                g.is_horizontal = True
                sketch.addConstraint(NamedSketcher.ConstraintHorizontal(g.ref))

    def generate_vertical_constraints(self, sketch):
        for g in self.geometries_data:
            if(g.is_horizontal):
                continue
            if self.tolerance.is_vertical(g.ref):
                g.is_vertical = True
                sketch.addConstraint(NamedSketcher.ConstraintVertical(g.ref))

    def generate_point_along_curve_constraints(self, sketch):
        for g1, g2 in permutations(self.geometries_data, 2):
            for p in g1.points.values():
                v = Part.Vertex(App.Vector(p.x, p.y, self.z_level))
                if self.tolerance.is_point_along_curve(v, g2.shape):
                    sketch.addConstraint(NamedSketcher.ConstraintPointAlongCurve(p.ref, g2.ref))

    def generate_tangent_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometries_data, 2):
            if self.tolerance.are_curves_tangent(g1.shape, g2.shape):
                sketch.addConstraint(NamedSketcher.ConstraintTangentCurves(g1.ref, g2.ref))

    def generate_normal_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometries_data, 2):
            if self.tolerance.are_curves_normal(g1.shape, g2.shape):
                #sketch.addConstraint(NamedSketcher.ConstraintNormal(g1.ref, g2.ref))
                print('Implement ConstraintOrthogonalCurves.')

    def generate_extra_constraints(self, sketch):
        # TODO: separate in different lists according to priority.
        for g in self.geometries_data:
            g.generate_extra_constraints(sketch)


#
# Geometry data.
#

class PointData:
    def __init__(self, ref, gcs_point):
        self.ref = ref
        self.gcs_point = gcs_point
        self.x = gcs_point.x
        self.y = gcs_point.y

class GeometryData:
    is_vertical = False
    is_horizontal = False
    def __init__(self, obj, shape, extra_constraints_fabric):
        self.obj = obj
        self.shape = shape
        self.points = {}
        self.extra_constraints_fabric = extra_constraints_fabric
        for p_ref in obj.getReferencesToPoints():
            gcs_point = p_ref.resolve()
            self.points[p_ref] = PointData(p_ref, gcs_point)

    def generate_extra_constraints(self, sketch):
        for constraint in self.extra_constraints_fabric(self):
            if sketch.isConstraintIndependent(constraint):
                sketch.addConstraint(constraint)

#
# Geometry creation.
#

def create_point(point):
    def extra_constraint_fabric(data):
        return []
    g = NamedSketcher.Point(point)
    return GeometryData(g, point.toShape(), extra_constraint_fabric)

def create_circle(edge):
    def extra_constraint_fabric(data):
        radius = (data.ref + "radius").resolveParameter().value
        return [NamedSketcher.ConstraintConstant(data.ref + "radius", radius)]

    part = Part.Circle(edge.Curve)
    g = NamedSketcher.Geometry(part)
    return GeometryData(g, part.toShape(), extra_constraint_fabric)

def create_ellipse(edge):
    return None

def create_linesegment(edge):
    def extra_constraint_fabric(data):
        start = (data.ref + "start").resolvePoint()
        end = (data.ref + "end").resolvePoint()
        result = []
        if data.is_horizontal:
            result.append(NamedSketcher.ConstraintXDistance(data.ref, end.x-start.x))
        elif data.is_vertical:
            result.append(NamedSketcher.ConstraintYDistance(data.ref, end.y-start.y))
        else:
            print("Implement ConstraintDistance!")
            #length = sqrt((start.x-end.x)**2 + (start.y-end.y)**2)
            #sketch.addConstraint(NamedSketcher.ConstraintDistance(data.ref, length))
        return result

    part = Part.LineSegment(edge.Curve, edge.FirstParameter, edge.LastParameter)
    g = NamedSketcher.Geometry(part)
    return GeometryData(g, part.toShape(), extra_constraint_fabric)

def create_bspline(edge):
    return None

def create_bezcurve(edge):
    return None
