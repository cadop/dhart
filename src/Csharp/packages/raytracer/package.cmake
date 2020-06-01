set(sources ${sources}
	${CMAKE_CURRENT_LIST_DIR}/src/EmbreeBVH.cs
	${CMAKE_CURRENT_LIST_DIR}/src/EmbreeRayTracer.cs
	${CMAKE_CURRENT_LIST_DIR}/src/RayTracerNative.cs
	${CMAKE_CURRENT_LIST_DIR}/src/RayResultStructs.cs
)
set(test_sources ${test_sources}
	${CMAKE_CURRENT_LIST_DIR}/test_raytracer.cs
)