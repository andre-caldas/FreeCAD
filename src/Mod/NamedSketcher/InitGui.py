#***************************************************************************
#*   Copyright (c) 2002,2003 Juergen Riegel <juergen.riegel@web.de>        *
#*                                                                         *
#*   This file is part of the FreeCAD CAx development system.              *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU Lesser General Public License (LGPL)    *
#*   as published by the Free Software Foundation; either version 2 of     *
#*   the License, or (at your option) any later version.                   *
#*   for detail see the LICENCE text file.                                 *
#*                                                                         *
#*   FreeCAD is distributed in the hope that it will be useful,            *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU Lesser General Public License for more details.                   *
#*                                                                         *
#*   You should have received a copy of the GNU Library General Public     *
#*   License along with FreeCAD; if not, write to the Free Software        *
#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
#*   USA                                                                   *
#*                                                                         *
#***************************************************************************/

# NamedSketcher gui init module
#
# Gathering all the information to start FreeCAD
# This is the second one of three init scripts, the third one
# runs when the gui is up

class NamedSketcherWorkbench ( Workbench ):
    "NamedSketcher workbench object"
    def __init__(self):
        self.__class__.Icon = FreeCAD.getResourceDir() + "Mod/NamedSketcher/Resources/icons/NamedSketcherWorkbench.svg"
        self.__class__.MenuText = "Named sketcher"
        self.__class__.ToolTip = "Named sketcher workbench"

    def Initialize(self):
        # load the module
        import NamedSketcherGui

        try:
            import Profiles
        except ImportError:
            print("Error in Profiles module")

        NamedSketcherGui.generateToolbars(self)
        NamedSketcherGui.generateMenus(self)

    def Activated(self):
        """This function is executed whenever the workbench is activated"""
        return

    def Deactivated(self):
        """This function is executed whenever the workbench is deactivated"""
        return

    def ContextMenu(self, recipient):
        """This function is executed whenever the user right-clicks on screen"""
        # "recipient" will be either "view" or "tree"
        #self.appendContextMenu("My commands", self.list) # add commands to the context menu

    def GetClassName(self):
        #return "NamedSketcherGui::Workbench"
        return "Gui::PythonWorkbench"

Gui.addWorkbench(NamedSketcherWorkbench())

FreeCAD.__unit_test__ += [ "TestNamedSketcherGui" ]
