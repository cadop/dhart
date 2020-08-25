cmake_minimum_required (VERSION 3.8)

add_executable(StringsToCosts)

target_sources(StringsToCosts PRIVATE "strtoc.cs")
target_link_libraries(StringsToCosts PRIVATE HumanFactorsCSharp)

set_target_properties(
		StringsToCosts
		PROPERTIES
		VS_DOTNET_REFERENCES "${normal_references}"
		VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5"
)

target_compile_options(HumanFactorsCSharp PRIVATE "/unsafe")
 install(TARGETS StringsToCosts
		EXPORT StringsToCosts
		RUNTIME DESTINATION bin
)




