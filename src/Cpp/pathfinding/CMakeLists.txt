cmake_minimum_required (VERSION 3.8)

# Break this into two libraries?
add_library(Pathfinder STATIC)
target_sources(
	Pathfinder
	PRIVATE
		src/path_finder.cpp
		src/path_finder.h
		src/boost_graph.h
		src/boost_graph.cpp
	)

target_link_libraries(
	Pathfinder
	PUBLIC
		HFExceptions
		SpatialStructures
)
target_include_directories(
	Pathfinder
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/src
		${EXTERNAL_DIR}/robin_hood
		${EXTERNAL_DIR}/boost
		${EXTERNAL_DIR}/Embree/include
	)
