cmake_minimum_required (VERSION 3.8)

add_executable(StringsToCosts)

target_sources(StringsToCosts PRIVATE "strtoc.cs")
target_link_libraries(StringsToCosts PRIVATE DHARTAPICSharp)
set(binary_path "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")

message([STATUS] "BINARY PATH ${binary_path}")
message([STATUS] "SOURCE PATH ${CMAKE_SOURCE_DIR}")

set_target_properties(
		StringsToCosts
		PROPERTIES
		VS_DOTNET_REFERENCES "${normal_references}"
		)

set_target_properties(
		StringsToCosts
		PROPERTIES
		VS_DOTNET_REFERENCES "${normal_references}"
		VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5"
)

target_compile_options(StringsToCosts PRIVATE "/unsafe")
 install(TARGETS StringsToCosts
		EXPORT StringsToCosts
		RUNTIME DESTINATION bin
)


# Code to copy over DLLS and ExampleModels on build 
add_custom_command(TARGET StringsToCosts POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/Embree/bin/tbb.dll" 
		"${binary_path}/tbb.dll"
)

add_custom_command(TARGET StringsToCosts POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/external/Embree/bin/embree3.dll" 
		"${binary_path}/embree3.dll"
)

# The following commands copy resources necessary for C# to perform its unittests
add_custom_command(TARGET StringsToCosts POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/DHARTAPI.dll"
		"${binary_path}/DHART_API.dll"
)

# Copy example models to the build directory for C#
add_custom_command(TARGET StringsToCosts POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/${PYTHON_PACKAGE_DIR}/dhart/Example Models"
		"${binary_path}/ExampleModels"
)

install (FILES ${CSHARP_DEPENDENCY_BINARIES} DESTINATION bin)


