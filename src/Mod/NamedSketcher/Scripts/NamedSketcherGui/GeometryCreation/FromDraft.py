## SPDX-License-Identifier: LGPL-2.1-or-later
# ***************************************************************************
# *                                                                         *
# *  Copyright (c) 2009, 2010 Yorik van Havre <yorik@uncreated.net>         *
# *  Copyright (c) 2009, 2010 Ken Cline <cline@frii.com>                    *
# *  Copyright (c) 2020 Eliud Cabrera Castillo <e.cabrera-castillo@tum.de>  *
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
"""Provides GUI tools to convert Draft objects to NamedSketches.
"""

import FreeCADGui
import Draft_rc
import draftguitools.gui_base_original as gui_base_original
import draftguitools.gui_tool_utils as gui_tool_utils
from draftutils.messages import _msg
from draftutils.translate import translate

# The module is used to prevent complaints from code checkers (flake8)
True if Draft_rc.__name__ else False


command = "NamedSketcherGui/GeometryCreation/FromDraft"

class FromDraft(gui_base_original.Modifier):
    """Gui Command for the FromDraft tool."""
    def GetResources(self):
        """Set icon, menu and tooltip."""
        return {'Pixmap'  : ':/' + command,
                'MenuText': 'From draft',
                'ToolTip' : 'Create geometries from a draft object'}

    def Activated(self):
        """Execute when the command is called."""
        super(FromDraft, self).Activated(name="Convert Draft/Sketch")
        if not FreeCADGui.Selection.getSelection():
            if self.ui:
                self.ui.selectUi(on_close_call=self.finish)
                _msg(translate("draft", "Select an object to convert."))
                self.call = self.view.addEventCallback(
                    "SoEvent",
                    gui_tool_utils.selectObject)
        else:
            self.proceed()

    def proceed(self):
        """Proceed with the command if one object was selected."""

        sel = FreeCADGui.Selection.getSelection()
        if not sel:
            return

        FreeCADGui.addModule("NamedSketcher.utils.Draft2NamedSketch")

        _cmd = "NamedSketcher.utils.Draft2NamedSketch.Draft2NamedSketch"
        _cmd += "("
        _cmd += "FreeCADGui.Selection.getSelection(), "
        _cmd += ").generate_sketch()"
        _cmd_list = ['sk = ' + _cmd,
                     'sk.addToCurrentDocument("Draft2NamedSketch_generated")',
                     'FreeCAD.ActiveDocument.recompute()']
        self.commit(translate("draft", "Convert to Sketch"), _cmd_list)

        self.finish()


FreeCADGui.addCommand(command, FromDraft());
