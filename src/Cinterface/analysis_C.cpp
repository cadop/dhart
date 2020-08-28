#include <analysis_C.h>

#include <robin_hood.h>

#include <HFExceptions.h>
#include <embree_raytracer.h>
#include <graph_generator.h>
#include <graph.h>

using HF::SpatialStructures::Graph;
using HF::GraphGenerator::GraphGenerator;
using namespace HF::Exceptions;

template <typename T> 
std::vector<T> MapToVector(int length, const T * in_ptr){
	if (length == 0)
		return std::vector<T>(0);
	else
		return std::vector<T>(in_ptr, in_ptr + length);
}

C_INTERFACE GenerateGraph(
	HF::RayTracer::EmbreeRayTracer* ray_tracer,
	const float* start_point,
	const float* spacing,
	int MaxNodes,
	float UpStep,
	float UpSlope,
	float DownStep,
	float DownSlope,
	int max_step_connections,
	int core_count,
	Graph** out_graph
){
	const std::array<float, 3> start_array{ start_point[0], start_point[1], start_point[2] };
	const std::array<double, 3> spacing_array{ spacing[0], spacing[1], spacing[2] };

	Graph* G = new Graph();
	GraphGenerator GraphGen(*ray_tracer);
	*G = GraphGen.BuildNetwork(
		start_array,
		spacing_array,
		MaxNodes,
		UpStep,
		UpSlope,
		DownStep,
		DownSlope,
		max_step_connections,
		core_count
	);

	if (G->Nodes().size() < 1) {
		delete G;
		return HF_STATUS::NO_GRAPH;
	}
	*out_graph = G;
	return OK;
}

C_INTERFACE GenerateGraphObstacles(
	HF::RayTracer::EmbreeRayTracer* ray_tracer,
	const float* start_point,
	const float* spacing,
	int MaxNodes,
	float UpStep,
	float UpSlope,
	float DownStep,
	float DownSlope,
	int max_step_connections,
	int core_count,
	const int* obstacle_ids,
	const int* walkable_ids,
	int num_obstacles,
	int num_walkables,
	Graph** out_graph
) {
	const std::array<float, 3> start_array{ start_point[0], start_point[1], start_point[2] };
	const std::array<double, 3> spacing_array{ spacing[0], spacing[1], spacing[2] };

	const std::vector<int> obstacle_vector = MapToVector(num_obstacles, obstacle_ids);
	const std::vector<int> walkable_vector = MapToVector(num_walkables, walkable_ids);

	Graph* G = new Graph();
	GraphGenerator GraphGen(*ray_tracer, obstacle_vector, walkable_vector);
	*G = GraphGen.BuildNetwork(
		start_array,
		spacing_array,
		MaxNodes,
		UpStep,
		UpSlope,
		DownStep,
		DownSlope,
		max_step_connections,
		core_count
	);

	if (G->Nodes().size() < 1) {
		delete G;
		return HF_STATUS::NO_GRAPH;
	}
	*out_graph = G;
	return OK;
}
