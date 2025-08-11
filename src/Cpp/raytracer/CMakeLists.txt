cmake_minimum_required (VERSION 3.10)
set(ROBIN_HOOD_DIR ${PROJECT_SOURCE_DIR}/external/robin_hood/)

include(FetchContent)

add_library(EmbreeRayTracer)
target_sources(
	EmbreeRayTracer 
	PRIVATE
		src/embree_raytracer.cpp
		src/RayRequest.cpp
		src/embree_raytracer.h
		src/RayRequest.h
		src/nanort.h
		src/ray_data.h 
		src/nanort_raytracer.cpp
		src/MultiRT.h
		src/MultiRT.cpp
		src/HitStruct.cpp
		src/HitStruct.h
	)

# Just die if we can't find embree for now.
# if(WIN32)
set(Embree_DIR ${PROJECT_SOURCE_DIR}/external/Embree)
message([STATUS] "Setting embree dir to " ${Embree_DIR})
# else()
# 	set(Embree_DIR ${PROJECT_SOURCE_DIR}/external/Embree)
# 	message([STATUS] "Setting embree dir to " ${Embree_DIR})
# endif()

find_package(Embree 3.0 REQUIRED)
#message([STATUS] ${ROBIN_HOOD_DIR})
# Look for robin_hood code 
#FetchContent_Declare(
#	robin-hood-hashing 
#	GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing
#)

#FetchContent_Populate(robin-hood-hashing)
#FetchContent_MakeAvailable(robin-hood-hashing)
#message(STATUS "RobinHood Downloaded to " ${robin-hood-hashing_SOURCE_DIR})

#FetchContent_GetProperties(robin-hood-hashing)
# if (Not Embree)
	# Code to download embree?
# endif()

target_include_directories(
	EmbreeRayTracer 
	PUBLIC 
		${EMBREE_INCLUDE_DIRS}/embree3
	PUBLIC
		${EXTERNAL_DIR}/robin_hood
		${CMAKE_CURRENT_LIST_DIR}/src
	)

target_link_libraries(
	EmbreeRayTracer 
	PRIVATE 
		${EMBREE_LIBRARY}
		OBJLoader
		HFExceptions
	)
