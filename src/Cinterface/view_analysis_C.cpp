#include <view_analysis_C.h>
#include <robin_hood.h>

#include <HFExceptions.h>
#include <view_analysis.h>
#include <embree_raytracer.h>
#include <graph.h>
#include <node.h>

using HF::SpatialStructures::Graph;
using namespace HF::Exceptions;
using HF::RayTracer::EmbreeRayTracer;
using std::vector;
using HF::SpatialStructures::Node;
using namespace HF;

C_INTERFACE SphereicalViewAnalysisAggregate(
	EmbreeRayTracer* ERT, 
	Node * node_ptr,
	int node_size,
	int max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	AGGREGATE_TYPE AT,
	vector<float>** out_scores,
	float** out_scores_ptr, 
	int* out_scores_size)
{
	vector<float>* scores = new vector<float>();
	vector<Node> nodes(node_ptr, node_ptr + node_size);
	
	*scores = ViewAnalysis::SphericalRayshootWithAnyRTForDistance(
		*ERT,
		nodes,
		max_rays,
		upward_fov,
		downward_fov,
		height,
		(ViewAnalysis::AGGREGATE_TYPE)AT
	);

	*out_scores = scores;
	*out_scores_ptr = scores->data();
	*out_scores_size = scores->size(); // Shouldn't this always be the same?

	return OK;
}

C_INTERFACE SphereicalViewAnalysisAggregateFlat(HF::RayTracer::EmbreeRayTracer* ERT, const float* node_ptr, int node_size, int max_rays, float upward_fov, float downward_fov, float height, AGGREGATE_TYPE AT, std::vector<float>** out_scores, float** out_scores_ptr, int* out_scores_size)
{
	vector<float>* scores = new vector<float>();
	vector<std::array<float, 3>> nodes = ConvertRawFloatArrayToPoints(node_ptr, node_size);

	*scores = ViewAnalysis::SphericalRayshootWithAnyRTForDistance(
		*ERT,
		nodes,
		max_rays,
		upward_fov,
		downward_fov,
		height,
		(ViewAnalysis::AGGREGATE_TYPE)AT
	);

	*out_scores = scores;
	*out_scores_ptr = scores->data();
	*out_scores_size = scores->size(); // Shouldn't this always be the same?

	return OK;
}


C_INTERFACE SphericalViewAnalysisNoAggregate(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const HF::SpatialStructures::Node* node_ptr,
	int node_size,
	int * max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	std::vector<RayResult> ** out_results,
	RayResult ** out_results_ptr
	//std::vector<float>** out_directions
	)
{
	
	vector<RayResult> * scores = new vector<RayResult>();
	vector<Node> nodes(node_ptr, node_ptr + node_size);
	
	*scores = ViewAnalysis::SphericalViewAnalysis<RayResult>(
		*ERT,
		nodes,
		*max_rays,
		upward_fov,
		downward_fov,
		height
	);

	*out_results = scores;
	*out_results_ptr = scores->data();
	*max_rays = scores->size()/node_size;
	//printf("Score size: %d\n", *max_rays);
	return OK;
}

C_INTERFACE SphericalViewAnalysisNoAggregateFlat(HF::RayTracer::EmbreeRayTracer* ERT, const float* node_ptr, int node_size, int* max_rays, float upward_fov, float downward_fov, float height, std::vector<RayResult>** out_results, RayResult** out_results_ptr)
{
	vector<RayResult>* scores = new vector<RayResult>();
	auto nodes = ConvertRawFloatArrayToPoints(node_ptr,node_size);

	*scores = ViewAnalysis::SphericalViewAnalysis<RayResult>(
		*ERT,
		nodes,
		*max_rays,
		upward_fov,
		downward_fov,
		height
		);

	*out_results = scores;
	*out_results_ptr = scores->data();
	*max_rays = scores->size() / node_size;
	return OK;
}

C_INTERFACE SphericalDistribute(
	int * num_rays, 
	vector<float>** out_direction_vector,
	float** out_direction_data,
	float upward_fov,
	float downward_fov) 
{
	*out_direction_vector = new vector<float>();
	auto array_array = ViewAnalysis::FibbonacciDistributePoints(*num_rays, upward_fov, downward_fov);
	
	auto& out_directions = **out_direction_vector;
	
	out_directions.resize(array_array.size()*3);
	for (int i = 0; i < array_array.size(); i += 1)
	{
		int os = i * 3;
		int y_idx = os + 1;
		int z_idx = os + 2;
		const auto& arr = array_array[i];
		out_directions[os] = arr[0];
		out_directions[y_idx] = arr[1];
		out_directions[z_idx] = arr[2];
	}

	*out_direction_data = out_directions.data();
	*num_rays = array_array.size();
	return OK;
}


