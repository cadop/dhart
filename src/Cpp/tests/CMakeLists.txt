
add_executable(HFUnitTests)

target_sources(
	HFUnitTests
	PRIVATE
		src/embree_raytracer.cpp
		src/OBJLoader.cpp
	)
target_include_directories(
	HFUnitTests
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(HFUnitTests gtest_main, OBJLoader, HFExceptions, EmbreeRayTracer)
