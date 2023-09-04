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
link_directories(${OCC_LIBRARY_DIR} ${Boost_LIBRARY_DIRS})

set(NamedSketcher_LIBS
    Part
    FreeCADApp
)


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
)
SOURCE_GROUP("Features" FILES ${Features_SRCS})


include_directories(${pybind11_INCLUDE_DIR})
SET(PythonBindings_SRCS
    Module.pybind.cpp
    NamedSketch.pybind.h
    NamedSketch.pybind.cpp
    Reference.pybind.h
    Reference.pybind.cpp
    geometries/Geometry.pybind.h
    geometries/Geometry.pybind.cpp
    constraints/Constraint.pybind.h
    constraints/Constraint.pybind.cpp
)
SOURCE_GROUP("Python bindings" FILES ${PythonBindings_SRCS})

SET(Geometries_SRCS
    geometries/GeometryBase.h
    geometries/GeometryBase.inc
    geometries/GeometryBase.cpp
    geometries/GeometryFactory.h
    geometries/GeometryFactory.cpp
    geometries/GeometryPoint.h
    geometries/GeometryPoint.cpp
    geometries/GeometryLineSegment.h
    geometries/GeometryLineSegment.cpp
    geometries/GeometryCircle.h
    geometries/GeometryCircle.cpp
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
    constraints/ConstraintConstant.cpp
    constraints/ConstraintConstant.h
    constraints/ConstraintEqual.cpp
    constraints/ConstraintHorizontal.h
    constraints/ConstraintHorizontal.cpp
    constraints/ConstraintVertical.h
    constraints/ConstraintVertical.cpp
    constraints/ConstraintBlockPoint.h
    constraints/ConstraintBlockPoint.cpp
    constraints/ConstraintXDistance.h
    constraints/ConstraintXDistance.cpp
    constraints/ConstraintYDistance.h
    constraints/ConstraintYDistance.cpp
    constraints/ConstraintDistance.h
    constraints/ConstraintDistance.cpp
    constraints/ConstraintPointSymmetric.h
    constraints/ConstraintPointSymmetric.cpp
    constraints/ConstraintPointAlongCurve.h
    constraints/ConstraintPointAlongCurve.cpp
    constraints/ConstraintTangentCurves.h
    constraints/ConstraintTangentCurves.cpp
    constraints/tangent_curves/TangentCurvesBase.h
    constraints/tangent_curves/TangentCurvesGeneric.h
    constraints/tangent_curves/TangentCurvesGeneric.cpp
    constraints/tangent_curves/TangentCurvesLineLine.h
    constraints/tangent_curves/TangentCurvesLineLine.cpp
    constraints/tangent_curves/TangentCurvesLineCircle.h
    constraints/tangent_curves/TangentCurvesLineCircle.cpp
    constraints/tangent_curves/TangentCurvesCircleCircle.h
    constraints/tangent_curves/TangentCurvesCircleCircle.cpp
    constraints/point_along_curve/PointAlongCurveBase.h
    constraints/point_along_curve/PointAlongCurveGeneric.h
    constraints/point_along_curve/PointAlongCurveGeneric.cpp
    constraints/point_along_curve/PointAlongCurveLine.h
    constraints/point_along_curve/PointAlongCurveLine.cpp
    constraints/point_along_curve/PointAlongCurveCircle.h
    constraints/point_along_curve/PointAlongCurveCircle.cpp
)
SOURCE_GROUP("Constraints" FILES ${Constraints_SRCS})

SET(gcs_solver_SRCS
    gcs_solver/Types.h
    gcs_solver/Shaker.h
    gcs_solver/System.h
    gcs_solver/System.cpp
    gcs_solver/Vector.h
    gcs_solver/Vector.inc
    gcs_solver/LinearTransform.h
    gcs_solver/LinearTransform.inc
    gcs_solver/Orthonormalization.h
    gcs_solver/Orthonormalization.cpp
#
    gcs_solver/parameters/Parameter.h
    gcs_solver/parameters/Parameter.cpp
    gcs_solver/parameters/ParameterGroup.h
    gcs_solver/parameters/ParameterGroup.cpp
    gcs_solver/parameters/ParameterGroupManager.h
    gcs_solver/parameters/ParameterGroupManager.cpp
    gcs_solver/parameters/ParameterValueMapper.h
#
    gcs_solver/equations/Equation.h
    gcs_solver/equations/Equation.cpp
    gcs_solver/equations/EquationProxy.h
    gcs_solver/equations/EquationProxy.cpp
    gcs_solver/equations/Constant.h
    gcs_solver/equations/Constant.cpp
    gcs_solver/equations/Equal.h
    gcs_solver/equations/Equal.cpp
    gcs_solver/equations/Difference.h
    gcs_solver/equations/Difference.cpp
    gcs_solver/equations/OrthogonalDisplacement.h
    gcs_solver/equations/OrthogonalDisplacement.cpp
    gcs_solver/equations/MediumParameter.h
    gcs_solver/equations/MediumParameter.cpp
    gcs_solver/equations/Distance.h
    gcs_solver/equations/Distance.cpp
    gcs_solver/equations/Colinear.h
    gcs_solver/equations/Colinear.cpp
    gcs_solver/equations/PointAlongCurve.h
    gcs_solver/equations/PointAlongCurve.cpp
    gcs_solver/equations/ConcurrentCurves.h
    gcs_solver/equations/ConcurrentCurves.cpp
    gcs_solver/equations/ParallelCurves.h
    gcs_solver/equations/ParallelCurves.cpp
#
    gcs_solver/linear_solvers/SolverBase.h
    gcs_solver/linear_solvers/SolverBase.cpp
# This gives errors in old Eigens...
#    gcs_solver/linear_solvers/Ldlt.h
#    gcs_solver/linear_solvers/Ldlt.cpp
    gcs_solver/linear_solvers/BiCG.h
    gcs_solver/linear_solvers/BiCG.cpp
    gcs_solver/linear_solvers/SimpleSolver.h
    gcs_solver/linear_solvers/SimpleSolver.cpp
)
SOURCE_GROUP("GCS Solver" FILES ${gcs_solver_SRCS})

SET(PreCompiled_SRCS
    PreCompiled.h
    PreCompiled.cpp
)
SOURCE_GROUP("Precompiled" FILES ${PreCompiled_SRCS})

SET(NamedSketcher_SRCS
    ${PreCompiled_SRCS}
    ${Features_SRCS}
    ${SketchModule_SRCS}
    ${Geometries_SRCS}
    ${Constraints_SRCS}
    ${gcs_solver_SRCS}
    ${PythonBindings_SRCS}
    ${Properties_SRCS}
#     ${Datatypes_SRCS}
)

add_library(NamedSketcherCpp SHARED ${NamedSketcher_SRCS})
target_link_libraries(NamedSketcherCpp ${NamedSketcher_LIBS})

if(FREECAD_USE_PCH)
    add_definitions(-D_PreComp_)
    GET_MSVC_PRECOMPILED_SOURCE("PreCompiled.cpp" NamedSketcher_CPP_SRCS ${NamedSketcher_PCH_SRCS})
    ADD_MSVC_PRECOMPILED_HEADER(NamedSketcher PreCompiled.h PreCompiled.cpp NamedSketcher_CPP_SRCS)
endif(FREECAD_USE_PCH)

SET_BIN_DIR(NamedSketcherCpp NamedSketcherCpp /Mod/NamedSketcher)
SET_PYTHON_PREFIX_SUFFIX(NamedSketcherCpp)

INSTALL(TARGETS NamedSketcherCpp DESTINATION ${CMAKE_INSTALL_LIBDIR})
