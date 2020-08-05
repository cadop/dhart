/*!
	\file		raytracer_C.h
	\brief		C Interface header file for Raytracer functionality

	\author		TBA
	\date		04 Aug 2020
*/
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
/*!
	\brief	The result of firing a ray at an object. Contains distance to the hitpoint and the ID of the mesh.
*/
struct RayResult {
	float distance = -1;
	int meshid = -1;

	/*!
		\brief	Update this result based on the ray intersection. Similar structures can be created
				to support different sethit functions.

		\param	node		Not used for this structure
		\param	direction	Not used for this structure
		\param	dist		Distance from node to the hit_point
		\param	mid			ID of the hit mesh
	*/
	template <typename N, typename V>
	void SetHit(const N& node, const V& direction, float dist, int mid) {
		distance = dist;
		meshid = mid;
	}
};

/*!
	\brief		 Create a new raytracer using several meshes.

	\param	mesh			The meshes to add to raytracer's BVH. 
	\param	out_raytracer	Output parameter for the new raytracer.
	
	\returns	HF_STATUS::MISSING_DEPEND if Embree's dll couldn't be found. 
				HF_STATUS::GENERIC_ERROR if <paramref name="mesh" /> is null.

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_CreateRayTracer
*/
C_INTERFACE CreateRaytracer(std::vector<HF::Geometry::MeshInfo>* mesh, HF::RayTracer::EmbreeRayTracer** out_raytracer);

/*!
	\brief		Delete an existing raytracer.

	\param	rt_to_destroy	Raytracer to destroy
	
	\returns	HF::OK on completion.

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_DestroyRayTracer
*/
C_INTERFACE DestroyRayTracer(HF::RayTracer::EmbreeRayTracer* rt_to_destroy);

/*!
	\brief		Fire rays for each node in origins/directions as ordered pairs and get distance back as a result.

	\param	ert				The raytracer to use for firing every ray. 

	\param	origins			An array of origin points to fire rays from. Should be an array of floats with every 3 floats
							representing a new origin point.

	\param	num_origins		The number of points in origins. NOTE: This should be equal to the length of origins/3, since
							every 3 floats in origins equals a single point.

	\param	directions		An array of directions points to fire rays from. 
							Should be an array of floats with every 3 floats representing a new direction.

	\param	num_directions	The number of directions in directions. NOTE: This should be equal to the length of
							directions / 3, since every 3 floats in directions equals a single direction.

	\param	out_results		Output parameter for ray results. 

	\param	results_data	Output parameter for the data of the array held by <paramref name="out_results" />

	\returns  HF_STATUS::OK on completion. 
			  HF::GENERIC_ERROR if the input parameters didn't meet at least one of the required cases below.

	\remarks
	<para> Can be fired in 3 configurations: </para>
	
	<list type="bullet">
	
		<item>
		Equal amount of directions/origins: Fire a ray for every pair of
		origin/direction in order. i.e. (origin[0], direction[0]), (origin[1], direction[1]).
		</item>

		<item>
		One direction, multiple origins: Fire a ray in the given direction from
		each origin point in origins.
		</item>
	
		<item>
		One origin, multiple directions: Fire a ray from the origin point in each
		direction in directions.
		</item>
	
	</list>

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Required definitions for the example below:<br>
		\link RayResult \link <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireRaysDistance \endlink <br>
		\link DestroyRayResultVector \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRaysDistance
*/
C_INTERFACE FireRaysDistance(
	HF::RayTracer::EmbreeRayTracer* ert,
	float* origins,
	int num_origins,
	float* directions,
	int num_directions,
	std::vector<RayResult>** out_results,
	RayResult** results_data
);

/*!
	\brief		Fire a single ray and get the distance to its hit and the meshID if it hit anything. 
				If it missed, then distance and meshid will both be -1.

	\param	ert		The ray tracer to fire from.

	\param	origin	The origin point to fire from.

	\param	direction	The direction to fire the ray in.

	\param	max_distance	Maximum distance to record a hit within. Any hits beyond this distance will not be counted.

	\param	out_distance	Out parameter for Distance to the hitpoint. Will be set to -1 if the ray didn't hit anything.

	\param	out_meshid	Out parameter for the ID of the hit mesh. Will be set to -1 if the ray didn't hit anything.

	\returns	HF_STATUS::OK on success

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireSingleRayDistance \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireSingleRayDistance
*/
C_INTERFACE FireSingleRayDistance(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* origin,
	const float* direction,
	const float max_distance,
	float* out_distance,
	int* out_meshid
);

/*!
	\brief		Fire a single ray from the raytracer and receive a point in return. 

	\param	ert		Raytracer to fire each ray from.
	\param	x		x coordinate of the ray's origin. Will be set to the hit point's x coordinate if the ray something.
	\param	y		y coordinate of the ray's origin. Will be set to the hit point's y coordinate if the ray something.
	\param	z		z coordinate of the ray's origin. Will be set to the hit point's z coordinate if the ray something.
	\param	dx		x coordinate of the ray's direction.
	\param	dy		y coordinate of the ray's direction.
	\param	dz		z coordinate of the ray's direction.
	\param	max_distance	Maximum distance to record a hit within. Any hits beyond this distance will not be counted.
	\param	result			Set to true if the ray hits, false otherwise

	\returns	HF::OK on completion

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireRay \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRay
*/
C_INTERFACE FireRay(HF::RayTracer::EmbreeRayTracer* ert, float& x, float& y, float& z, float dx, float dy, float dz, float max_distance, bool& result);

/*!
	\brief		Fire multiple rays at once in parallel and receive their hitpoints in return. The number of
				directions must be equal to the number of origins.

	\param	ert				Raytracer to fire each ray from.

	\param	origins			A list of floats representing origin points, with each 3 floats representing one point. If the
							ray fired from a point is successful, said point will be overwritten with the place it hit.

	\param	directions		A list of floats representing ray directions, with each 3 floats representing one direction.
	\param	size			Number of points and directions, equal to the total number of floats in one array / 3.			
	\param	max_distance	Maximum distance a ray can travel and still hit a target. Any hits beyond this point will not be recorded.

	\param	result_array	Output parameter containing an ordered list of booleans set to true if the their rays hit, and
							false if their rays did not.

	\returns	HF_STATUS::OK on completion.

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireMultipleRays \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleRays
*/
C_INTERFACE FireMultipleRays(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* directions, int size, float max_distance, bool* result_array);

/*!
	\brief		Fire rays from each origin point in the given direction.

	\param	ert				The raytracer to fire each ray from.
	\param	origins			A list of floats representing origin points, with each 3 floats representing one point.
	\param	direction		An array of 3 floats representing the X, Y, and Z coordinates respectively.
	\param	size			Number of points and directions, equal to the total number of floats in one array / 3.
	\param	max_distance	Maximum distance a ray can travel and still hit a target. 

	\result_array	Output parameter conatining an ordered list of booleans set to true if the their rays hit, 
					and false if their rays did not.

	\returns	HF_STATUS::OK on completion.

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireMultipleOriginsOneDirection \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleOriginsOneDirection
*/
C_INTERFACE FireMultipleOriginsOneDirection(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* direction, int size, float max_distance, bool* result_array);

/*!
	\brief	Fire rays from a single origin point in multiple directions and get a the points where they intersected the geometry.

	\param	ert				A pointer to a valid embree raytracer.
	\param	origin			An array of 3 floats representing the X, Y, and Z coordinates of the origin to fire from respectively

	\param	directions		A list of floats representing directions, with each 3 floats representing one direction. If a
							ray fired in a direction resulted in a hit, that directionwill be overwritten to the hitpoint.

	\param	size			Number of points and directions, equal to the total number of floats in one array / 3
	\param	max_distance	Maximum distance a ray can travel and still hit a target
	\param	result_array	Output parameter conatining an ordered list of booleans set to true if the their rays hit, and false if their rays did not.

	\param	returns			HF_STATUS::OK on completion.

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireMultipleDirectionsOneOrigin \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleDirectionsOneOrigin
*/
C_INTERFACE FireMultipleDirectionsOneOrigin(HF::RayTracer::EmbreeRayTracer* ert, const float* origin, float* directions, int size, float max_distance, bool* result_array);

/*!
	\brief		Fire one or more occlusion rays in parallel.

	\param	ert				A pointer to a valid embree raytracer
	\param	origins			A list of floats representing origin points, with each 3 floats representing one point
	\param	directions		A list of floats representing ray directions, with each 3 floats representing one direction

	\param	origin_size		How many origins points are included are included. Note that a single origin point is 3 floats,
							so this should equal the length of the origin array/3. This must match direction_size or be equal to one.

	\param	direction_size	How many directions are included. Note that a single direction is 3 floats, so this should equal
							the length of the origin array/3. This must match origin_size or be equal to one.

	\param	max_distance		Maximum distance a ray can travel and still hit a target.
	\param	result_array		Output array booleans

	\returns		HF_STATUS::OK on completion

	\remarks	Occlusion rays are noticably faster than standard rays but are only capable of returning whether
				they hit something or not. This makes them good for line of sight checks.

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireOcclusionRays \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireOcclusionRays
*/
C_INTERFACE FireOcclusionRays(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* origins,
	const float* directions,
	int origin_size,
	int direction_size,
	float max_distance,
	bool* result_array
);

/*!
	\brief	Destroy a vector of rayresults.

	\param	analysis	The ray results to destroy

	\returns	HF_STATUS::OK on completion

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireRaysDistance \endlink <br>
		\link DestroyRayResultVector \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_DestroyRayResultVector
*/
C_INTERFACE DestroyRayResultVector(
	std::vector<RayResult>* analysis
);

/**@}*/
