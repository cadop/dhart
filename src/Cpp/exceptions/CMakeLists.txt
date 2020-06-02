# CMakeList.txt : CMake project for humanfactors, include source and define
# project specific logic here.
#

add_library(HFExceptions)
target_sources(
	HFExceptions
	PRIVATE
		src/HFExceptions.cpp
		src/HFExceptions.h
	)

target_include_directories(
	HFExceptions
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/src
)
