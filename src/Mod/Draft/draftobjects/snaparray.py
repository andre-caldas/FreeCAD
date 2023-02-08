# ***************************************************************************
# *   Copyright (c) 2009, 2010 Yorik van Havre <yorik@uncreated.net>        *
# *   Copyright (c) 2009, 2010 Ken Cline <cline@frii.com>                   *
# *   Copyright (c) 2018 Benjamin Alterauge (ageeye)                        *
# *   Copyright (c) 2020 Carlo Pavan <carlopav@gmail.com>                   *
# *   Copyright (c) 2020 Eliud Cabrera Castillo <e.cabrera-castillo@tum.de> *
# *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU Lesser General Public License (LGPL)    *
# *   as published by the Free Software Foundation; either version 2 of     *
# *   the License, or (at your option) any later version.                   *
# *   for detail see the LICENCE text file.                                 *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this program; if not, write to the Free Software   *
# *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
# *   USA                                                                   *
# *                                                                         *
# ***************************************************************************
"""Provides the object code for the SnapArray object.
"""
## @package snaparray
# \ingroup draftobjects
# \brief Provides the object code for the SnapArray object.

from PySide.QtCore import QT_TRANSLATE_NOOP

import FreeCAD as App
import DraftVecUtils
import draftutils.utils as utils

from draftutils.messages import _wrn, _err
from draftutils.translate import translate
from draftobjects.draftlink import DraftLink

## \addtogroup draftobjects
# @{


class SnapArray(DraftLink):
    """The Draft Snap Array object."""

    def __init__(self, obj):
        super(SnapArray, self).__init__(obj, "SnapArray")

    def attach(self, obj):
        """Set up the properties when the object is attached."""
        self.set_properties(obj)
        super(SnapArray, self).attach(obj)

    def linkSetup(self, obj):
        """Set up the object as a link object."""
        super(SnapArray, self).linkSetup(obj)
        obj.configLinkProperty(ElementCount='Count')

    def set_properties(self, obj):
        """Set properties only if they don't exist."""
        properties = obj.PropertiesList
# snap_mode = rigid, loose, rescale

        if "Base" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Base object that will be duplicated")
            obj.addProperty("App::PropertyLink",
                            "Base",
                            "Objects",
                            _tip)
            obj.Base = None

        if "SnapMapObject" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Object containing points used to distribute the copies.")
            obj.addProperty("App::PropertyLink",
                            "SnapMapObject",
                            "Objects",
                            _tip)
            obj.SnapMapObject = None

        if "Count" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Number of copies in the array.\nThis property is read-only, as the number depends on the points in 'Point Object'.")
            obj.addProperty("App::PropertyInteger",
                            "Count",
                            "Objects",
                            _tip)
            obj.Count = 0
            obj.setEditorMode("Count", 1)  # Read only

        if "ExtraPlacement" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Additional placement, shift and rotation, that will be applied to each copy")
            obj.addProperty("App::PropertyPlacement",
                            "ExtraPlacement",
                            "Objects",
                            _tip)
            obj.ExtraPlacement = App.Placement()

        if self.use_link and "ExpandArray" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Show the individual array elements (only for Link arrays)")
            obj.addProperty("App::PropertyBool",
                            "ExpandArray",
                            "Objects",
                            _tip)
            obj.setPropertyStatus('Shape', 'Transient')

        if "AnchorMap" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Map with anchors to snap into each snap map element")
            obj.addProperty("App::PropertyLink",
                            "AnchorMap",
                            "Objects",
                            _tip)

        if "AnchorNumbers" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Wich line in Anchor shall we use?'.")
            obj.addProperty("App::PropertyIntegerList",
                            "AnchorNumbers",
                            "Objects",
                            _tip)

    def execute(self, obj):
        """Run when the object is created or recomputed."""
        if self.props_changed_placement_only() \
                or not obj.Base \
                or not obj.SnapMapObject:
            self.props_changed_clear()
            return

        matrix_list = get_matrix_list(obj.SnapMapObject)
        anchor_map = obj.AnchorMap
        if hasattr(anchor_map, 'getLinkedObject'):
            anchor_map = anchor_map.getLinkedObject()
        anchor_matrix_list = get_anchor_matrix_list(anchor_map, obj.AnchorNumbers)
        obj.Count = len(matrix_list)
        pls = build_placements(obj.Base, matrix_list, anchor_matrix_list, obj.ExtraPlacement)

        self.buildShape(obj, obj.Placement, pls)
        self.props_changed_clear()
        return (not self.use_link)

    def onDocumentRestored(self, obj):
        """Execute code when the document is restored.

        Add properties that don't exist and migrate old properties.
        """
        # If the ExtraPlacement property has never been added before
        # it will add it first, and set it to the base object's position
        # in order to produce the same displacement as before.
        # Then all the other properties will be processed.
        properties = obj.PropertiesList

        if "ExtraPlacement" not in properties:
            _tip = QT_TRANSLATE_NOOP("App::Property", "Additional placement, shift and rotation, that will be applied to each copy")
            obj.addProperty("App::PropertyPlacement",
                            "ExtraPlacement",
                            "Objects",
                            _tip)
            obj.ExtraPlacement.Base = obj.Base.Placement.Base
            _wrn("v0.19, " + obj.Label + ", " + translate("draft","added property 'ExtraPlacement'"))

        self.set_properties(obj)
        super(SnapArray, self).onDocumentRestored(obj)


def remove_equal_vecs (vec_list):
    """Remove equal vectors from a list.

    Parameters
    ----------
    vec_list: list of App.Vectors

    Returns
    -------
    list of App.Vectors
    """
    res_list = []
    for vec in vec_list:
        for res in res_list:
            if DraftVecUtils.equals(vec, res):
                break
        else:
            res_list.append(vec)
    return res_list

def get_matrix_list(snap_map):
    """Extract a list of placement matrices from a Sketch object.

    Parameters
    ----------
    snap_map: Sketcher::SketchObject

    Returns
    -------
    list of pairs of App.Matrix
    """
    place = snap_map.Placement
    # I'd rather test for instanceof(...), but... there must be a reason!
    matrix_list = []
    for i,geo in enumerate(snap_map.Geometry):
        if snap_map.getConstruction(i):
            continue
        if geo.TypeId == "Part::GeomLineSegment":
            matrix_list.append(matrix_from_two_points(place, geo.StartPoint, geo.EndPoint))

    pt_list = [v.Point for i,v in enumerate(snap_map.Geometry) if not snap_map.getConstruction(i) and v.TypeId == "Part::GeomPoint"]
    pt_list = remove_equal_vecs(pt_list)
    for pt in pt_list:
        matrix_list.append(matrix_from_two_points(place, pt, pt + App.Vector(1,0,0)))
    return matrix_list

def get_anchor_matrix_list(anchor_map, anchor_numbers):
    """Extract a list of placement matrices from a Sketch 'AnchorMap' object.

    Parameters
    ----------
    anchor_map: Sketcher::SketchObject

    anchor_numbers: list of integers that reference to anchor_map.Geometry objects.

    Returns
    -------
    list of pairs of (App.Vector, float)
    """
    if not anchor_map:
        return []

    place = anchor_map.Placement
    anchor_matrices = []
    for i,geo in enumerate(anchor_map.Geometry):
        if anchor_map.getConstruction(i):
            continue
        # I'd rather test for instanceof(...), but... there must be a reason!
        if geo.TypeId == "Part::GeomLineSegment":
            anchor_matrices.append(matrix_from_two_points(place, geo.StartPoint, geo.EndPoint).inverse())
        elif geo.TypeId == "Part::GeomPoint":
            anchor_matrices.append(matrix_from_two_points(place, geo.Point, geo.Point + App.Vector(1,0,0)).inverse)
        else:
            _wrn('Anchor type "{}" not supported.'.format(geo.TypeId))
            anchor_matrices.append(App.Matrix())

    matrix_list = []
    for i in anchor_numbers:
        try:
            matrix_list.append(anchor_matrices[i])
        except IndexError:
            _wrn('Anchor index "{}" not found.'.format(i))
            matrix_list.append(App.Matrix())
    return matrix_list


def matrix_from_two_points(place, p, q):
        h = q - p
        v = App.Vector(-h.y, h.x, 0)
        p0 = place.multVec(p)
        eh = (place.multVec(q) - p0).normalize()
        ev = (place.multVec(p + v) - p0).normalize()
        ez = (place.multVec(p + App.Vector(0,0,1)) - p0).normalize()
        matrix = App.Matrix(eh, ev, ez, p0)
        return matrix

def build_placements(base_object, matrix_list, anchor_matrices, placement=App.Placement()):
    """Build a placements from the base object and list of places relative to Sketch.

    Returns
    -------
    list of App.Placements
    """
    pls = list()

    for i,matrix in enumerate(matrix_list):
        place = App.Placement()
        place.Matrix = placement.Matrix * matrix * base_object.Placement.Matrix
        try:
          place.Matrix = place.Matrix * anchor_matrices[i]
        except IndexError:
            _wrn('Anchor for SnapArray element number "{}" not found.'.format(i))
        pls.append(place)

    return pls


# Alias for compatibility with v0.18 and earlier
_SnapArray = SnapArray

## @}
