set(sources ${sources}
	${CMAKE_CURRENT_LIST_DIR}/src/NativeUtils.cs
	${CMAKE_CURRENT_LIST_DIR}/src/PointerHolder.cs
	${CMAKE_CURRENT_LIST_DIR}/src/Exceptions.cs
	${CMAKE_CURRENT_LIST_DIR}/src/CommonTypes.cs
	${CMAKE_CURRENT_LIST_DIR}/src/NativeCollections.cs
)
set(test_sources ${test_sources}
	${CMAKE_CURRENT_LIST_DIR}/test_base.cs
	${CMAKE_CURRENT_LIST_DIR}/overall_examples.cs
)