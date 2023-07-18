set(NamedSketcherGui_UIC_SRCS
#    TaskNamedSketcherSolverAdvanced.ui
#    InsertDatum.ui
#    NamedSketcherSettings.ui
#    NamedSketcherSettingsGrid.ui
#    NamedSketcherSettingsColors.ui
#    NamedSketcherSettingsDisplay.ui
)

SET(NamedSketcherGui_SRCS
    __init__.py
    #
    GeometryCreation/__init__.py
    GeometryCreation/FromDraft.py
    GeometryCreation/Point.py
    GeometryCreation/LineSegment.py
    GeometryCreation/Circle.py
    #
    ConstraintCreation/__init__.py
    ConstraintCreation/Coincident.py
    ConstraintCreation/Horizontal.py
    ConstraintCreation/Vertical.py
    ConstraintCreation/BlockPoint.py
    ConstraintCreation/PointSymmetric.py
    ConstraintCreation/LineSymmetric.py
    ConstraintCreation/Distance.py
    ConstraintCreation/XDistance.py
    ConstraintCreation/YDistance.py
    ConstraintCreation/PointAlongCurve.py
    ConstraintCreation/TangentCurves.py
)

SET(NamedSketcherGuiIcon_SVG
    NamedSketcherWorkbench.svg
)

add_custom_target(NamedSketcherGui ALL
    SOURCES
        ${NamedSketcherGui_SRCS}
        ${NamedSketcherGuiIcon_SVG}
)

fc_target_copy_resource(NamedSketcherGui
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/Mod/NamedSketcher/NamedSketcherGui
    ${NamedSketcherGui_SRCS}
    ${NamedSketcherGuiIcon_SVG}
)

set(NamedSketcherResources_SRCS
    NamedSketcherResourcesLoader.cpp
#    xxxx.ui
)
qt_add_resources(NamedSketcherResources_SRCS NamedSketcherResources.qrc)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${XercesC_INCLUDE_DIRS}
)

add_library(NamedSketcherResources SHARED ${NamedSketcherResources_SRCS})
target_link_libraries(NamedSketcherResources FreeCADGui)

SET_BIN_DIR(NamedSketcherGui NamedSketcherGui /Mod/NamedSketcher)
SET_BIN_DIR(NamedSketcherResources NamedSketcherResources /Mod/NamedSketcher)
SET_PYTHON_PREFIX_SUFFIX(NamedSketcherResources)

INSTALL(TARGETS NamedSketcherResources DESTINATION ${CMAKE_INSTALL_LIBDIR})
#INSTALL(FILES ${NamedSketcherGuiIcon_SVG} DESTINATION "${CMAKE_INSTALL_DATADIR}/Mod/Sketcher/Resources/icons")
