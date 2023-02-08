# ***************************************************************************
# *   Copyright (c) 2009, 2010 Yorik van Havre <yorik@uncreated.net>        *
# *   Copyright (c) 2009, 2010 Ken Cline <cline@frii.com>                   *
# *   Copyright (c) 2018 Benjamin Alterauge (ageeye)                        *
# *   Copyright (c) 2020 Carlo Pavan <carlopav@gmail.com>                   *
# *   Copyright (c) 2020 Eliud Cabrera Castillo <e.cabrera-castillo@tum.de> *
# *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
# *                                                                         *
# *   This file is part of the FreeCAD CAx development system.              *
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
"""Provides functions to create SnapArray objects.

The copies will be created according to a snap map.
"""
## @package make_snaparray
# \ingroup draftmake
# \brief Provides functions to create SnapArray objects.

## \addtogroup draftmake
# @{
import FreeCAD as App
import draftutils.utils as utils
import draftutils.gui_utils as gui_utils

from draftutils.messages import _msg, _err
from draftutils.translate import translate
from draftobjects.snaparray import SnapArray

if App.GuiUp:
    from draftviewproviders.view_array import ViewProviderDraftArray
    from draftviewproviders.view_draftlink import ViewProviderDraftLink


def make_snap_array(base_object, snap_map, extra=None, use_link=True):
    """Make a Draft SnapArray object.

    Create copies of a `base_object` at the lines defined by
    a `snap_map`.

    Parameters
    ----------
    base_object: Part::Feature or str
        Any of object that has a `Part::TopoShape` that can be duplicated.
        This means most 2D and 3D objects produced with any workbench.
        If it is a string, it must be the `Label` of that object.
        Since a label is not guaranteed to be unique in a document,
        it will use the first object found with this label.

    snap_map: Sketcher::SketchObject or str
        An object containing snapping points and lines.

    extra: Base::Placement, Base::Vector3, or Base::Rotation, optional
        It defaults to `None`.
        If it is provided, it is an additional placement that is applied
        to each copy of the array.
        The input could be a full placement, just a vector indicating
        the additional translation, or just a rotation.

    Returns
    -------
    Part::FeaturePython
        A scripted object of type `'SnapArray'`.
        Its `Shape` is a compound of the copies of the original object.

    None
        If there is a problem it will return `None`.
    """
    _name = "make_snap_array"
    utils.print_header(_name, "Snap array")

    found, doc = utils.find_doc(App.activeDocument())
    if not found:
        _err(translate("draft", "No active document. Aborting."))
        return None

    if isinstance(base_object, str):
        base_object_str = base_object

    found, base_object = utils.find_object(base_object, doc)
    if not found:
        # This is odd... can't base_object be converted to string???
        # When not found, what is base_object?
        # This error message only works when isinstance(base_object, str)!
        # Otherwise, base_object_str is not defined.
        # What if base_object is not a string and is not in doc?
        _msg("base_object: {}".format(base_object_str))
        _err(translate("draft", "Wrong input: object not in document."))
        return None

    _msg("base_object: {}".format(base_object.Label))

    if isinstance(snap_map, str):
        snap_map_str = snap_map

    found, snap_map = utils.find_object(snap_map, doc)
    if not found:
        # The same as above.
        _msg("snap_map: {}".format(snap_map_str))
        _err(translate("draft", "Wrong input: object not in document."))
        return None

    _msg("snap_map: {}".format(snap_map.Label))
    if snap_map.TypeId != "Sketcher::SketchObject":
        _err(translate("draft", "Wrong input: object has the wrong type."))
        return None

    _msg("extra: {}".format(extra))
    if not extra:
        extra = App.Placement()
    try:
        utils.type_check([(extra, (App.Placement,
                                   App.Vector,
                                   App.Rotation))],
                         name=_name)
    except TypeError:
        _err(translate("draft", "Wrong input: must be a placement, a vector, or a rotation."))
        return None

    # Convert the vector or rotation to a full placement
    if isinstance(extra, App.Vector):
        extra = App.Placement(extra, App.Rotation())
    elif isinstance(extra, App.Rotation):
        extra = App.Placement(App.Vector(), extra)

    if use_link:
        # The SnapArray class must be called in this special way
        # to make it a LinkArray
        new_obj = doc.addObject("Part::FeaturePython", "SnapArray",
                                SnapArray(None), None, True)
    else:
        new_obj = doc.addObject("Part::FeaturePython", "SnapArray")
        SnapArray(new_obj)

    new_obj.Base = base_object
    new_obj.SnapMapObject = snap_map
    new_obj.ExtraPlacement = extra

    if App.GuiUp:
        if use_link:
            ViewProviderDraftLink(new_obj.ViewObject)
        else:
            ViewProviderDraftArray(new_obj.ViewObject)
            gui_utils.format_object(new_obj, new_obj.Base)

            if hasattr(new_obj.Base.ViewObject, "DiffuseColor"):
                if len(new_obj.Base.ViewObject.DiffuseColor) > 1:
                    new_obj.ViewObject.Proxy.resetColors(new_obj.ViewObject)

        new_obj.Base.ViewObject.hide()
        gui_utils.select(new_obj)

    return new_obj


def makeSnapArray(base, ptlst):
    """Create SnapArray. DEPRECATED. Use 'make_snap_array'."""
    utils.use_instead('make_snap_array')

    return make_snap_array(base, ptlst)

## @}
