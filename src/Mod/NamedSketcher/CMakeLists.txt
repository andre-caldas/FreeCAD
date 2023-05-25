
add_subdirectory(App)
if(BUILD_GUI)
    add_subdirectory(Gui)
endif(BUILD_GUI)

set(NamedSketcher_Scripts
    Init.py
#    NamedSketcherExample.py
#    TestNamedSketcherApp.py
#    Profiles.py
)

set(NamedSketcher_TestScripts
#     NamedSketcherTests/__init__.py
#     NamedSketcherTests/TestSketchFillet.py
#     NamedSketcherTests/TestNamedSketcherSolver.py
#     NamedSketcherTests/TestSketchExpression.py
)

if(BUILD_GUI)
    list (APPEND NamedSketcher_Scripts
          InitGui.py
#          TestNamedSketcherGui.py
    )
endif(BUILD_GUI)

set(NamedSketcher_Profiles
#     ProfileLib/Hexagon.py
#     ProfileLib/RegularPolygon.py
#     ProfileLib/__init__.py
)

add_custom_target(NamedSketcherScripts ALL
    SOURCES ${NamedSketcher_Scripts} ${NamedSketcher_Profiles} ${NamedSketcher_TestScripts}
)

fc_target_copy_resource(NamedSketcherScripts
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/Mod/NamedSketcher
    ${NamedSketcher_Scripts}
    ${NamedSketcher_Profiles}
    ${NamedSketcher_TestScripts}
)

INSTALL(
    FILES
        ${NamedSketcher_Scripts}
    DESTINATION
        Mod/NamedSketcher
)

INSTALL(
    FILES
        ${NamedSketcher_Profiles}
    DESTINATION
        Mod/NamedSketcher/ProfileLib
)

INSTALL(
    FILES
        ${NamedSketcher_TestScripts}
    DESTINATION
        Mod/NamedSketcher/NamedSketcherTests
)
