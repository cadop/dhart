add_library(Eigen INTERFACE)

message(" EIGEN DIR " ${CMAKE_LIST_DIRECTORY})
target_include_directories(
	Eigen
	PUBLIC
		${CMAKE_LIST_DIRECTORY}
)

