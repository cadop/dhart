#include <analysis_C.h>

#include <robin_hood.h>

#include <HFExceptions.h>
#include <embree_raytracer.h>
#include <graph_generator.h>
#include <graph.h>

using HF::SpatialStructures::Graph;
using HF::AnalysisMethods::GraphGenerator;
using namespace HF::Exceptions;


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
	const std::array<float, 3> spacing_array{ spacing[0], spacing[1], spacing[2] };

	Graph* G = new Graph();
	GraphGenerator GraphGen(*ray_tracer, 0);
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
