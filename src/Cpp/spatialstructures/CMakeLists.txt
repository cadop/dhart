cmake_minimum_required (VERSION 3.8)

add_library(SpatialStructures STATIC)

set(CMAKE_CXX_STANDARD 17)
target_sources(
	SpatialStructures
	PRIVATE
		src/Constants.cpp
		src/edge.cpp
		src/node.cpp
		src/path.cpp
		src/graph.cpp
		
		src/Constants.h
		src/Edge.h
		src/node.h
		src/path.h
		src/graph.h
	)

target_link_libraries(
	SpatialStructures
	PRIVATE
		HFExceptions
)
target_include_directories(
	SpatialStructures
	PUBLIC
		${EXTERNAL_DIR}/Eigen
		${EXTERNAL_DIR}/robin_hood
		${CMAKE_CURRENT_LIST_DIR}/src
	)
