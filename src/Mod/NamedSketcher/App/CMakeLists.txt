if(MSVC)
    add_definitions(-DFCAppNamedSketcher -DHAVE_ACOSH -DHAVE_ASINH -DHAVE_ATANH)
else(MSVC)
    add_definitions(-DHAVE_LIMITS_H -DHAVE_CONFIG_H)
endif(MSVC)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${Boost_INCLUDE_DIRS}
    ${OCC_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)
link_directories(${OCC_LIBRARY_DIR})

set(NamedSketcher_LIBS
    Part
    FreeCADApp
)

generate_from_xml(NamedSketchPy)
generate_from_xml(geometries/GeometryBasePy)
generate_from_xml(geometries/GeometryPointPy)
generate_from_xml(geometries/GeometryLineSegmentPy)
generate_from_xml(constraints/ConstraintBasePy)
generate_from_xml(constraints/ConstraintConcidentPy)


SET(Properties_SRCS
    constraints/PropertyConstraintList.h
    constraints/PropertyConstraintList.cpp
    geometries/PropertyGeometryList.h
    geometries/PropertyGeometryList.cpp
)
SOURCE_GROUP("Properties" FILES ${Properties_SRCS})


SET(Features_SRCS
    NamedSketch.cpp
    NamedSketch.h
#     PythonConverter.cpp
#     PythonConverter.h
)
SOURCE_GROUP("Features" FILES ${Features_SRCS})


SET(Python_SRCS
    NamedSketchPy.xml
#    NamedSketchPyImp.cpp
    geometries/GeometryBasePy.xml
#    geometries/GeometryBasePyImp.cpp
    geometries/GeometryPointPy.xml
#    geometries/GeometryPointPyImp.cpp
    geometries/GeometryLineSegmentPy.xml
#    geometries/GeometryLineSegmentPyImp.cpp
    constraints/ConstraintBasePy.xml
#    constraints/ConstraintBasePyImp.cpp
    constraints/ConstraintCoincidentPy.xml
#    constraints/ConstraintCoincidentPyImp.cpp
)
SOURCE_GROUP("Python" FILES ${Python_SRCS})

SET(Geometries_SRCS
    geometries/GeometryBase.h
    geometries/GeometryBase.cpp
    geometries/GeometryFactory.h
    geometries/GeometryFactory.cpp
    geometries/GeometryPoint.h
    geometries/GeometryPoint.cpp
    geometries/GeometryLineSegment.h
    geometries/GeometryLineSegment.cpp
)
SOURCE_GROUP("Geometries" FILES ${Geometries_SRCS})

SET(Constraints_SRCS
    constraints/ConstraintBase.h
    constraints/ConstraintBase.cpp
    constraints/ConstraintFactory.h
    constraints/ConstraintFactory.cpp
    constraints/ConstraintCoincident.h
    constraints/ConstraintCoincident.cpp
    constraints/ConstraintEqual.h
    constraints/ConstraintEqual.cpp
    constraints/ConstraintHorizontal.h
    constraints/ConstraintHorizontal.cpp
    constraints/ConstraintXDistance.h
    constraints/ConstraintXDistance.cpp
)
SOURCE_GROUP("Constraints" FILES ${Constraints_SRCS})

SET(gcs_solver_SRCS
    gcs_solver/Types.h
    gcs_solver/Shaker.h
    gcs_solver/System.h
    gcs_solver/System.cpp
    gcs_solver/LinearTransform.h
    gcs_solver/LinearTransform.cpp
    gcs_solver/Orthonormalization.h
    gcs_solver/Orthonormalization.cpp
#
    gcs_solver/parameters/Parameter.h
    gcs_solver/parameters/Parameter.cpp
    gcs_solver/parameters/Vector.h
    gcs_solver/parameters/Vector.cpp
    gcs_solver/parameters/ParameterProxy.h
    gcs_solver/parameters/ParameterProxy.cpp
    gcs_solver/parameters/ParameterGroup.h
    gcs_solver/parameters/ParameterGroup.cpp
    gcs_solver/parameters/ParameterProxyManager.h
    gcs_solver/parameters/ParameterProxyManager.cpp
#
    gcs_solver/equations/Equation.h
    gcs_solver/equations/Equation.cpp
    gcs_solver/equations/Constant.h
    gcs_solver/equations/Constant.cpp
    gcs_solver/equations/Equal.h
    gcs_solver/equations/Equal.cpp
    gcs_solver/equations/Difference.h
    gcs_solver/equations/Difference.cpp
    gcs_solver/equations/Distance.h
    gcs_solver/equations/Distance.cpp
    gcs_solver/equations/Colinear.h
    gcs_solver/equations/Colinear.cpp
#
    gcs_solver/linear_solvers/SolverBase.h
    gcs_solver/linear_solvers/SolverBase.cpp
    gcs_solver/linear_solvers/Ldlt.h
    gcs_solver/linear_solvers/Ldlt.cpp
#    gcs_solver/linear_solvers/BiCGStab.h
)
SOURCE_GROUP("GCS Solver" FILES ${gcs_solver_SRCS})

SET(PreCompiled_SRCS
    PreCompiled.h
    PreCompiled.cpp
)
SOURCE_GROUP("Precompiled" FILES ${PreCompiled_SRCS})

SET(SketchModule_SRCS
    AppNamedSketcher.cpp
    AppNamedSketcherPy.cpp
#     PreCompiled.cpp
#     PreCompiled.h
)
SOURCE_GROUP("Module" FILES ${SketchModule_SRCS})

SET(NamedSketcher_SRCS
    ${PreCompiled_SRCS}
    ${Features_SRCS}
    ${SketchModule_SRCS}
    ${Geometries_SRCS}
    ${Constraints_SRCS}
    ${gcs_solver_SRCS}
    ${Python_SRCS}
    ${Properties_SRCS}
#     ${Datatypes_SRCS}
)

add_library(NamedSketcher SHARED ${NamedSketcher_SRCS})
target_link_libraries(NamedSketcher ${NamedSketcher_LIBS})

if(FREECAD_USE_PCH)
    add_definitions(-D_PreComp_)
    GET_MSVC_PRECOMPILED_SOURCE("PreCompiled.cpp" NamedSketcher_CPP_SRCS ${NamedSketcher_PCH_SRCS})
    ADD_MSVC_PRECOMPILED_HEADER(NamedSketcher PreCompiled.h PreCompiled.cpp NamedSketcher_CPP_SRCS)
endif(FREECAD_USE_PCH)


SET_BIN_DIR(NamedSketcher NamedSketcher /Mod/NamedSketcher)
SET_PYTHON_PREFIX_SUFFIX(NamedSketcher)

INSTALL(TARGETS NamedSketcher DESTINATION ${CMAKE_INSTALL_LIBDIR})
