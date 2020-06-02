cmake_minimum_required (VERSION 3.8)

project ("humanfactors"
	VERSION "0.0.1")

# HumanFactors_Configure Variables
set(project_name "HumanFactors")
set_property(
	CACHE HumanFactors_Config PROPERTY
	STRINGS 
		All
		StandaloneRayTracer
		StandaloneGraphGenerator
		StandaloneVisibilityGraph
		StandaloneViewAnalysis
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHsc/MP")
option(HumanFactors_EnableTests "Install unit tests to the bin directory" ON)
set(HumanFactors_Config "StandaloneGraphGenerator" CACHE STRING "What projects to build")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}\\external")
set (ENABLE_EXPORTS ON)

add_subdirectory(humanfactorscsharp)
# Setup
add_library(HumanFactors SHARED)

# Add some global packages to the list
target_include_directories(
	HumanFactors
	PRIVATE
		interfaces
	    ${CMAKE_CURRENT_LIST_DIR}
)
add_subdirectory(packages/exceptions)
add_subdirectory(external)
install(
	FILES
		humanfactorspy/humanfactorspy/common_native_functions.py
		humanfactorspy/humanfactorspy/Exceptions.py
		humanfactorspy/humanfactorspy/native_numpy_like.py
		humanfactorspy/humanfactorspy/__init__.py
	DESTINATION humanfactorspy
)
install(DIRECTORY 
			"humanfactorspy/humanfactorspy/Example Models" 
			"humanfactorspy/humanfactorspy/Examples" 
			
		DESTINATION ${CMAKE_INSTALL_PREFIX})
install(
	FILES
		humanfactorspy/README.md
		humanfactorspy/setup.py
	DESTINATION
		${CMAKE_INSTALL_PREFIX}
)

install(
	FILES 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
	DESTINATION humanfactorspy/bin
)
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

# Set Compiler flags based on whether or not this is the release build
if(CMAKE_BUILD_TYPE MATCHES Release)
	set(
		CMAKE_CXX_FLAGS "/EHsc /openmp -O2"
	)
	message([STATUS] "Using release optimizations")
else()
	# set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,clang-analyzer*,clang-analyzer-cplusplus*,performance*,cppcoreguidlines* --extra-arg=-EHsc --extra-arg-before=--driver-mode=cl)
	set(
		CMAKE_CXX_FLAGS 
			"/openmp /EHsc /MP"
	)
endif()

# Change linked targets based on HumanFactors_Config
message([STATUS] ${HumanFactors_Config} " Selected as the current HumanFactors_Configuration")
if(${HumanFactors_Config} STREQUAL "All")
	message([STATUS] "Generating all packages")
	add_subdirectory(packages/raytracer)
	add_subdirectory(packages/spatialstructures)
	add_subdirectory(packages/objloader)
	add_subdirectory(packages/analysismethods)
	add_subdirectory(packages/pathfinding)
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
	install(
		FILES 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll
		${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
		DESTINATION bin
	)
	target_sources(
	    HumanFactors
		PRIVATE
			interfaces/analysis_C.cpp
			interfaces/cinterface_utils.cpp
			interfaces/raytracer_C.cpp
			interfaces/spatialstructures_C.cpp
			interfaces/view_analysis_C.cpp
			interfaces/visibility_graph_C.cpp
			interfaces/analysis_C.h
			interfaces/cinterface_utils.h
			interfaces/raytracer_C.h
			interfaces/spatialstructures_C.h
			interfaces/view_analysis_C.h
			interfaces/visibility_graph_C.h
			interfaces/objloader_C.cpp
			interfaces/objloader_C.h
			interfaces/pathfinder_C.cpp
			interfaces/pathfinder_C.h
	)
	install( 
		DIRECTORY 
			humanfactorspy/humanfactorspy/raytracer	
			humanfactorspy/humanfactorspy/spatialstructures
			humanfactorspy/humanfactorspy/geometry
			humanfactorspy/humanfactorspy/viewanalysis
			humanfactorspy/humanfactorspy/visibilitygraph
			humanfactorspy/humanfactorspy/pathfinding
			humanfactorspy/humanfactorspy/graphgenerator
			DESTINATION humanfactorspy
	)

elseif(${HumanFactors_Config} STREQUAL "StandaloneRayTracer")
	message([STATUS] "Building standalone raytracer")
	add_subdirectory(packages/raytracer)
	add_subdirectory(packages/objloader)

	target_link_libraries(
		HumanFactors 
		PRIVATE 
			EmbreeRayTracer
			OBJLoader
	)
	target_sources(
	    HumanFactors
		PRIVATE
			interfaces/objloader_C.cpp
			interfaces/objloader_C.h
			interfaces/raytracer_C.cpp
			interfaces/raytracer_C.h
			interfaces/cinterface_utils.h
			interfaces/cinterface_utils.cpp
	)
	install(
		FILES 
			${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll 
			${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
		DESTINATION bin
	)
	install(
		DIRECTORY
			humanfactorspy/humanfactorspy/geometry
			humanfactorspy/humanfactorspy/raytracer
		DESTINATION humanfactorspy
	)

elseif(${HumanFactors_Config} STREQUAL "StandaloneGraphGenerator")
	message([STATUS] "Building standalone graph generator")
	add_subdirectory(packages/raytracer)
	add_subdirectory(packages/objloader)
	add_subdirectory(packages/analysismethods)
	add_subdirectory(packages/spatialstructures)

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
			interfaces/objloader_C.cpp
			interfaces/objloader_C.h
			interfaces/raytracer_C.cpp
			interfaces/raytracer_C.h
			interfaces/cinterface_utils.h
			interfaces/cinterface_utils.cpp
			interfaces/spatialstructures_C.h
			interfaces/spatialstructures_C.cpp
	)
	install(
		FILES 
			${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll 
			${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
		DESTINATION bin
	)
	install(
		DIRECTORY
			humanfactorspy/humanfactorspy/geometry
			humanfactorspy/humanfactorspy/raytracer
			humanfactorspy/humanfactorspy/spatialstructures
			humanfactorspy/humanfactorspy/graphgenerator
		DESTINATION humanfactorspy
	)

	elseif(${HumanFactors_Config} STREQUAL "StandaloneVisibilityGraph")
		message([STATUS] "Building standalone Visibility Graph")
		add_subdirectory(packages/raytracer)
		add_subdirectory(packages/objloader)
		add_subdirectory(packages/analysismethods)
		add_subdirectory(packages/spatialstructures)

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
				interfaces/objloader_C.cpp
				interfaces/objloader_C.h
				interfaces/raytracer_C.cpp
				interfaces/raytracer_C.h
				interfaces/cinterface_utils.h
				interfaces/cinterface_utils.cpp
				interfaces/spatialstructures_C.h
				interfaces/spatialstructures_C.cpp
				interfaces/visibility_graph_C.cpp
				interfaces/visibility_graph_C.h
		)
		install(
			FILES 
				${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll 
				${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
			DESTINATION bin
		)
		install(
			DIRECTORY
				humanfactorspy/humanfactorspy/geometry
				humanfactorspy/humanfactorspy/raytracer
				humanfactorspy/humanfactorspy/spatialstructures
				humanfactorspy/humanfactorspy/visibilitygraph
			DESTINATION humanfactorspy
		)
	
	elseif(${HumanFactors_Config} STREQUAL "StandaloneViewAnalysis")
		message([STATUS] "Building standalone ViewAnalysis")
		add_subdirectory(packages/raytracer)
		add_subdirectory(packages/objloader)
		add_subdirectory(packages/analysismethods)
		add_subdirectory(packages/spatialstructures)

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
				interfaces/objloader_C.cpp
				interfaces/objloader_C.h
				interfaces/raytracer_C.cpp
				interfaces/raytracer_C.h
				interfaces/cinterface_utils.h
				interfaces/cinterface_utils.cpp
				interfaces/spatialstructures_C.h
				interfaces/spatialstructures_C.cpp
				interfaces/view_analysis_C.cpp
				interfaces/view_analysis_C.h
		)
		install(
			FILES 
				${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll 
				${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
			DESTINATION bin
		)
		install(
			DIRECTORY
				humanfactorspy/humanfactorspy/geometry
				humanfactorspy/humanfactorspy/raytracer
				humanfactorspy/humanfactorspy/spatialstructures
				humanfactorspy/humanfactorspy/viewanalysis
			DESTINATION humanfactorspy
		)
	elseif(${HumanFactors_Config} STREQUAL "StandaloneViewAnalysis")
	
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
					tests/src/AnalysisMethods.cpp
					tests/src/embree_raytracer.cpp
					tests/src/SpatialStructures.cpp
					tests/src/OBJLoader.cpp
					tests/src/VisibilityGraph.cpp
					tests/src/ViewAnalysis.cpp
					tests/src/PathFinding.cpp
			)
	elseif(${HumanFactors_Config} STREQUAL "StandaloneRayTracer")
		target_sources(
			HFUnitTests
			PRIVATE
				tests/src/embree_raytracer.cpp
				tests/src/OBJLoader.cpp
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
	elseif(${HumanFactors_Config} STREQUAL "StandaloneGraphGenerator")
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
				tests/src/embree_raytracer.cpp
				tests/src/OBJLoader.cpp
				tests/src/SpatialStructures.cpp
				tests/src/AnalysisMethods.cpp
		)
	elseif(${HumanFactors_Config} STREQUAL "StandaloneVisibilityGraph")
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
				tests/src/embree_raytracer.cpp
				tests/src/OBJLoader.cpp
				tests/src/SpatialStructures.cpp
				tests/src/VisibilityGraph.cpp
		)
	elseif(${HumanFactors_Config} STREQUAL "StandaloneViewAnalysis")
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
				tests/src/embree_raytracer.cpp
				tests/src/OBJLoader.cpp
				tests/src/SpatialStructures.cpp
				tests/src/ViewAnalysis.cpp
		)

	endif()
	# add_test(NAME FirstTest COMMAND HFUnitTests)
	gtest_discover_tests(HFUnitTests 
		WORKING_DIRECTORY $<TARGET_FILE_DIR:HFUnitTests>
	)
	install(
		TARGETS HFUnitTests
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include
	)

	add_custom_command(
		TARGET HFUnitTests PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/embree3.dll
		$<TARGET_FILE_DIR:HFUnitTests>
	)
	add_custom_command(
		TARGET HFUnitTests PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy 
		${CMAKE_CURRENT_SOURCE_DIR}/bin/tbb.dll
		$<TARGET_FILE_DIR:HFUnitTests>
	)
	add_custom_command(
		TARGET HFUnitTests PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/tests/Example Models"
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

install(TARGETS ${project_name}
		EXPORT ${project_name}Targets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION humanfactorspy/bin
		INCLUDES DESTINATION include
	)
install(EXPORT ${project_name}Targets
		FILE
			${project_name}Targets.cmake
#		NAMESPACE
#			${project_name}::
		DESTINATION
			lib/cmake/${project_name}
		)

include(CMakePackageConfigHelpers)
add_custom_command(TARGET HumanFactors POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/HumanFactors.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/humanfactorscsharp/$<CONFIGURATION>/HumanFactors.dll")

include(CMakePackageConfigHelpers)
add_custom_command(TARGET HumanFactors POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        "${CMAKE_CURRENT_BINARY_DIR}/humanfactorscsharp/$<CONFIGURATION>/")

include(CMakePackageConfigHelpers)
add_custom_command(TARGET HumanFactors POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/humanfactorspy/humanfactorspy/Example Models"
        "${CMAKE_CURRENT_BINARY_DIR}/humanfactorscsharp/$<CONFIGURATION>/ExampleModels")



#add_library(${project_name}::${project_name} ALIAS ${project_name})
