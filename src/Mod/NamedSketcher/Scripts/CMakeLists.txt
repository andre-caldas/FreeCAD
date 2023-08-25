set(StartScripts
    Init.py
#    NamedSketcherExample.py
#    TestNamedSketcherApp.py
#    Profiles.py
)

set(TestScripts
#     NamedSketcherTests/__init__.py
#     NamedSketcherTests/TestSketchFillet.py
#     NamedSketcherTests/TestNamedSketcherSolver.py
#     NamedSketcherTests/TestSketchExpression.py
)

if(BUILD_GUI)
    list(APPEND StartScripts
          InitGui.py
#          TestNamedSketcherGui.py
    )
endif(BUILD_GUI)

set(Profiles
#     ProfileLib/Hexagon.py
#     ProfileLib/RegularPolygon.py
#     ProfileLib/__init__.py
)

set(PythonScripts
    NamedSketcher/__init__.py
    NamedSketcher/utils/__init__.py
    NamedSketcher/utils/Tolerance.py
    NamedSketcher/utils/Draft2NamedSketch.py
)
source_group("Python Scripts" FILES ${PythonScripts})

set(GuiCommands
    NamedSketcherGui/__init__.py
    #
    NamedSketcherGui/ViewProviders/NamedSketch.py
    #
    NamedSketcherGui/GeometryCreation/__init__.py
    NamedSketcherGui/GeometryCreation/FromDraft.py
    NamedSketcherGui/GeometryCreation/Point.py
    NamedSketcherGui/GeometryCreation/LineSegment.py
    NamedSketcherGui/GeometryCreation/Circle.py
    #
    NamedSketcherGui/ConstraintCreation/__init__.py
    NamedSketcherGui/ConstraintCreation/Coincident.py
    NamedSketcherGui/ConstraintCreation/Horizontal.py
    NamedSketcherGui/ConstraintCreation/Vertical.py
    NamedSketcherGui/ConstraintCreation/BlockPoint.py
    NamedSketcherGui/ConstraintCreation/PointSymmetric.py
    NamedSketcherGui/ConstraintCreation/LineSymmetric.py
    NamedSketcherGui/ConstraintCreation/Distance.py
    NamedSketcherGui/ConstraintCreation/XDistance.py
    NamedSketcherGui/ConstraintCreation/YDistance.py
    NamedSketcherGui/ConstraintCreation/PointAlongCurve.py
    NamedSketcherGui/ConstraintCreation/TangentCurves.py
    #
    NamedSketcherGui/Draw/__init__.py
    #
    NamedSketcherGui/Draw/DrawMode/__init__.py
)
source_group("GUI Commands" FILES ${GuiCommands})


set(AllScripts
    ${StartScripts}
    ${TestScripts}
    ${Profiles}
    ${PythonScripts}
    ${GuiCommands}
)

add_custom_target(NamedSketcherScripts ALL
    SOURCES
    ${AllScripts}
)

fc_target_copy_resource(NamedSketcherScripts
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/Mod/NamedSketcher
    ${AllScripts}
)

foreach (file ${AllScripts})
    get_filename_component(dir ${file} DIRECTORY)
    install(
        FILES
        ${file}
        DESTINATION
        "${CMAKE_BINARY_DIR}/Mod/NamedSketcher/${dir}"
    )
endforeach()
