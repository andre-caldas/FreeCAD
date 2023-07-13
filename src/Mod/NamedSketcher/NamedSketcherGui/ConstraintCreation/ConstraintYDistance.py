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

command = "NamedSketcherGui_ConstraintCreation_YDistance"

class YDistance:
    def GetResources(self):
        return {'Pixmap'  : command,
                'MenuText': 'Vertical distance',
                'ToolTip' :
                    '''Specifies the difference of the y-coordinate of two points.
                    Instead of two points, you can also specify anything that has a start and an end point.
                    For example, a line.'''}

    def IsActive(self):
        return True;

    def Activate(self):
        pass

FreeCADGui.addCommand(command, YDistance());
