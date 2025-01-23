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
	int min_connections,
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
		min_connections,
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
	int min_connections,
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
		min_connections,
		core_count
	);

	if (G->Nodes().size() < 1) {
		delete G;
		return HF_STATUS::NO_GRAPH;
	}
	*out_graph = G;
	return OK;
}

C_INTERFACE CalculateAndStoreStepTypes(
	HF::SpatialStructures::Graph* g, 
	HF::RayTracer::EmbreeRayTracer* ray_tracer,
	float up_step,
	float down_step,
	float up_slope,
	float down_slope,
	float ground_offset,
	float node_z,
	float node_spacing
	) {
	HF::GraphGenerator::Precision precision = { node_z, node_spacing, ground_offset };
	HF::GraphGenerator::GraphParams params;

	//Setup params struct
	params.up_step = up_step; params.down_step = down_step;
	params.up_slope = up_slope; params.down_slope = down_slope;
	params.precision.ground_offset = ground_offset;
	params.precision.node_z = node_z;
	params.precision.node_spacing = node_spacing;

	auto result = CalculateStepType(*g, HF::RayTracer::MultiRT(ray_tracer), params);

	g->AddEdges(result, "step_type");

	return OK;
}
