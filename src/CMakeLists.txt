cmake_minimum_required (VERSION 3.8)

# Paths
set(PYTHON_PACKAGE_DIR "Python")
set(CSHARP_DIR "CSharp")
set(C_INTERFACE_DIR "Cinterface")
set(C_PACKAGE_DIR "Cpp")
set(C_TEST_DRIVER_DIR "Cpp/tests/src")
set(DEPENDENCY_BINARIES
		"${CMAKE_SOURCE_DIR}/external/Embree/bin/tbb.dll" 
		"${CMAKE_SOURCE_DIR}/external/Embree/bin/embree3.dll"
		"${CMAKE_SOURCE_DIR}/external/Microsoft/Release/msvcp140.dll"
		"${CMAKE_SOURCE_DIR}/external/Microsoft/Release/vcruntime140.dll"
)


set(DEBUG_DEPENDENCY_BINARIES
	"${CMAKE_SOURCE_DIR}/external/Microsoft/Debug/vcruntime140_1d.dll"
	"${CMAKE_SOURCE_DIR}/external/Microsoft/Debug/vcruntime140d.dll"
	"${CMAKE_SOURCE_DIR}/external/Microsoft/Debug/vcomp140d.dll"
	"${CMAKE_SOURCE_DIR}/external/Microsoft/Debug/msvcp140d.dll"
)

set(RELEASE_DEPENDENCY_BINARIES
	"${CMAKE_SOURCE_DIR}/external/Microsoft/Release/vcruntime140_1.dll"
	"${CMAKE_SOURCE_DIR}/external/Microsoft/Release/vcomp140.dll"
)



project ("humanfactors" VERSION "0.0.1")
set(project_name "HumanFactors")


# Configuration options
set_property(
	CACHE HumanFactors_Config PROPERTY
	STRINGS 
		All
		RayTracer
		GraphGenerator
		ViewAnalysis
		VisibilityGraph
		Pathfinder
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHsc /MP")
option(HumanFactors_EnableTests "Install unit tests to the bin directory" ON)
option(HumanFactors_EnableCSharp "Include C# Interface" ON)
option(HumanFactors_EnablePython "Include Python Interface" ON)
set(HumanFactors_InstallTitle "" CACHE STRING "Installed package will be written to release/<InstallTitle>/. An empty string will follow default behavior.")
set(HumanFactors_Config "GraphGenerator" CACHE STRING "What projects to build")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}\\external")
set (ENABLE_EXPORTS ON)

if (NOT "${HumanFactors_InstallTitle}" STREQUAL "")
	set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/release/${HumanFactors_InstallTitle})
endif()

# Change install path if specified

#if (NOT HumanFactors_UseDefaultInstallPath)
#	if (HumanFactors_EnablePython) 
#		set(LANGUAGE_SUPPORT_STRING ${LANGUAGE_SUPPORT_STRING}Python_)
#	endif()
#	if (HumanFactors_EnableCSharp) 
#		set(LANGUAGE_SUPPORT_STRING ${LANGUAGE_SUPPORT_STRING}CSharp_) 
#	endif()
#
#	set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/release/${LANGUAGE_SUPPORT_STRING}${HumanFactors_Config})
#	message(${CMAKE_INSTALL_PREFIX})
#endif()

# Setup
add_library(HumanFactors SHARED)
# These files are always needed
target_include_directories(	HumanFactors PRIVATE ${C_INTERFACE_DIR}  ${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(${C_PACKAGE_DIR}/exceptions)
add_subdirectory(external)

# Set Compiler flags based on whether or not this is the release build
if(CMAKE_BUILD_TYPE MATCHES Release)
	set(CMAKE_CXX_FLAGS "/EHsc /openmp -O2")
	message([STATUS] "Using release optimizations")
else()
	set(CMAKE_CXX_FLAGS "/openmp /EHsc /MP")
endif()


# /$$$$$$                       /$$$$$$  /$$                                          /$$     /$$                    
# /$$__  $$                     /$$__  $$|__/                                         | $$    |__/                    
#| $$  \__/  /$$$$$$  /$$$$$$$ | $$  \__/ /$$  /$$$$$$  /$$   /$$  /$$$$$$  /$$$$$$  /$$$$$$   /$$  /$$$$$$  /$$$$$$$ 
#| $$       /$$__  $$| $$__  $$| $$$$    | $$ /$$__  $$| $$  | $$ /$$__  $$|____  $$|_  $$_/  | $$ /$$__  $$| $$__  $$
#| $$      | $$  \ $$| $$  \ $$| $$_/    | $$| $$  \ $$| $$  | $$| $$  \__/ /$$$$$$$  | $$    | $$| $$  \ $$| $$  \ $$
#| $$    $$| $$  | $$| $$  | $$| $$      | $$| $$  | $$| $$  | $$| $$      /$$__  $$  | $$ /$$| $$| $$  | $$| $$  | $$
#|  $$$$$$/|  $$$$$$/| $$  | $$| $$      | $$|  $$$$$$$|  $$$$$$/| $$     |  $$$$$$$  |  $$$$/| $$|  $$$$$$/| $$  | $$
# \______/  \______/ |__/  |__/|__/      |__/ \____  $$ \______/ |__/      \_______/   \___/  |__/ \______/ |__/  |__/
#                                             /$$  \ $$                                                               
#                                            |  $$$$$$/                                                               
#                                             \______/                                                                

# These are lists of the files that need to be installed based configuration
set(PYTHON_MODULES  #Folders in the python directory that need to be copied over
		"${PYTHON_PACKAGE_DIR}/humanfactorspy/Example Models" 
		"${PYTHON_PACKAGE_DIR}/humanfactorspy/Examples" 
)

set(PYTHON_FILES # Files to be placed in the python root directory
		${PYTHON_PACKAGE_DIR}/humanfactorspy/common_native_functions.py
		${PYTHON_PACKAGE_DIR}/humanfactorspy/Exceptions.py
		${PYTHON_PACKAGE_DIR}/humanfactorspy/native_numpy_like.py
		${PYTHON_PACKAGE_DIR}/humanfactorspy/__init__.py

)
set(ROOT_FILES # Files to be placed in the root of the project
		${PYTHON_PACKAGE_DIR}/README.md
)

# Change linked targets based on HumanFactors_Config
message([STATUS] ${HumanFactors_Config} " Selected as the current HumanFactors_Configuration")
if(${HumanFactors_Config} STREQUAL "All")
	message([STATUS] "Generating all packages")
	add_subdirectory(${C_PACKAGE_DIR}/raytracer)
	add_subdirectory(${C_PACKAGE_DIR}/spatialstructures)
	add_subdirectory(${C_PACKAGE_DIR}/objloader)
	add_subdirectory(${C_PACKAGE_DIR}/analysismethods)
	add_subdirectory(${C_PACKAGE_DIR}/pathfinding)
	target_link_libraries(
		HumanFactors PRIVATE 
			EmbreeRayTracer
			OBJLoader
			SpatialStructures	
			GraphGenerator
			VisibilityGraph
			ViewAnalysis
			Pathfinder
	)
	target_sources(
	    HumanFactors
		PRIVATE
			${C_INTERFACE_DIR}/analysis_C.cpp
			${C_INTERFACE_DIR}/cinterface_utils.cpp
			${C_INTERFACE_DIR}/raytracer_C.cpp
			${C_INTERFACE_DIR}/spatialstructures_C.cpp
			${C_INTERFACE_DIR}/view_analysis_C.cpp
			${C_INTERFACE_DIR}/visibility_graph_C.cpp
			${C_INTERFACE_DIR}/analysis_C.h
			${C_INTERFACE_DIR}/cinterface_utils.h
			${C_INTERFACE_DIR}/raytracer_C.h
			${C_INTERFACE_DIR}/spatialstructures_C.h
			${C_INTERFACE_DIR}/view_analysis_C.h
			${C_INTERFACE_DIR}/visibility_graph_C.h
			${C_INTERFACE_DIR}/objloader_C.cpp
			${C_INTERFACE_DIR}/objloader_C.h
			${C_INTERFACE_DIR}/pathfinder_C.cpp
			${C_INTERFACE_DIR}/pathfinder_C.h
	)
	LIST (APPEND PYTHON_MODULES
			${PYTHON_PACKAGE_DIR}/humanfactorspy/raytracer	
			${PYTHON_PACKAGE_DIR}/humanfactorspy/spatialstructures
			${PYTHON_PACKAGE_DIR}/humanfactorspy/geometry
			${PYTHON_PACKAGE_DIR}/humanfactorspy/viewanalysis
			${PYTHON_PACKAGE_DIR}/humanfactorspy/visibilitygraph
			${PYTHON_PACKAGE_DIR}/humanfactorspy/pathfinding
			${PYTHON_PACKAGE_DIR}/humanfactorspy/graphgenerator
	)

elseif(${HumanFactors_Config} STREQUAL "RayTracer")
	message([STATUS] "Building  raytracer")
	add_subdirectory(${C_PACKAGE_DIR}/raytracer)
	add_subdirectory(${C_PACKAGE_DIR}/objloader)

	target_link_libraries(
		HumanFactors 
		PRIVATE 
			EmbreeRayTracer
			OBJLoader
	)
	target_sources(
	    HumanFactors
		PRIVATE
			${C_INTERFACE_DIR}/objloader_C.cpp
			${C_INTERFACE_DIR}/objloader_C.h
			${C_INTERFACE_DIR}/raytracer_C.cpp
			${C_INTERFACE_DIR}/raytracer_C.h
			${C_INTERFACE_DIR}/cinterface_utils.h
			${C_INTERFACE_DIR}/cinterface_utils.cpp
	)
	LIST (APPEND PYTHON_MODULES
			${PYTHON_PACKAGE_DIR}/humanfactorspy/geometry
			${PYTHON_PACKAGE_DIR}/humanfactorspy/raytracer
	)

elseif(${HumanFactors_Config} STREQUAL "GraphGenerator")
	message([STATUS] "Building  graph generator")
	add_subdirectory(${C_PACKAGE_DIR}/raytracer)
	add_subdirectory(${C_PACKAGE_DIR}/objloader)
	add_subdirectory(${C_PACKAGE_DIR}/analysismethods)
	add_subdirectory(${C_PACKAGE_DIR}/spatialstructures)

	target_link_libraries(
		HumanFactors 
		PRIVATE
			EmbreeRayTracer
			OBJLoader
			SpatialStructures
			GraphGenerator
	)
	target_sources(
	    HumanFactors
		PRIVATE
			${C_INTERFACE_DIR}/objloader_C.cpp
			${C_INTERFACE_DIR}/objloader_C.h
			${C_INTERFACE_DIR}/raytracer_C.cpp
			${C_INTERFACE_DIR}/raytracer_C.h
			${C_INTERFACE_DIR}/cinterface_utils.h
			${C_INTERFACE_DIR}/cinterface_utils.cpp
			${C_INTERFACE_DIR}/spatialstructures_C.h
			${C_INTERFACE_DIR}/spatialstructures_C.cpp
			${C_INTERFACE_DIR}/analysis_C.cpp
			${C_INTERFACE_DIR}/analysis_C.h
	)
	LIST(APPEND PYTHON_MODULES
			${PYTHON_PACKAGE_DIR}/humanfactorspy/geometry
			${PYTHON_PACKAGE_DIR}/humanfactorspy/raytracer
			${PYTHON_PACKAGE_DIR}/humanfactorspy/spatialstructures
			${PYTHON_PACKAGE_DIR}/humanfactorspy/graphgenerator
	)

	elseif(${HumanFactors_Config} STREQUAL "VisibilityGraph")
		message([STATUS] "Building  Visibility Graph")
		add_subdirectory(${C_PACKAGE_DIR}/raytracer)
		add_subdirectory(${C_PACKAGE_DIR}/objloader)
		add_subdirectory(${C_PACKAGE_DIR}/analysismethods)
		add_subdirectory(${C_PACKAGE_DIR}/spatialstructures)

		target_link_libraries(
			HumanFactors 
			PRIVATE
				EmbreeRayTracer
				OBJLoader
				SpatialStructures
				VisibilityGraph
		)
		target_sources(
			HumanFactors
			PRIVATE
				${C_INTERFACE_DIR}/objloader_C.cpp
				${C_INTERFACE_DIR}/objloader_C.h
				${C_INTERFACE_DIR}/raytracer_C.cpp
				${C_INTERFACE_DIR}/raytracer_C.h
				${C_INTERFACE_DIR}/cinterface_utils.h
				${C_INTERFACE_DIR}/cinterface_utils.cpp
				${C_INTERFACE_DIR}/spatialstructures_C.h
				${C_INTERFACE_DIR}/spatialstructures_C.cpp
				${C_INTERFACE_DIR}/visibility_graph_C.cpp
				${C_INTERFACE_DIR}/visibility_graph_C.h
		)
		LIST(APPEND PYTHON_MODULES
				${PYTHON_PACKAGE_DIR}/humanfactorspy/geometry
				${PYTHON_PACKAGE_DIR}/humanfactorspy/raytracer
				${PYTHON_PACKAGE_DIR}/humanfactorspy/spatialstructures
				${PYTHON_PACKAGE_DIR}/humanfactorspy/visibilitygraph
		)
	
	elseif(${HumanFactors_Config} STREQUAL "ViewAnalysis")
		message([STATUS] "Building  ViewAnalysis")
		add_subdirectory(${C_PACKAGE_DIR}/raytracer)
		add_subdirectory(${C_PACKAGE_DIR}/objloader)
		add_subdirectory(${C_PACKAGE_DIR}/analysismethods)
		add_subdirectory(${C_PACKAGE_DIR}/spatialstructures)

		target_link_libraries(
			HumanFactors 
			PRIVATE
				EmbreeRayTracer
				OBJLoader
				SpatialStructures
				ViewAnalysis
		)
		target_sources(
			HumanFactors
			PRIVATE
				${C_INTERFACE_DIR}/objloader_C.cpp
				${C_INTERFACE_DIR}/objloader_C.h
				${C_INTERFACE_DIR}/raytracer_C.cpp
				${C_INTERFACE_DIR}/raytracer_C.h
				${C_INTERFACE_DIR}/cinterface_utils.h
				${C_INTERFACE_DIR}/cinterface_utils.cpp
				${C_INTERFACE_DIR}/spatialstructures_C.h
				${C_INTERFACE_DIR}/spatialstructures_C.cpp
				${C_INTERFACE_DIR}/view_analysis_C.cpp
				${C_INTERFACE_DIR}/view_analysis_C.h
		)
		LIST(APPEND PYTHON_MODULES
				${PYTHON_PACKAGE_DIR}/humanfactorspy/geometry
				${PYTHON_PACKAGE_DIR}/humanfactorspy/raytracer
				${PYTHON_PACKAGE_DIR}/humanfactorspy/spatialstructures
				${PYTHON_PACKAGE_DIR}/humanfactorspy/viewanalysis
		)
	
	elseif(${HumanFactors_Config} STREQUAL "VisibilityGraph")
		message([STATUS] "Building Visibilitygraph")
		add_subdirectory(${C_PACKAGE_DIR}/raytracer)
		add_subdirectory(${C_PACKAGE_DIR}/objloader)
		add_subdirectory(${C_PACKAGE_DIR}/analysismethods)
		add_subdirectory(${C_PACKAGE_DIR}/spatialstructures)

		target_link_libraries(
			HumanFactors 
			PRIVATE
				EmbreeRayTracer
				OBJLoader
				SpatialStructures
				VisibilityGraph
		)
		target_sources(
			HumanFactors
			PRIVATE
				${C_INTERFACE_DIR}/objloader_C.cpp
				${C_INTERFACE_DIR}/objloader_C.h
				${C_INTERFACE_DIR}/raytracer_C.cpp
				${C_INTERFACE_DIR}/raytracer_C.h
				${C_INTERFACE_DIR}/cinterface_utils.h
				${C_INTERFACE_DIR}/cinterface_utils.cpp
				${C_INTERFACE_DIR}/spatialstructures_C.h
				${C_INTERFACE_DIR}/spatialstructures_C.cpp
				${C_INTERFACE_DIR}/visibility_graph_C.h
				${C_INTERFACE_DIR}/visibility_graph_C.cpp
		)
		LIST(APPEND PYTHON_MODULES
				${PYTHON_PACKAGE_DIR}/humanfactorspy/geometry
				${PYTHON_PACKAGE_DIR}/humanfactorspy/raytracer
				${PYTHON_PACKAGE_DIR}/humanfactorspy/spatialstructures
				${PYTHON_PACKAGE_DIR}/humanfactorspy/VisibilityGraph
		)
		
	elseif(${HumanFactors_Config} STREQUAL "Pathfinder")

		add_subdirectory(${C_PACKAGE_DIR}/spatialstructures)
		add_subdirectory(${C_PACKAGE_DIR}/pathfinding)
		target_link_libraries(
			HumanFactors 
			PRIVATE
				SpatialStructures
				Pathfinder
		)
		target_sources(
			HumanFactors
			PRIVATE
				${C_INTERFACE_DIR}/cinterface_utils.h
				${C_INTERFACE_DIR}/cinterface_utils.cpp
				${C_INTERFACE_DIR}/spatialstructures_C.h
				${C_INTERFACE_DIR}/spatialstructures_C.cpp
				${C_INTERFACE_DIR}/pathfinder_C.h
				${C_INTERFACE_DIR}/pathfinder_C.cpp
		)
		LIST(APPEND PYTHON_MODULES
				${PYTHON_PACKAGE_DIR}/humanfactorspy/spatialstructures
				${PYTHON_PACKAGE_DIR}/humanfactorspy/pathfinder
		)
	
else()
	message(FATAL_ERROR "That is not a valid configuration ${HumanFactors_Config}")
endif()


# /$$$$$$$$                   /$$             
#|__  $$__/                  | $$             
#   | $$  /$$$$$$   /$$$$$$$/$$$$$$   /$$$$$$$
#   | $$ /$$__  $$ /$$_____/_  $$_/  /$$_____/
#   | $$| $$$$$$$$|  $$$$$$  | $$   |  $$$$$$ 
#   | $$| $$_____/ \____  $$ | $$ /$$\____  $$
#   | $$|  $$$$$$$ /$$$$$$$/ |  $$$$//$$$$$$$/
#   |__/ \_______/|_______/   \___/ |_______/ 
                                            
if(HumanFactors_EnableTests)
	enable_testing()
	include(GoogleTest)
	# Download and unpack googletest at HumanFactors_Configure time
	configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	  RESULT_VARIABLE result
	  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
	if(result)
	  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
	  RESULT_VARIABLE result
	  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
	if(result)
	  message(FATAL_ERROR "Build step for googletest failed: ${result}")
	endif()

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This defines
	# the gtest and gtest_main targets.
	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
					 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
					 EXCLUDE_FROM_ALL)

	# The gtest/gtest_main targets carry header search path
	# dependencies automatically when using CMake 2.8.11 or
	# later. Otherwise we have to add them here ourselves.
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
	  include_directories("${gtest_SOURCE_DIR}/include")
	endif()

	# Change the linked targets based on what config we're using
	add_executable(HFUnitTests)
	if(${HumanFactors_Config} STREQUAL "All")
		message([STATUS] "Building all tests")
		target_link_libraries(
			HFUnitTests
			PRIVATE
				gtest_main
				EmbreeRayTracer
				OBJLoader
				SpatialStructures
				GraphGenerator
				ViewAnalysis
				VisibilityGraph
				HFExceptions
				Pathfinder
		)			
		target_sources(
			HFUnitTests
				PRIVATE
					${C_TEST_DRIVER_DIR}/AnalysisMethods.cpp
					${C_TEST_DRIVER_DIR}/embree_raytracer.cpp
					${C_TEST_DRIVER_DIR}/SpatialStructures.cpp
					${C_TEST_DRIVER_DIR}/OBJLoader.cpp
					${C_TEST_DRIVER_DIR}/VisibilityGraph.cpp
					${C_TEST_DRIVER_DIR}/ViewAnalysis.cpp
					${C_TEST_DRIVER_DIR}/PathFinding.cpp
			)
	elseif(${HumanFactors_Config} STREQUAL "RayTracer")
		target_sources(
			HFUnitTests
			PRIVATE
				${C_TEST_DRIVER_DIR}/embree_raytracer.cpp
				${C_TEST_DRIVER_DIR}/OBJLoader.cpp
			)
		target_include_directories(
			HFUnitTests
			PRIVATE
				${CMAKE_CURRENT_LIST_DIR}/src
		)

		target_link_libraries(HFUnitTests 
			PRIVATE 
				gtest_main
				OBJLoader
				HFExceptions 
				EmbreeRayTracer
			)
	elseif(${HumanFactors_Config} STREQUAL "GraphGenerator")
		target_link_libraries(
			HFUnitTests
			PRIVATE
				gtest_main
				OBJLoader
				HFExceptions
				EmbreeRayTracer
				SpatialStructures
				GraphGenerator
		)
		target_sources(
			HFUnitTests
			PRIVATE
				${C_TEST_DRIVER_DIR}/embree_raytracer.cpp
				${C_TEST_DRIVER_DIR}/OBJLoader.cpp
				${C_TEST_DRIVER_DIR}/SpatialStructures.cpp
				${C_TEST_DRIVER_DIR}/AnalysisMethods.cpp
		)
	elseif(${HumanFactors_Config} STREQUAL "VisibilityGraph")
		target_link_libraries(
			HFUnitTests
			PRIVATE
				gtest_main
				OBJLoader
				HFExceptions
				EmbreeRayTracer
				SpatialStructures
				VisibilityGraph
		)
		target_sources(
			HFUnitTests
			PRIVATE
				${C_TEST_DRIVER_DIR}/embree_raytracer.cpp
				${C_TEST_DRIVER_DIR}/OBJLoader.cpp
				${C_TEST_DRIVER_DIR}/SpatialStructures.cpp
				${C_TEST_DRIVER_DIR}/VisibilityGraph.cpp
		)
	elseif(${HumanFactors_Config} STREQUAL "ViewAnalysis")
		target_link_libraries(
			HFUnitTests
			PRIVATE
				gtest_main
				OBJLoader
				HFExceptions
				EmbreeRayTracer
				SpatialStructures
				ViewAnalysis
		)
		target_sources(
			HFUnitTests
			PRIVATE
				${C_TEST_DRIVER_DIR}/embree_raytracer.cpp
				${C_TEST_DRIVER_DIR}/OBJLoader.cpp
				${C_TEST_DRIVER_DIR}/SpatialStructures.cpp
				${C_TEST_DRIVER_DIR}/ViewAnalysis.cpp
		)
	elseif(${HumanFactors_Config} STREQUAL "VisibilityGraph")
		target_link_libraries(
			HFUnitTests
			PRIVATE
				gtest_main
				OBJLoader
				HFExceptions
				EmbreeRayTracer
				SpatialStructures
				VisibilityGraph
		)
		target_sources(
			HFUnitTests
			PRIVATE
				${C_TEST_DRIVER_DIR}/embree_raytracer.cpp
				${C_TEST_DRIVER_DIR}/OBJLoader.cpp
				${C_TEST_DRIVER_DIR}/SpatialStructures.cpp
				${C_TEST_DRIVER_DIR}/VisibilityGraph.cpp
		)
	elseif(${HumanFactors_Config} STREQUAL "Pathfinder")
		target_link_libraries(
			HFUnitTests
			PRIVATE
				gtest_main
				HFExceptions
				SpatialStructures
				Pathfinder
		)
		target_sources(
			HFUnitTests
			PRIVATE
				${C_TEST_DRIVER_DIR}/SpatialStructures.cpp
				${C_TEST_DRIVER_DIR}/PathFinding.cpp
		)
	endif()
	# add_test(NAME FirstTest COMMAND HFUnitTests)
	gtest_discover_tests(HFUnitTests WORKING_DIRECTORY $<TARGET_FILE_DIR:HFUnitTests>)
	install(
		TARGETS HFUnitTests
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION includes
	)
	add_custom_command(
		TARGET HFUnitTests PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy 
		${DEPENDENCY_BINARIES}
		$<TARGET_FILE_DIR:HFUnitTests>
	)
	add_custom_command(
		TARGET HFUnitTests PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/${C_PACKAGE_DIR}/tests/Example Models"
		$<TARGET_FILE_DIR:HFUnitTests>
	)
endif()

# /$$$$$$                      /$$              /$$ /$$
#|_  $$_/                     | $$             | $$| $$
#  | $$   /$$$$$$$   /$$$$$$$/$$$$$$   /$$$$$$ | $$| $$
#  | $$  | $$__  $$ /$$_____/_  $$_/  |____  $$| $$| $$
#  | $$  | $$  \ $$|  $$$$$$  | $$     /$$$$$$$| $$| $$
#  | $$  | $$  | $$ \____  $$ | $$ /$$/$$__  $$| $$| $$
# /$$$$$$| $$  | $$ /$$$$$$$/ |  $$$$/  $$$$$$$| $$| $$
#|______/|__/  |__/|_______/   \___/  \_______/|__/|__/


# Install the current C++ Project
install(TARGETS ${project_name}
		EXPORT ${project_name}Targets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include
	)
install(FILES ${DEPENDENCY_BINARIES} DESTINATION bin)

# Install python if the option is set to true
if(HumanFactors_EnablePython)
	message("Installing Python!")
	install(DIRECTORY ${PYTHON_MODULES} DESTINATION humanfactorspy)
	install(FILES ${PYTHON_FILES} DESTINATION humanfactorspy)
	install(FILES ${DEPENDENCY_BINARIES} DESTINATION humanfactorspy/bin)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/HumanFactors.dll" DESTINATION humanfactorspy/bin)

	install (FILES ${RELEASE_DEPENDENCY_BINARIES} CONFIGURATIONS Release DESTINATION humanfactorspy/bin)
	install (FILES ${DEBUG_DEPENDENCY_BINARIES} CONFIGURATIONS Debug DESTINATION humanfactorspy/bin)

	LIST(APPEND ROOT_FILES ${PYTHON_PACKAGE_DIR}/setup.py)
endif()


install (FILES ${RELEASE_DEPENDENCY_BINARIES} CONFIGURATIONS Release DESTINATION bin)
install (FILES ${DEBUG_DEPENDENCY_BINARIES} CONFIGURATIONS Debug DESTINATION bin)

#
#if ($<IF<$<Config:Release>>)
#	install(FILES ${RELEASE_DEPENDENCY_BINARIES} DESTINATION humanfactorspy/bin)
#else()
#	install(FILES ${DEBUG_DEPENDENCY_BINARIES} DESTINATION humanfactorspy/bin)
#endif()


#install(FILES $<IF:$<CONFIG:Release>,${RELEASE_DEPENDENCY_BINARIES},${DEBUG_DEPENDENCY_BINARIES}> DESTINATION humanfactorspy/bin)


install(EXPORT ${project_name}Targets
	FILE
		${project_name}Targets.cmake
#	NAMESPACE
#		${project_name}::
	DESTINATION
		lib/cmake/${project_name}
)

if (HumanFactors_EnableCSharp)
	add_subdirectory(${CSHARP_DIR})

	add_custom_command(TARGET HumanFactors POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_SOURCE_DIR}/external/Embree/bin/tbb.dll" 
			"${CMAKE_CURRENT_BINARY_DIR}/CSharp/$<CONFIGURATION>/tbb.dll"
	)

	add_custom_command(TARGET HumanFactors POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_SOURCE_DIR}/external/Embree/bin/embree3.dll" 
			"${CMAKE_CURRENT_BINARY_DIR}/CSharp/$<CONFIGURATION>/embree3.dll"
	)

	# The following commands copy resources necessary for C# to perform its unittests
	add_custom_command(TARGET HumanFactors POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/HumanFactors.dll"
			"${CMAKE_CURRENT_BINARY_DIR}/CSharp/$<CONFIGURATION>/HumanFactors.dll"
	)

	# Copy example models to the build directory for C#
	add_custom_command(TARGET HumanFactors POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/${PYTHON_PACKAGE_DIR}/humanfactorspy/Example Models"
			"${CMAKE_CURRENT_BINARY_DIR}/CSharp/$<CONFIGURATION>/ExampleModels"
	)
endif()


install(FILES ${ROOT_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX})
#add_library(${project_name}::${project_name} ALIAS ${project_name})
