## SPDX-License-Identifier: LGPL-2.1-or-later
# ***************************************************************************
# *                                                                         *
# *   Copyright (c) 2009, 2010 Yorik van Havre <yorik@uncreated.net>        *
# *   Copyright (c) 2009, 2010 Ken Cline <cline@frii.com>                   *
# *   Copyright (c) 2020 FreeCAD Developers                                 *
# *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
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
import DraftGeomUtils
from draftutils import utils
from draftutils import gui_utils

import NamedSketcher
from draftutils.translate import translate


class Draft2NamedSketch:
    tolerance = 1e-9

    def __init__(self, objects_list):
        self.objects_list = objects_list

    def generate_sketch(self):
        self.geometries_data = []
        self.all_points_data = []
        sketch = NamedSketcher.NamedSketch();
        create_geometries(sketch)
        generate_constraints(sketch)
        return sketch

    def create_geometries(self, sketch):
        normal = get_normal()
        if normal is None:
            return

        # TODO: I don't really understand what coordinate system
        # this "normal" refers to.
        # As a consequence, I do not understand how to properly set up sketch.Placement.
        if normal.x || normal.y:
            rot = App.Rotation(App.Base.Vector(0,0,1), normal)
        else:
            rot = App.Rotation()
        sketch.Placement(App.Base.Vector(0,0,0), rot.inverse())

        for obj in self.objects_list:
            tp = utils.get_type(obj)
            geo = None
            shape = obj.Shape.copy()
            shape.rotate(rot)
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


    def get_normal(self):
        # We only use the view direction when the whole shape is contained
        # in a straight line.
        if App.GuiUp:
            v_dir = gui_utils.get_3d_view().getViewDirection()
        else:
            v_dir = App.Base.Vector(0,0,-1)

        # lists to accumulate shapes with defined normal and undefined normal
        shape_norm_yes = list()
        shape_norm_no = list()

        for obj in self.object_list:
            if isinstance(obj,Part.Shape):
                shape = obj
            elif hasattr(obj,'Shape'):
                shape = obj.Shape
            else:
                App.Console.PrintError(translate("draft",
                                       "No shape found")+"\n")
                return None

            if not DraftGeomUtils.is_planar(shape, self.tolerance):
                App.Console.PrintError(translate("draft",
                                       "All Shapes must be planar")+"\n")
                return None

            if DraftGeomUtils.get_normal(shape, self.tolerance):
                shape_norm_yes.append(shape)
            else:
                shape_norm_no.append(shape)


        shapes_list = shape_norm_yes + shape_norm_no

        # test if all shapes are coplanar
        if len(shape_norm_yes) >= 1:
            for shape in shapes_list:
                if not DraftGeomUtils.are_coplanar(shape_norm_yes[0], shape, self.tolerance):
                    App.Console.PrintError(translate("draft",
                                           "All Shapes must be coplanar")+"\n")
                    return None
            # define sketch normal
            normal = DraftGeomUtils.get_normal(shape_norm_yes[0], self.tolerance)

        else:
            # suppose all geometries are straight lines or points
            points = [vertex.Point for shape in shapes_list for vertex in shape.Vertexes]
            if len(points) >= 2:
                poly = Part.makePolygon(points)
                if not DraftGeomUtils.is_planar(poly, self.tolerance):
                    App.Console.PrintError(translate("draft",
                                           "All Shapes must be coplanar")+"\n")
                    return None
                normal = DraftGeomUtils.get_normal(poly, self.tolerance)
                if not normal:
                    # all points aligned
                    poly_dir = poly.Edges[0].Curve.Direction
                    normal = (v_dir - v_dir.dot(poly_dir)*poly_dir).normalize()
                    normal = normal.negative()
            else:
                # only one point
                normal = v_dir.negative()
        return normal


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
            if are_coincident(p1.obj, p2.obj, self.tolerance):
                coincident_groups[p1] add(p2)
        processed_points = set()
        partition = set()

        def get_equivalent_class(p, result=set()):
            result.add(p)
            for n in coincident_groups[p]:
                if not n in result:
                    get_equivalent_class(n, result)

        processed_points = set()
        for p in self.all_points_data:
            if p in processed_points:
                continue
            equivalent_class = get_equivalent_class(p)
            processed_points |= equivalent_class

            if len(equivalent_class) <= 1:
                continue

            constraint = NamedSketcher.ConstraintCoincident()
            for a in equivalent_class:
                constraint.addPoint(a.ref)
            sketch.addConstraint(constraint)

    def generate_horizontal_constraints(self, sketch):
        for g in self.geometry_refs:
            if is_horizontal(g, self.tolerance):
                sketch.addConstraint(NamedCurve.ConstraintHorizontal(g.ref))

    def generate_vertical_constraints(self, sketch):
        for g in self.geometry_refs:
            if is_vertical(g, self.tolerance):
                sketch.addConstraint(NamedCurve.ConstraintVertical(g))

    def generate_point_over_curve_constraints(self, sketch):
        for g1, g2 in permutations(self.geometry_refs):
            g1_pts = g1.getReferences_Point()
            for p in g1_pts:
                is_point_over_curve(p, g2, self.tolerance):
                    sketch.addConstraint(NamedCurve.ConstraintPointOverCurve(p, g2))

    def generate_tangent_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometry_refs, 2):
            if are_curves_tangent(g1, g2, self.tolerance):
                sketch.addConstraint(NamedCurve.ConstraintTangent(g1, g2))

    def generate_normal_curve_constraints(self, sketch):
        for g1, g2 in combinations(self.geometry_refs, 2):
            if are_curves_normal(g1, g2, self.tolerance):
                sketch.addConstraint(NamedCurve.ConstraintNormal(g1, g2))

    def generate_circle_radius_constraints(self, sketch):
        for g in self.geometry_refs:
            if is_radius_underconstrained(g, self.tolerance):
                sketch.addConstraint(NamedCurve.ConstraintConstant(g + "radius"))


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
        for p_ref in [ref + p for p in obj.getReferencedPoints()]:
            p_obj = NamedSketcher.resolveReferenceToPoint(p_ref)
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

def are_equal(a, b, tolerance):
    return abs(a - b) < tolerance

def are_coincident(p1, p2, tolerance):
    return are_equal(p1.x, p2.x, tolerance) && are_equal(p1.x, p2.x, tolerance)

def is_horizontal(geo, tolerance):
    try:
        start = NamedSketcher.getReferencedPoint(geo + "start")
        end = NamedSketcher.getReferencedPoint(geo + "end")
    except NamedSketcher.ExceptionUnresolvedPath:
        return false
    return are_equal(start.y, end.y, tolerance)

def is_vertical(geo, tolerance):
    try:
        start = NamedSketcher.getReferencedPoint(geo + "start")
        end = NamedSketcher.getReferencedPoint(geo + "end")
    except NamedSketcher.ExceptionUnresolvedPath:
        return false
    return are_equal(start.x, end.x, tolerance)

def is_point_over_curve(p, geo, tolerance):
    return false

def are_curves_tangent(geo1, geo2, tolerance):
    return false

def are_curves_normal(geo1, geo2, tolerance):
    return false

def is_radius_underconstrained(geo, tolerance):
    try:
        radius = NamedSketcher.getReferencedPoint(geo + "radius")
    except NamedSketcher.ExceptionUnresolvedPath:
        return false
    return true # TODO: implement underconstrainment check.

