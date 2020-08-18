cmake_minimum_required (VERSION 3.8)

add_library(GraphGenerator STATIC)
target_sources(
	GraphGenerator
	PRIVATE
		src/unique_queue.cpp
		src/unique_queue.h
		src/graph_generator.h
		src/graph_generator.cpp
		src/graph_utils.cpp
	)
target_link_libraries(
	GraphGenerator
	PRIVATE
		EmbreeRayTracer
		SpatialStructures
	PUBLIC
		HFExceptions
)
target_include_directories(
	GraphGenerator
	PRIVATE
		${EXTERNAL_DIR}/robin_hood
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/src
	)

add_library(VisibilityGraph STATIC)
target_sources(
	VisibilityGraph
	PRIVATE
		src/visibility_graph.cpp
		src/visibility_graph.h
)
target_include_directories(
	VisibilityGraph
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/src
)
target_link_libraries(
	VisibilityGraph
	PRIVATE
		SpatialStructures
		EmbreeRayTracer
)

add_library(ViewAnalysis STATIC)
target_sources(
	ViewAnalysis
	PRIVATE
		src/view_analysis.cpp
		src/view_analysis.h
)
target_include_directories(
	ViewAnalysis
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/src
)
target_link_libraries(
	ViewAnalysis
	PRIVATE
		SpatialStructures
		EmbreeRayTracer
)