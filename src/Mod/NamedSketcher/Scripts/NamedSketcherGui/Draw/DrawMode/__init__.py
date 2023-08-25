## SPDX-License-Identifier: LGPL-2.1-or-later
#****************************************************************************
#*                                                                          *
#*   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
#*                                                                          *
#*   This file is part of FreeCAD.                                          *
#*                                                                          *
#*   FreeCAD is free software: you can redistribute it and/or modify it     *
#*   under the terms of the GNU Lesser General Public License as            *
#*   published by the Free Software Foundation, either version 2.1 of the   *
#*   License, or (at your option) any later version.                        *
#*                                                                          *
#*   FreeCAD is distributed in the hope that it will be useful, but         *
#*   WITHOUT ANY WARRANTY; without even the implied warranty of             *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
#*   Lesser General Public License for more details.                        *
#*                                                                          *
#*   You should have received a copy of the GNU Lesser General Public       *
#*   License along with FreeCAD. If not, see                                *
#*   <https://www.gnu.org/licenses/>.                                       *
#*                                                                          *
#***************************************************************************/

import FreeCADGui

command = "NamedSketcherGui/ConstraintCreation/DrawMode"

class DrawMode:
    def GetResources(self):
        return {'Pixmap'  : ':/' + command,
                'Accel'   : 'Ctrl+Shift+d',
                'MenuText': 'Draw mode',
                'ToolTip' : 'Efficient way to specify basic geometries and basic constraints with ease.'}

    def __init__(self):
        import LineController
        import NoDrawController
        # Button becomes gray and accelerator does not work when not clickable.
        self.is_clickable = True
        # List of points objects used.
        self.points = []
        # Controllers are passed an index to self.points,
        # so we can change the controller on the fly.
        self.current_point_index
        # Index of points that still need to be visited by a geometry.
        # See the "T" command or the "(S)plit" command.
        self.points_left_behind = []
        self.next_controller_pkg = LineController
        self.geometry_controller = NoDrawController.Controller(self)

    def IsActive(self):
        print('Draw mode!!!')
        return self.is_clickable

    def Activated(self):
        self.is_clickable = False
        FreeCADGui.

    def processKeys(self, info):
        pass

command_instance = DrawMode()
FreeCADGui.addCommand(command, command_instance)
