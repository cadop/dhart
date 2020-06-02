#include <cinterface_utils.h>
#include <vector>
#include <raytracer_C.h>
#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {
		struct Node;
		class Graph;
	}
}
/// <summary> How to aggregate edges the results of view analysis </summary>
enum class AGGREGATE_TYPE {
	/// <summary> Number of rays that hit. </summary>
	COUNT = 0,
	/// <summary> Sum of distances from the origin to each of its hitpoints. </summary>
	SUM = 1,
	/// <summary> Average distance of origin to its hit points. </summary>
	AVERAGE = 2,
	/// <summary> Maximum distance from origin to its hit points. </summary>
	MAX = 3,
	/// <summary> Minimum distance from origin to its hit points. </summary>
	MIN = 4
};
/**
* @defgroup ViewAnalysis
Analyze the view from from points in the environment.
* @{
*/

/// <summary>
/// Perform view analysis from a list of points and aggregate the results into an ordered list of
/// scores. Scores are aggregated by the aggregation specified in AT aggregation method.
/// </summary>
/// <param name="ERT"> Raytracer to fire all rays from. </param>
/// <param name="node_ptr"> Positions to perform view analysis from. </param>
/// <param name="node_size"> Number of nodes in the array pointed at by node_ptr. </param>
/// <param name="max_rays"> Number of rays to evenly distribute in a sphere. </param>
/// <param name="height"> Height to offset nodes from the ground. </param>
/// <param name="AT"> Type of aggregation method to use on distance values. </param>
/// <param name="out_scores"> Output parameter for score vector. </param>
/// <param name="out_scores_ptr"> Output parameter for the score vector's underlying data. </param>
/// <param name="out_scores_size"> Number of elements in the output vector.. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE SphereicalViewAnalysisAggregate(
	HF::RayTracer::EmbreeRayTracer* ERT,
	HF::SpatialStructures::Node* node_ptr,
	int node_size,
	int max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	AGGREGATE_TYPE AT,
	std::vector<float>** out_scores,
	float** out_scores_ptr,
	int* out_scores_size
);

/// <summary>
/// Perform view analysis from a list of points and aggregate the results into an ordered list of
/// scores. Scores are aggregated by the aggregation specified in AT aggregation method.
/// </summary>
/// <param name="ERT"> Raytracer to fire all rays from. </param>
/// <param name="node_ptr">
/// Positions to perform view analysis from as a flat array of floats. Each 3 floats should
/// represent a new point.
/// </param>
/// <param name="node_size">
/// Number of nodes in the array pointed at by node_ptr. Should be equal to the length of node_ptr/3.
/// </param>
/// <param name="max_rays"> Number of rays to evenly distribute in a sphere. </param>
/// <param name="upward_fov"> Maximum degrees upward from the viewer's eye level to consider. </param>
/// <param name="downward_fov"> Maximum degrees downward from the viewer's eye level to consider. </param>
/// <param name="height"> Height to offset nodes from the ground. </param>
/// <param name="AT"> Type of aggregation method to use on distance values. </param>
/// <param name="out_scores"> Output parameter for score vector. </param>
/// <param name="out_scores_ptr"> Output parameter for the score vector's underlying data. </param>
/// <param name="out_scores_size"> Number of elements in the output vector.. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE SphereicalViewAnalysisAggregateFlat(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const float* node_ptr,
	int node_size,
	int max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	AGGREGATE_TYPE AT,
	std::vector<float>** out_scores,
	float** out_scores_ptr,
	int* out_scores_size
);

/// <summary>
/// Perform view analysis and recieve the distance and meshid for each individual ray.
/// </summary>
/// <param name="ERT"> Raytracer to fire rays from. </param>
/// <param name="node_ptr"> Positions to perform view analysis from . </param>
/// <param name="node_size"> Number of nodes in node_ptr. </param>
/// <param name="max_rays"> Number of rays to fire per node. </param>
/// <param name="upward_fov"> Maximum degrees upward from the viewer's eye level to consider. </param>
/// <param name="downward_fov"> Maximum degrees downward from the viewer's eye level to consider. </param>
/// <param name="height"> Height to offset nodes to. </param>
/// <param name="out_results"> Vector to store results in. </param>
/// <param name="out_results_ptr"> Pointer to the result vector's data. </param>
/// <returns> HF_STATUS::OK on vomplryion. </returns>
C_INTERFACE SphericalViewAnalysisNoAggregate(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const HF::SpatialStructures::Node* node_ptr,
	int node_size,
	int* max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	std::vector<RayResult>** out_results,
	RayResult** out_results_ptr
	//std::vector<float>** out_directions
);

/// <summary>
/// Perform view analysis and recieve the distance and meshid for each individual ray.
/// </summary>
/// <param name="ERT"> Raytracer to fire all rays from. </param>
/// <param name="node_ptr">
/// Positions to perform view analysis from as a flat array of floats. Each 3 floats should
/// represent a new point.
/// </param>
/// <param name="node_size">
/// Number of nodes in the array pointed at by node_ptr. Should be equal to the length of node_ptr/3.
/// </param>
/// <param name="max_rays"> Number of rays to evenly distribute in a sphere. </param>
/// <param name="upward_fov"> Maximum degrees upward from the viewer's eye level to consider. </param>
/// <param name="downward_fov"> Maximum degrees downward from the viewer's eye level to consider. </param>
/// <param name="height"> Height to offset nodes from the ground. </param>
/// <param name="out_scores"> Output parameter for score vector. </param>
/// <param name="out_scores_ptr"> Output parameter for the score vector's underlying data. </param>
/// <param name="out_scores_size"> Number of elements in the output vector. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE SphericalViewAnalysisNoAggregateFlat(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const float* node_ptr,
	int node_size,
	int* max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	std::vector<RayResult>** out_results,
	RayResult** out_results_ptr
	//std::vector<float>** out_directions
);

/// <summary> Distribute directions in a sphere </summary>
/// <param name="num_rays"> The number of directions to generate. </param>
/// <param name="out_direction_vector"> Output parameter for the resulting directions. </param>
/// <param name="upward_fov"> Maximum degrees upward from the viewer's eye level to consider. </param>
/// <param name="downward_fov"> Maximum degrees downward from the viewer's eye level to consider. </param>
/// <returns> HF_STATUS::OK On success. </returns>
C_INTERFACE SphericalDistribute(
	int* num_rays,
	std::vector<float>** out_direction_vector,
	float** out_direction_data,
	float upward_fov,
	float downward_fov
);

/**@}*/
