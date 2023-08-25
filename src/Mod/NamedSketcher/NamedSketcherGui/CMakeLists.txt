set(NamedSketcherGui_UIC_SRCS
#    TaskNamedSketcherSolverAdvanced.ui
#    InsertDatum.ui
#    NamedSketcherSettings.ui
#    NamedSketcherSettingsGrid.ui
#    NamedSketcherSettingsColors.ui
#    NamedSketcherSettingsDisplay.ui
)
source_group("UI" FILES ${NamedSketcherGui_UIC_SRCS})

set(NamedSketcherViewProviders_SRCS
    ViewProviders/ViewProviderNamedSketch.h
    ViewProviders/ViewProviderNamedSketch.cpp
)
source_group("View providers" FILES ${NamedSketcherViewProviders_SRCS})

set(NamedSketcherGui_SRCS
    ${NamedSketcherGui_UIC_SRCS}
    ${NamedSketcherViewProviders_SRCS}
)

set(NamedSketcherGuiIcon_SVG
    NamedSketcherWorkbench.svg
)

set(NamedSketcherResources_SRCS
    NamedSketcherResources.pybind.cpp
#    xxxx.ui
)
source_group("Resource loader" FILES ${NamedSketcherResources_SRCS})
qt_add_resources(NamedSketcherResources_SRCS NamedSketcherResources.qrc)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${OCC_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${XercesC_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIR}
)

set(LIBS
    NamedSketcherCpp
    PartGui
    FreeCADGui
)

# Cargo cult... I have no idea why I have to put the SVG here...
# Probably I should just explicitly make NamedSketcherGuiCpp depend on the SVG.
add_library(NamedSketcherGuiCpp SHARED ${NamedSketcherGui_SRCS} ${NamedSketcherGuiIcon_SVG})
target_link_libraries(NamedSketcherGuiCpp ${LIBS})

add_library(NamedSketcherResources SHARED ${NamedSketcherResources_SRCS})
target_link_libraries(NamedSketcherResources NamedSketcherGuiCpp ${LIBS})

SET_BIN_DIR(NamedSketcherGuiCpp NamedSketcherGuiCpp /Mod/NamedSketcher)
SET_BIN_DIR(NamedSketcherResources NamedSketcherResources /Mod/NamedSketcher)
SET_PYTHON_PREFIX_SUFFIX(NamedSketcherGuiCpp)
SET_PYTHON_PREFIX_SUFFIX(NamedSketcherResources)

fc_copy_sources(NamedSketcherGuiCpp
    "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/Mod/NamedSketcher"
    ${NamedSketcherGuiIcon_SVG}
)

install(TARGETS NamedSketcherResources DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${NamedSketcherGuiIcon_SVG} DESTINATION ${CMAKE_INSTALL_DATADIR}/Mod/NamedSketcher)
