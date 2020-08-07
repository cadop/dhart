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

/*!
	\defgroup RayTracer
	Perform efficient ray intersections using Intel's Embree Library.

	@{

	<b> Raytracer setup: </b><br>
	Every example below will be assumed to begin with this body of code;<br>
	we will call it the 'setup':<br>

	First, begin by loading the .OBJ file:<br>
	\code
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());

		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

		// Load mesh
		// The array rot will rotate the mesh 90 degrees with respect to the x-axis,
		// i.e. makes the mesh 'z-up'.
		//
		// Notice that we pass the address of the loaded_obj pointer
		// to LoadOBJ. We do not want to pass loaded_obj by value, but by address --
		// so that we can dereference it and assign it to the address of (pointer to)
		// the free store memory allocated within LoadOBJ.
		const float rot[] = { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
	\endcode

	Then, create the BVH:<br>
	\code
		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);
	\endcode

	At this point, you are ready to use your BVH.<br>
	All examples below will assume you have already created a BVH from the .OBJ file provided.<br>
	(all examples below begin with the setup code described above)
	
	<b> Raytracer teardown: </b><br>
	When you are finished with the BVH, you must then release its memory resources:<br>
	\code
		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}
	\endcode

	After destroying the BVH, you must also do the same for the (vector<MeshInfo> *) used by LoadOBJ.
	\code
		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode

	The client is responsible for releasing the memory for<br>
	the mesh (vector<MeshInfo> *) and the BVH (EmbreeRayTracer *).<br>
	Every example for each function should be followed up by the 'teardown' code described above.
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
				HF_STATUS::GENERIC_ERROR if <paramref name="mesh"/> is null.

	\see	Raytracer setup, Raytracer teardown
*/
C_INTERFACE CreateRaytracer(std::vector<HF::Geometry::MeshInfo>* mesh, HF::RayTracer::EmbreeRayTracer** out_raytracer);

/*!
	\brief		Delete an existing raytracer.

	\param	rt_to_destroy	Raytracer to destroy
	
	\returns	HF::OK on completion.

	\see	Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRaysDistance

	\see Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireSingleRayDistance

	\see Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRay

	\see Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleRays

	\see Raytracer setup, Raytracer teardown
*/
C_INTERFACE FireMultipleRays(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* directions, int size, float max_distance, bool* result_array);

/*!
	\brief		Fire rays from each origin point in the given direction.

	\param	ert				The raytracer to fire each ray from.
	\param	origins			A list of floats representing origin points, with each 3 floats representing one point.
	\param	direction		An array of 3 floats representing the X, Y, and Z coordinates respectively.
	\param	size			Number of points and directions, equal to the total number of floats in one array / 3.
	\param	max_distance	Maximum distance a ray can travel and still hit a target. 

	\param	result_array	Output parameter conatining an ordered list of booleans set to true if the their rays hit, 
							and false if their rays did not.

	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleOriginsOneDirection

	\see Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleDirectionsOneOrigin

	\see Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireOcclusionRays

	\see Raytracer setup, Raytracer teardown
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

	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_DestroyRayResultVector

	\see Raytracer setup, Raytracer teardown, FireRaysDistance
*/
C_INTERFACE DestroyRayResultVector(
	std::vector<RayResult>* analysis
);

/**@}*/
