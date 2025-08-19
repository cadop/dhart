cmake_minimum_required (VERSION 3.10)

# Break this into two libraries?
add_library(OBJLoader STATIC)
target_sources(
	OBJLoader
	PRIVATE
		src/objloader.cpp
		src/objloader.h
		src/meshinfo.h
		src/meshinfo.cpp
	)

target_link_libraries(
	OBJLoader
	PUBLIC
		HFExceptions
)
target_include_directories(
	OBJLoader
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/src
		${EXTERNAL_DIR}/Eigen
		${EXTERNAL_DIR}/robin_hood
	)
