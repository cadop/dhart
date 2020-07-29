cmake_minimum_required(VERSION 3.8)
project("HumanFactorsCSharp" CSharp)

### NOTE:
# If you have visual studio open for the generated projects, please close it before attempting to regenerate.
# failure to do so will cause a failure to build. If you do get in this state, you can fix it by deleting 
# the build directory entirely, then trying again

# Include whatever needs to be included for both the test and "normal" assemblies
LIST (APPEND test_references "Microsoft.Csharp")
List (APPEND test_references "HumanFactorsCSharp")
List (APPEND test_references "Microsoft.VisualStudio.QualityTools.UnitTestFramework")
LIST (APPEND test_references "System")
LIST (APPEND test_references "System.Numerics.Vectors")
LIST (APPEND test_references "System.Memory")
LIST (APPEND test_references "netstandard")

LIST (APPEND normal_references "System")
LIST (APPEND normal_references "System.Runtime.InteropServices")
LIST (APPEND normal_references "System.Numerics")
LIST (APPEND normal_references "System.Numerics.Vectors")
LIST (APPEND normal_references "System.Memory")
LIST (APPEND normal_references "netstandard")
LIST (APPEND normal_references "System.Runtime.Serialization")

# Set the C# language version
set(CMAKE_CSharp_FLAGS "/langversion:7.3")

# Include packages. These will add source files to ${sources} and test files to ${test_sources} for each package
include(${CMAKE_CURRENT_LIST_DIR}/packages/base/package.cmake)

if(${HumanFactors_Config} STREQUAL "All")
	include(${CMAKE_CURRENT_LIST_DIR}/packages/spatialstructures/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/geometry/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/raytracer/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/graphgenerator/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/viewanalysis/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/visibilitygraph/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/pathfinding/package.cmake)
elseif(${HumanFactors_Config} STREQUAL "RayTracer")
	include(${CMAKE_CURRENT_LIST_DIR}/packages/raytracer/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/geometry/package.cmake)
elseif(${HumanFactors_Config} STREQUAL "GraphGenerator")
	include(${CMAKE_CURRENT_LIST_DIR}/packages/spatialstructures/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/raytracer/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/geometry/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/graphgenerator/package.cmake)
elseif(${HumanFactors_Config} STREQUAL "VisibilityGraph")
	include(${CMAKE_CURRENT_LIST_DIR}/packages/spatialstructures/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/raytracer/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/geometry/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/visibilitygraph/package.cmake)
elseif(${HumanFactors_Config} STREQUAL "ViewAnalysis")
	include(${CMAKE_CURRENT_LIST_DIR}/packages/raytracer/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/geometry/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/viewanalysis/package.cmake)
elseif(${HumanFactors_Config} STREQUAL "Pathfinder")
	include(${CMAKE_CURRENT_LIST_DIR}/packages/pathfinding/package.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/packages/spatialstructures/package.cmake)
endif()


add_library(HumanFactorsCSharp SHARED)

# Here we use the source list as input to the HumanFactorsCSharp target
target_sources(HumanFactorsCSharp PRIVATE ${sources})

set_target_properties(
		HumanFactorsCSharp
		PROPERTIES
		VS_DOTNET_REFERENCES "${normal_references}"
		VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5"
)

target_compile_options(HumanFactorsCSharp PRIVATE "/unsafe")
 install(TARGETS HumanFactorsCSharp
		EXPORT HumanFactorsCSharp
		RUNTIME DESTINATION bin
)

# Set up test assembly
add_library(hfcsharptest)
target_sources(hfcsharptest PRIVATE	${test_sources})
target_compile_options(hfcsharptest PRIVATE "/unsafe") # Must be compiled with unsafe
set_target_properties(
		hfcsharptest
		PROPERTIES
		VS_GLOBAL_PROJECT_TYPES "{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"
		# VS_GLOBAL_PROJECT_TYPES "{d902c041-2bf7-4fa0-afd7-2a09df2dbb7a}"
		VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.1"
		VS_DOTNET_REFERENCES "${test_references}"
)

install(TARGETS hfcsharptest EXPORT hfcsharptest RUNTIME DESTINATION bin)


