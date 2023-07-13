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

command = "NamedSketcherGui_ConstraintCreation_PointSymmetric"

class PointSymmetric:
    def GetResources(self):
        return {'Pixmap'  : command,
                'MenuText': 'Point symmetric',
                'ToolTip' : 'Keeps a certain point always in the middle of other two choosen points'}

    def IsActive(self):
        return True;

    def Activate(self):
        pass

FreeCADGui.addCommand(command, PointSymmetric());
