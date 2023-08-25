import FreeCAD, FreeCADGui
from pivy import coin

class ViewProviderNamedSketch:
    def __init__(self, obj):
        obj.Proxy = self

    def attach(self, obj):
        self.wireframe = coin.SoGroup()
        #data=coin.lines, points circles
        style=coin.SoDrawStyle()
        style.style = coin.SoDrawStyle.LINES
        self.wireframe.addChild(style)
        #self.wireframe.addChild(data)
        obj.addDisplayMode(self.wireframe, "Wireframe");

    def updateData(self, fp, prop):
        pass

    def getDisplayModes(self,obj):
        modes=[]
        modes.append("Wireframe")
        return modes

    def getDefaultDisplayMode(self):
        return "Wireframe"

    def onChanged(self, vp, prop):
        pass
