#qt_add_resources(NamedSketcherGui_SRCS Resources/NamedSketcher.qrc)

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
    ConstraintCreation/ConstraintEqual.py
    ConstraintCreation/ConstraintConstant.py
    ConstraintCreation/ConstraintCoincident.py
    ConstraintCreation/ConstraintHorizontal.py
    ConstraintCreation/ConstraintVertical.py
    ConstraintCreation/ConstraintBlockPoint.py
    ConstraintCreation/ConstraintPointSymmetric.py
    ConstraintCreation/ConstraintXDistance.py
    ConstraintCreation/ConstraintYDistance.py
    ConstraintCreation/ConstraintPointAlongCurve.py
    ConstraintCreation/ConstraintTangentCurves.py
)

SET(NamedSketcherGuiIcon_SVG
    Resources/icons/NamedSketcherWorkbench.svg
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
