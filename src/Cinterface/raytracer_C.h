/*!
	\file		raytracer_C.h
	\brief		C Interface header file for Raytracer functionality

	\author		TBA
	\date		04 Aug 2020
*/

#ifndef RAYTRACER_C_H
#define RAYTRACER_C_H

#include <vector>
#include <array>

namespace HF {
	namespace RayTracer {
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
	\section raytracer_setup Raytracer setup
	Every example below will be assumed to begin with this body of code;<br>
	we will call it the 'setup':<br>

	First, begin by <b>loading the .OBJ file</b>:<br>
	\ref mesh_setup (from \link objloader_C.h \endlink)

	Then, <b>create the BVH</b>:<br>
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

		if (status != -1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}
	\endcode

	At this point, <b>you are ready to use your BVH</b>.<br>
	All examples below will assume you have already created a BVH from the .OBJ file provided.<br>
	(all examples below begin with the setup code described above)
	
	\section raytracer_teardown Raytracer teardown
	When you are finished with the BVH, you must then <b>release its memory resources</b>:<br>
	\code
		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}
	\endcode

	After destroying the BVH,<br>
	you must also do the same for the (vector<\link HF::Geometry::MeshInfo \endlink> *) used by LoadOBJ.<br>
	\ref mesh_teardown (from \link objloader_C.h \endlink)

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019C4EA752E0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019C4EA12820, code: 1`\n

	<br>
	<b>The client is responsible for releasing the memory for<br>
	the mesh (vector<\link HF::Geometry::MeshInfo \endlink> *) and the BVH (\link HF::RayTracer::EmbreeRayTracer \endlink *).</b><br>
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
	\param use_precise If true, use a more precise but slower method of triangle intersections
	
	\returns	HF_STATUS::MISSING_DEPEND if Embree's dll couldn't be found. 
				HF_STATUS::GENERIC_ERROR if <paramref name="mesh"/> is null.

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
*/
C_INTERFACE CreateRaytracer(
	HF::Geometry::MeshInfo * mesh, 
	HF::RayTracer::EmbreeRayTracer ** out_raytracer,
	bool use_precise
);

/*!
	\brief		 Create a new raytracer using several meshes.

	\param	mesh			The meshes to add to raytracer's BVH.
	\param num_meshes	Number of meshes in meshes
	\param	out_raytracer	Output parameter for the new raytracer.

	\returns	HF_STATUS::MISSING_DEPEND if Embree's dll couldn't be found.
				HF_STATUS::GENERIC_ERROR if `mesh` is null.
*/
C_INTERFACE CreateRaytracerMultiMesh(
	HF::Geometry::MeshInfo** meshes,
	int num_meshes,
	HF::RayTracer::EmbreeRayTracer** out_raytracer,
	bool use_precise
);


/*!
	\brief Add a new mesh to a raytracer.

	\param ERT raytracer to add the mesh to
	\param MI MeshInfo to add to the raytracer. Will try to maintain IDs, however if there is a collision, then the
			  MeshInfo will be updated to contain the ID assigned to it by the raytracer. 

	\returns HF_STATUS::OK On completion
*/
C_INTERFACE AddMesh(
	HF::RayTracer::EmbreeRayTracer* ERT,
	HF::Geometry::MeshInfo* MI
);


/*!
	\brief Add a new mesh to a raytracer.

	\param ERT raytracer toa dd the mesh to
	\param MI MeshInfo to add to the raytracer. Will try to maintain IDs, however if there is a collision, then each
			  MeshInfo will be updated to contain the ID assigned to it by the raytracer.
	\param number_of_meshes Number of meshes in `MI`.

	\returns HF_STATUS::OK On completion
*/
C_INTERFACE AddMeshes(
	HF::RayTracer::EmbreeRayTracer* ERT,
	HF::Geometry::MeshInfo ** MI,
	int number_of_meshes
);


/*!
	\brief		Delete an existing raytracer.

	\param	rt_to_destroy	Raytracer to destroy
	
	\returns	HF::OK on completion.

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
*/
C_INTERFACE DestroyRayTracer(HF::RayTracer::EmbreeRayTracer* rt_to_destroy);

/*!
	\brief		Fire rays for each node in origins/directions as ordered pairs and get distance back as a result.

	\param	ert				The raytracer to use for firing every ray. 

	\param	origins			An array of origin points to fire rays from. Should be an array of floats with every 3 floats
							representing a new origin point.

	\param	num_origins		The number of points in origins. NOTE: This should be equal to the length of origins / 3, since
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

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	Define the <b>vector components</b> of the <b>ray</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRaysDistance_points_components

	Prepare a <b>pointer</b> to std::vector<\link RayResult \endlink>, and a pointer to a \link RayResult \endlink .<br>
	Then, invoke</b> \link FireRaysDistance \endlink .<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRaysDistance

	We can <b>review the results</b> by outputting the contents of the <b>results container</b>:<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRaysDistance_results

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019C4EA752E0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019C4EA12280, code: 1`\n
	`>>> Ray result: [{Distance: 2`\n
	`>>> Mesh ID: 0}, {Distance: 1.5`\n
	`>>> Mesh ID: 0}, {Distance: 1.33333`\n
	`>>> Mesh ID: 0}]`\n
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
	\brief		Fire a single ray and get the distance to its hit and the mesh ID if it hit anything. 
				If it missed, then distance and meshid will both be -1.

	\param	ert				The ray tracer to fire from.
	\param	origin			The origin point to fire from.
	\param	direction		The direction to fire the ray in.
	\param	max_distance	Maximum distance to record a hit within. Any hits beyond this distance will not be counted.
	\param	out_distance	Out parameter for Distance to the hitpoint. Will be set to -1 if the ray didn't hit anything.
	\param	out_meshid		Out parameter for the ID of the hit mesh. Will be set to -1 if the ray didn't hit anything.

	\returns	HF_STATUS::OK on success

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	Define the <b>vector components</b> of the <b>ray</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireSingleRayDistance_points_dir

	Define a <b>max_distance</b>.<br>
	Also create variables <b>distance</b> and <b>mesh_id</b> and initialize them as shown.<br>
	They will be mutated if a <b>hit</b> occurs.<br>
	Then, <b>invoke</b> \link FireSingleRayDistance \endlink
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireSingleRayDistance

	If <b>mesh_id</b> is not <b>-1</b>, and <b>distance</b> is not <b>-1</b>, a <b>hit</b> was made.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireSingleRayDistance_results

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019C4EA3FCB0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019C4EA12820, code: 1`\n
	`>>> Distance is 2, meshid is 0`\n
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

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	Define the <b>vector components</b> of the <b>ray</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRay_points_direction

	Define a <b>maximum distance value</b>, and a <b>hit point</b> that determines the ray's <b>intended destination</b>.</b>.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRay_distance_hit_point

	<b>Invoke</b> \link PointIntersection \endlink . <b>did_hit</b> will be set <b>true</b> if a hit occurred.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireRay

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019E906599A0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019E86D22BE0, code: 1`\n
	`>>> Hit point: [0, 0, 0]`\n
*/
C_INTERFACE PointIntersection(HF::RayTracer::EmbreeRayTracer* ert, float& x, float& y, float& z, float dx, float dy, float dz, float max_distance, bool& result);

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

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	Define the <b>vector components</b> of the <b>ray</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleRays_points_directions

	Define a <b>maximum distance value</b>, and a <b>container</b> to store the <b>ray collision results</b>.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleRays

	We can <b>review the results</b> by outputting the contents of the <b>results container</b>:<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleRays_results

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019E906596A0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019E86D22500, code: 1`\n
	`>>> result[0]: hit`\n
	`>>> [0, 0, 0], direction [0, 0, -1]`\n
	`>>> result[1]: hit`\n
	`>>> [0, 0, -2.38419e-07], direction [0, 0, -2]`\n
	`>>> result[2]: hit`\n
	`>>> [0, 0, 2.38419e-07], direction [0, 0, -3]`\n
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

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleOriginsOneDirection_start_point

	Define the <b>vector components</b> of the <b>ray</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleOriginsOneDirection_direction

	Define a <b>maximum distance value</b>, and a <b>container</b> to store the <b>ray collision results</b>.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleOriginsOneDirection

	We can <b>review the results</b> by outputting the contents of the <b>results container</b>:<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleOriginsOneDirection_results

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019E90659220, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019E86D22BE0, code: 1`\n
	`>>> result[0]: hit`\n
	`>>> [0, 0, -1], from point [0, 0, 0]`\n
	`>>> result[1]: hit`\n
	`>>> [0, 0, -1], from point [0, 0, -2.38419e-07]`\n 
	`>>> result[2]: hit`\n
	`>>> [0, 0, -1], from point [0, 0, 0]`\n
*/
C_INTERFACE FireMultipleOriginsOneDirection(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* direction, int size, float max_distance, bool* result_array);

/*!
	\brief	Fire rays from a single origin point in multiple directions and get a the points where they intersected the geometry.

	\param	ert				A pointer to a valid embree raytracer.
	\param	origin			An array of 3 floats representing the X, Y, and Z coordinates of the origin to fire from respectively

	\param	directions		A list of floats representing directions, with each 3 floats representing one direction. If a
							ray fired in a direction resulted in a hit, that direction will be overwritten to the hitpoint.

	\param	size			Number of points and directions, equal to the total number of floats in one array / 3
	\param	max_distance	Maximum distance a ray can travel and still hit a target
	\param	result_array	Output parameter conatining an ordered list of booleans set to true if the their rays hit, and false if their rays did not.

	\param	returns			HF_STATUS::OK on completion.

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	Define the <b>vector components</b> of the <b>ray(s)</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleDirectionsOneOrigin_start_point_direction

	Define a <b>maximum distance value</b>, and a <b>container</b> to store the <b>ray collision results</b>.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleDirectionsOneOrigin

	We can <b>review the results</b> by outputting the contents of the <b>results container</b>:<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireMultipleDirectionsOneOrigin_results

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019E906587A0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019E86D22500, code: 1`\n
	`>>> result[0]: hit`\n
	`>>> [0, 0, 0], direction [0, 0, 0]`\n
	`>>> result[1]: hit`\n
	`>>> [0, 0, 0], direction [0, 0, 0]`\n
	`>>> result[2]: hit`\n
	`>>> [0, 0, 0], direction [0, 0, 1.19209e-07]`\n
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
							the length of the origin array / 3. This must match origin_size or be equal to one.

	\param	max_distance	Maximum distance a ray can travel and still hit a target.
	\param	result_array	Output array booleans

	\returns		HF_STATUS::OK on completion

	\remarks	Occlusion rays are noticably faster than standard rays but are only capable of returning whether
				they hit something or not. This makes them good for line of sight checks.

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	Begin by <b>loading an .obj file</b> (\ref mesh_setup).<br>
	Then, <b>create a BVH</b> (\ref raytracer_setup) using the <b>mesh</b>.<br>

	Define the <b>starting points</b> from where the ray will fire.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireOcclusionRays_start_point

	Define the <b>vector components</b> of the <b>ray(s)</b> that will be fired.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireOcclusionRays_components

	Define a <b>maximum distance value</b>, and a <b>container</b> to store the <b>ray occlusion results</b>.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireOcclusionRays

	If <b>does_occlude</b> is true, the ray <b>connects</b>.<br>
	\snippet tests\src\embree_raytracer_cinterface.cpp snippet_FireOcclusionRays_result

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used in this example --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::RayTracer::EmbreeRayTracer \endlink *) instances.

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 0000019E90658440, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 0000019E86D22BE0, code: 1`\n
	`>>> Using multidirection, single origin`\n
	`>>> Does the ray connect? True`\n
*/
C_INTERFACE Occlusions(
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

	\see	\ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH), FireRaysDistance
*/
C_INTERFACE DestroyRayResultVector(
	std::vector<RayResult>* analysis
);

C_INTERFACE PreciseIntersection(HF::RayTracer::EmbreeRayTracer* RT, double x, double y, double z, double dx, double dy, double dz, double * out_distance);

/**@}*/

#endif /* RAYTRACER_C_H */
