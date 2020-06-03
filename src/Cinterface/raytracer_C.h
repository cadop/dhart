#pragma once
#include <vector>
#include <array>

namespace HF {
	namespace RayTracer {
		class MeshInfo;
		class EmbreeRayTracer;
	}
	namespace Geometry {
		class MeshInfo;
	}
}

#define C_INTERFACE extern "C" __declspec(dllexport) int

/**
* @defgroup RayTracer
* Perform efficient ray intersections using Intel's Embree Library.
* @{
*/
/// <summary>
/// The result of firing a ray at an object. Contains distance to the hitpoint and the ID of the mesh.
/// </summary>
struct RayResult {
	float distance = -1;
	int meshid = -1;

	/// <summary>
	/// Update this result based on the ray intesection. Similar structures can be created to
	/// support different sethit methods.
	/// </summary>
	/// <param name="node"> Not used for this structure. </param>
	/// <param name="direction"> Not used for this structure. </param>
	/// <param name="dist"> Distance from node to the hit_point. </param>
	/// <param name="mid"> ID of the hit mesh. </param>
	template <typename N, typename V>
	void SetHit(const N& node, const V& direction, float dist, int mid) {
		distance = dist;
		meshid = mid;
	}
};

/// <summary> Create a new raytracer using several meshes. </summary>
/// <param name="mesh"> The meshes to add to raytracer's BVH. </param>
/// <param name="out_raytracer"> Output parameter for the new raytracer. </param>
/// <returns>
/// HF_STATUS::MISSING_DEPEND if Embree's dll couldn't be found. HF_STATUS::GENERIC_ERROR if
/// <paramref name="mesh" /> is null.
/// </returns>
C_INTERFACE CreateRaytracer(std::vector<HF::Geometry::MeshInfo>* mesh, HF::RayTracer::EmbreeRayTracer** out_raytracer);

/// <summary> Delete an existing raytracer. </summary>
/// <param name="rt_to_destroy"> Raytracer to destroy. </param>
/// <returns> HF::OK on completion. </returns>
C_INTERFACE DestroyRayTracer(HF::RayTracer::EmbreeRayTracer* rt_to_destroy);

/// <summary>
/// Fire rays for each node in origins/directions as ordered pairs and get distance back as a result.
/// </summary>
/// <param name="ert"> The raytracer to use for firing every ray. </param>
/// <param name="origins">
/// An array of origin points to fire rays from. Should be an array of floats with every 3 floats
/// representing a new origin point.
/// </param>
/// <param name="num_origins">
/// The number of points in origins. NOTE: This should be equal to the length of origins/3, since
/// every 3 floats in origins equals a single point.
/// </param>
/// <param name="directions">
/// An array of directions points to fire rays from. Should be an array of floats with every 3
/// floats representing a new direction.
/// </param>
/// <param name="num_directions">
/// The number of directions in directions. NOTE: This should be equal to the length of
/// directions/3, since every 3 floats in directions equals a single direction.
/// </param>
/// <param name="out_results"> Output parameter for ray results. </param>
/// <param name="results_data">
/// Output parameter for the data of the array held by <paramref name="out_results" />
/// </param>
/// <returns>
/// HF_STATUS::OK on completion. HF::GENERICE_ERROR if the input parameters didn't meet atleast one
/// of the required cases below.
/// </returns>
/// <remarks>
/// <para> Can be fired in 3 configurations: </para>
/// <list type="bullet">
/// <item>
/// <font color="#2a2a2a"> Equal amount of directions/origins: Fire a ray for every pair of
/// origin/direction in order. i.e. (origin[0], direction[0]), (origin[1], direction[1]). </font>
/// </item>
/// <item>
/// <font color="#2a2a2a"> One direction, multiple origins: Fire a ray in the given direction from
/// each origin point in origins. </font>
/// </item>
/// <item>
/// <font color="#2a2a2a"> One origin, multiple directions: Fire a ray from the origin point in each
/// direction in directions. </font>
/// </item>
/// </list>
/// </remarks>
C_INTERFACE FireRaysDistance(
	HF::RayTracer::EmbreeRayTracer* ert,
	float* origins,
	int num_origins,
	float* directions,
	int num_directions,
	std::vector<RayResult>** out_results,
	RayResult** results_data
);

/// <summary>
/// Fire a single ray and get the distance to its hit and the meshID if it hit anything. If it
/// missed, then distance and meshid will both be -1.
/// </summary>
/// <param name="ert"> The ray tracer to fire from. </param>
/// <param name="origin"> The origin point to fire from. </param>
/// <param name="direction"> The direction to fire the ray in. </param>
/// <param name="max_distance">
/// Maximum distance to record a hit within. Any hits beyond this distance will not be counted.
/// </param>
/// <param name="out_distance">
/// Out parameter for Distance to the hitpoint. Will be set to -1 if the ray didn't hit anything.
/// </param>
/// <param name="out_meshid">
/// Out parameter for the ID of the hit mesh. Will be set to -1 if the ray didn't hit anything.
/// </param>
/// <returns> HF_STATUS::OK on success. </returns>
C_INTERFACE FireSingleRayDistance(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* origin,
	const float* direction,
	const float max_distance,
	float* out_distance,
	int* out_meshid
);

/// <summary> Fire a single ray from the raytracer and recieve a point in return. </summary>
/// <param name="ert"> Raytracer to fire each ray from. </param>
/// <param name="x">
/// x coordinate of the ray's origin. Will be set to the hit point's x coordinate if the ray something.
/// </param>
/// <param name="y">
/// y coordinate of the ray's origin. Will be set to the hit point's y coordinate if the ray something.
/// </param>
/// <param name="z">
/// z coordinate of the ray's origin. Will be set to the hit point's z coordinate if the ray something.
/// </param>
/// <param name="dx"> x coordinate of the ray's direction. </param>
/// <param name="dy"> y coordinate of the ray's direction. </param>
/// <param name="dz"> z coordinate of the ray's direction. </param>
/// <param name="max_distance">
/// Maximum distance to record a hit within. Any hits beyond this distance will not be counted.
/// </param>
/// <param name="result"> Set to true if the ray hits, false otherwise </param>
/// <returns> HF::OK on completion. </returns>
C_INTERFACE FireRay(HF::RayTracer::EmbreeRayTracer* ert, float& x, float& y, float& z, float dx, float dy, float dz, float max_distance, bool& result);

/// <summary>
/// Fire multiple rays at once in parallel and recieve their hitpoints in return. The number of
/// directions must be equal to the number of origins.
/// </summary>
/// <param name="ert"> Raytracer to fire each ray from. </param>
/// <param name="origins">
/// A list of floats representing origin points, with each 3 floats representing one point. If the
/// ray fired from a point is successful, said point will be overwritten with the place it hit.
/// </param>
/// <param name="directions">
/// A list of floats representing ray directions, with each 3 floats representing one direction.
/// </param>
/// <param name="size">
/// Number of points and directions, equal to the total number of floats in one array / 3.
/// </param>
/// <param name="max_distance">
/// Maximum distance a ray can travel and still hit a target. Any hits beyond this point will not be recorded.
/// </param>
/// <param name="result_array">
/// Output parameter conatining an ordered list of booleans set to true if the their rays hit, and
/// false if their rays did not.
/// </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE FireMultipleRays(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* directions, int size, float max_distance, bool* result_array);

/// <summary> Fire rays from each origin point in the given direction. </summary>
/// <param name="ert"> The raytracer to fire each ray from. </param>
/// <param name="origins">
/// A list of floats representing origin points, with each 3 floats representing one point.
/// </param>
/// <param name="direction"> An array of 3 floats representing the X, Y, and Z coordinates respectively. </param>
/// <param name="size">
/// Number of points and directions, equal to the total number of floats in one array / 3.
/// </param>
/// <param name="max_distance"> Maximum distance a ray can travel and still hit a target. </param>
/// <param name="result_array">
/// Output parameter conatining an ordered list of booleans set to true if the their rays hit, and
/// false if their rays did not.
/// </param>
C_INTERFACE FireMultipleOriginsOneDirection(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* direction, int size, float max_distance, bool* result_array);

/// <summary>
/// Fire rays from a single origin point in multiple directions and get a the points where they
/// intersected the geometry.
/// </summary>
/// <param name="ert"> A pointer to a valid embree raytracer. </param>
/// <param name="origin">
/// An array of 3 floats representing the X, Y, and Z coordinates of the origin to fire from respectively.
/// </param>
/// <param name="directions">
/// A list of floats representing directions, with each 3 floats representing one direction. If a
/// ray fired in a direction resulted in a hit, that directionwill be overwritten to the hitpoint.
/// </param>
/// <param name="size">
/// Number of points and directions, equal to the total number of floats in one array / 3
/// </param>
/// <param name="max_distance"> Maximum distance a ray can travel and still hit a target </param>
/// <param name="result_array">
/// Output parameter conatining an ordered list of booleans set to true if the their rays hit, and
/// false if their rays did not.
/// </param>
C_INTERFACE FireMultipleDirectionsOneOrigin(HF::RayTracer::EmbreeRayTracer* ert, const float* origin, float* directions, int size, float max_distance, bool* result_array);

/// <summary> Fire one or more occlusion rays in parallel. </summary>
/// <param name="ert"> A pointer to a valid embree raytracer </param>
/// <param name="origins">
/// A list of floats representing origin points, with each 3 floats representing one point
/// </param>
/// <param name="directions">
/// a list of floats representing ray directions, with each 3 floats representing one direction
/// </param>
/// <param name="origin_size">
/// How many origins points are included are included. Note that a single origin point is 3 floats,
/// so this should equal the length of the origin array/3. This must match direction_size or be
/// equal to one.
/// </param>
/// <param name="direction_size">
/// How many directions are included. Note that a single direction is 3 floats, so this should equal
/// the length of the origin array/3. This must match origin_size or be equal to one.
/// </param>
/// <param name="max_distance"> Maximum distance a ray can travel and still hit a target. </param>
/// <param name="result_array"> Output array booleans </param>
/// <remarks>
/// Occlusion rays are noticably faster than standard rays but are only capable of returning whether
/// they hit something or not. This makes them good for line of sight checks.
/// </remarks>
C_INTERFACE FireOcclusionRays(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* origins,
	const float* directions,
	int origin_size,
	int direction_size,
	float max_distance,
	bool* result_array
);

/// <summary> Destroy a vector of rayresults. </summary>
/// <param name="analysis"> The ray results to destroy. </param>
C_INTERFACE DestroyRayResultVector(
	std::vector<RayResult>* analysis
);

/**@}*/