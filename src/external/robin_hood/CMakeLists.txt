add_library(robin_hood INTERFACE)

message(" ROBIN_HOOD " ${CMAKE_LIST_DIRECTORY})
target_include_directories(
	robin_hood
	PUBLIC
		${CMAKE_LIST_DIRECTORY}
)

