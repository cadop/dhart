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

		\code
			// TODO example
		\endcode
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

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		//
		//	Use bvh
		//

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		//
		//	Use bvh
		//

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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
	<font color="#2a2a2a"> Equal amount of directions/origins: Fire a ray for every pair of
	origin/direction in order. i.e. (origin[0], direction[0]), (origin[1], direction[1]). </font>
	</item>

	<item>
	<font color="#2a2a2a"> One direction, multiple origins: Fire a ray in the given direction from
	each origin point in origins. </font>
	</item>
	
	<item>
	<font color="#2a2a2a"> One origin, multiple directions: Fire a ray from the origin point in each
	direction in directions. </font>
	</item>
	
	</list>

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>
		\link HF::RayTracer::EmbreeRayTracer \endlink <br>

	Required definitions for the types outlined below are required:<br>
		\link RayResult \link <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link CreateRaytracer \endlink <br>
		\link FireRaysDistance \endlink <br>
		\link DestroyRayResultVector \endlink <br>
		\link DestroyRayTracer \endlink <br>
		\link DestroyMeshInfo \endlink <br>

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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define points for rays. Every three array members constitutes one point.
		// These are Cartesian coordinates.
		float points[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = 9;
		const int count_points = size_points / 3;

		// Define directions for casting rays. Every three array members constitutes 
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = 9;
		const int count_dir = 3;

		// Declare a pointer to vector<RayResult>.
		// FireRaysDistance will allocate memory for this pointer,
		// we must call DestroyRayResultVector on ray_result when we are done with it.
		// ray_result_data will refer to the address of (*ray_result)'s internal buffer.
		// As such, we do NOT call operator delete on ray_result_data.
		std::vector<RayResult>* ray_result = nullptr;
		RayResult* ray_result_data = nullptr;

		status = FireRaysDistance(bvh, points, count_points, dir, count_dir, &ray_result, &ray_result_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireRaysDistance, code: " << status << std::endl;
		}

		//
		// Iterate over *(ray_result) and output its contents
		//
		auto it = ray_result->begin();

		std::cout << "Ray result: [";
		while (it < ray_result->end()) {
			auto rr = *(it++);
			std::cout <<
				"{Distance: " << rr.distance << "\n" << "Mesh ID: " << rr.meshid
				<< "}";

			if (it < ray_result->end()) {
				std::cout << ", ";
			}
		}
		std::cout << "]" << std::endl;

		//
		// Memory resource cleanup.
		//

		// destroy vector<RayResult>
		status = DestroyRayResultVector(ray_result);

		if (status != 1) {
			std::cerr << "Error at DestroyRayResultVector, code: " << status << std::endl;
		}

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define point to start ray.
		// These are Cartesian coordinates.
		float p1[] = { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray.
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f };

		// Fire a ray for the hitpoint (Fire a ray, get a hit point back)
		float max_distance = -1;
		bool did_hit = false;

		// Fire a ray for the distance/meshid (Fire a ray, get a distance/mesh ID back)
		float distance = 0.0f;
		int mesh_id = -1;
		status = FireSingleRayDistance(bvh, p1, dir, max_distance, &distance, &mesh_id);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireSingleRayDistance, code: " << status << std::endl;
		}

		std::cout << "Distance is " << distance << ", " << "meshid is " << mesh_id << std::endl;

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define point to start ray.
		// These are Cartesian coordinates.
		float p1[] = { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray.
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f };

		// Fire a ray for the hitpoint (Fire a ray, get a hit point back)
		float max_distance = -1;
		bool did_hit = false;

		// We copy the contents of p1 into hit_point.
		// hit_point will be initialized to the origin point values,
		// and if a hit occurs, hit_point will be set to the hit coordinate values.
		//
		// We will know if a hit occurs if did_hit is set 'true' by FireRay.
		float hit_point[] = { p1[0], p1[1], p1[2] };
		status = FireRay(bvh, hit_point[0], hit_point[1], hit_point[2], dir[0], dir[1], dir[2], max_distance, did_hit);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireRay, code: " << status << std::endl;
		}

		if (did_hit) {
			std::cout << "Hit point: " << "[" << hit_point[0] << ", " << hit_point[1] << ", " << hit_point[2] << "]" << std::endl;
		}
		else {
			std::cout << "Hit point: " << "(miss)" << std::endl;
		}

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define points for rays. Every three array members constitutes one point.
		// These are Cartesian coordinates.
		float points[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = 9;
		const int count_points = size_points / 3;

		// Define directions for casting rays. Every three array members constitutes one direction.
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = 9;
		const int count_dir = 3;

		// Maximum distance a ray can travel and still hit its target.
		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from points[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.
		bool results[count_dir];

		status = FireMultipleRays(bvh, points, dir, count_points, max_distance, results);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleRays, code: " << status << std::endl;
		}

		//
		// Review results:
		//
		for (int i = 0, k = 0; i < count_dir; i++, k += 3) {
			std::string label = results[i] ? "hit" : "miss";

			std::cout << "result[" << i << "]: " << label << std::endl;
			std::cout << "[" << points[k] << ", " << points[k + 1] << ", " << points[k + 2]
				<< "], direction [" << dir[k] << ", " << dir[k + 1] << ", " << dir[k + 2] << "]"
				<< std::endl;
		}

		//
		// Memory resource cleanup.
		//

		if (status != 1) {
			std::cerr << "Error at DestroyRayResultVector, code: " << status << std::endl;
		}

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define points to start rays. Every three array members constitutes one point.
		// These are Cartesian coordinates.
		float p1[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 4.0f };
		const int size_p1 = 9;
		const int count_points = size_p1 / 3;

		// Define one direction to cast rays. Every three array members constitutes one direction.
		// These are vector components, not Cartesian coordinates.
		const float dir[] = { 0.0f, 0.0f, -1.0f };
		const int size_dir = 3;
		const int count_dir = size_dir / 3;

		// Maximum distance a ray can travel and still hit a target
		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from p1[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.

		// count_points rays will be fired, from the coordinates described at the array p1.
		// results[i] is true if a ray fired from origin point {p1[i], p1[i + 1], p1[i + 2]} via direction dir
		// makes a hit.
		bool results[count_points];

		// results will be mutated by FireMultipleOriginsOneDirection.
		status = FireMultipleOriginsOneDirection(bvh, p1, dir, count_points, max_distance, results);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleDirectionsOneOrigin, code: " << status << std::endl;
		}

		//
		//	Review results:
		//
		for (int i = 0, k = 0; i < count_points; i++, k += 3) {
			std::string label = results[i] ? "hit" : "miss";

			std::cout << "result[" << i << "]: " << label << std::endl;
			std::cout << "[" << dir[0] << ", " << dir[1] << ", " << dir[2]
				<< "], from point [" << p1[k] << ", " << p1[k + 1] << ", " << p1[k + 2] << "]"
				<< std::endl;
		}

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define point to start ray. Every three array members constitutes one point.
		// These are Cartesian coordinates.
		const float p1[] = { 0.0f, 0.0f, 2.0f };

		// Define directions to cast rays. Every three array members consitutes one direction.
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = 9;
		const int count_dir = size_dir / 3;

		// Maximum distance a ray can travel and still hit its target.
		const int max_distance = -1;

		// If a given ray i hits a target 
		// (dir[i] is the x component of a vector extending from points[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.
		bool results[count_dir];

		// dir, and results will be mutated by FireMultipleDirectionsOneOrigin.
		// if results[i] is true, representing a hit,
		// dir[i], dir[i + 1], dir[i + 2] represents a hit point.
		status = FireMultipleDirectionsOneOrigin(bvh, p1, dir, count_dir, max_distance, results);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleDirectionsOneOrigin, code: " << status << std::endl;
		}

		//
		// Review results:
		//
		for (int i = 0, k = 0; i < count_dir; i++, k += 3) {
			std::string label = results[i] ? "hit" : "miss";

			std::cout << "result[" << i << "]: " << label << std::endl;
			std::cout << "[" << dir[0] << ", " << dir[1] << ", " << dir[2]
				<< "], direction [" << dir[k] << ", " << dir[k + 1] << ", " << dir[k + 2] << "]"
				<< std::endl;
		}

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define point to start ray.
		// These are Cartesian coordinates.
		const int size_p1_occl = 3;
		const float p1_occl[size_p1_occl] = { 0.0f, 0.0f, 2.0f };

		// count_origin represents how many sets of origin coordinates we are dealing with.
		const int count_origin = size_p1_occl / 3;

		// All of the direction components, inline, one after another. These are vector components.
		// Every three array members constitutes one direction.
		const int size_dir_occl = 9;
		const float dir_occl[size_dir_occl] = { 0.0f, 0.0f, -1.0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

		// count_dir_occl represents how many sets of directions we are dealing with.
		const int count_dir_occl = size_dir_occl / 3;

		// The array results should be the amount of rays we are firing, i.e. the value of count_origin.
		bool results[size_p1_occl] = { false };
		float max_distance_occl = 9999.0f;
		status = FireOcclusionRays(bvh, p1_occl, dir_occl, count_origin, count_dir_occl, max_distance_occl, results);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireOcclusionRays, code: " << status << std::endl;
		}

		bool does_occlude = results[0];
		std::cout << "Does the ray connect? " << (does_occlude ? std::string("True") : std::string("False")) << std::endl;

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
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

		// Create BVH

		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		// Define points for rays. Every three array members constitutes one point.
		// These are Cartesian coordinates.
		float points[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = 9;
		const int count_points = size_points / 3;

		// Define directions for casting rays. Every three array members constitutes one direction.
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = 9;
		const int count_dir = 3;

		// Declare a pointer to vector<RayResult>.
		// FireRaysDistance will allocate memory for this pointer,
		// we must call DestroyRayResultVector on ray_result when we are done with it.
		// ray_result_data will refer to the address of (*ray_result)'s internal buffer.
		// As such, we do NOT call operator delete on ray_result_data.
		std::vector<RayResult>* ray_result = nullptr;
		RayResult* ray_result_data = nullptr;

		status = FireRaysDistance(bvh, points, count_points, dir, count_dir, &ray_result, &ray_result_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireRaysDistance, code: " << status << std::endl;
		}

		//
		// Memory resource cleanup.
		//

		// destroy vector<RayResult>
		status = DestroyRayResultVector(ray_result);

		if (status != 1) {
			std::cerr << "Error at DestroyRayResultVector, code: " << status << std::endl;
		}

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode
*/
C_INTERFACE DestroyRayResultVector(
	std::vector<RayResult>* analysis
);

/**@}*/