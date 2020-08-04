/*!
	\file		raytracer_C.h
	\brief		C Interface header file for Raytracer functionality

	\author		TBA
	\date		04 Aug 2020

	\details
	All examples for each function assume the following code has been run.

	This code block shows how to load the HumanFactors DLL explicitly --
	you should only run this code once in your program.

	- Load the DLLs, in the order specified in the example below
	- Obtain pointers to the functions you want to call in the HumanFactors DLL
	- Call the desired functions via the function pointers
	- When finished with the HumanFactors DLL, free all DLLs loaded (in reverse order of loading)

	\code
		// Requires #include <Windows.h>

		// Load the following DLLs in this order:
		// tbb.dll, embree3.dll, HumanFactors.dll
		// If the DLLs are not loaded in this order,
		// HumanFactors.dll will fail to load!

		// Provide a relative path to the DLLs.
		const wchar_t path_tbb[27] = L"..\\x64-Release\\bin\\tbb.dll";
		const wchar_t path_embree3[31] = L"..\\x64-Release\\bin\\embree3.dll";
		const wchar_t path_humanfactors[36] = L"..\\x64-Release\\bin\\HumanFactors.dll";

		//
		//	Load tbb.dll first
		//
		HINSTANCE dll_tbb = LoadLibrary(path_tbb);

		if (dll_tbb == nullptr) {
			std::cerr << "Unable to load " << "tbb.dll" << std::endl;
			exit(EXIT_FAILURE);
		}
		else {
			std::cout << "Loaded successfully: " << "tbb.dll" << std::endl;
		}

		//
		//	embree3.dll depends on tbb.dll
		//
		HINSTANCE dll_embree3 = LoadLibrary(path_embree3);

		if (dll_embree3 == nullptr) {
			std::cerr << "Unable to load " << "embree3.dll" << std::endl;

			FreeLibrary(dll_tbb);
			exit(EXIT_FAILURE);
		}
		else {
			std::cout << "Loaded successfully: " << "embree3.dll" << std::endl;
		}

		//
		//	HumanFactors.dll depends on both tbb.dll and embree3.dll.
		//
		HINSTANCE dll_hf = LoadLibrary(path_humanfactors);

		if (dll_hf == nullptr) {
			std::cerr << "Unable to load " << "HumanFactors.dll" << std::endl;

			FreeLibrary(dll_embree3);
			FreeLibrary(dll_tbb);

			exit(EXIT_FAILURE);
		}
		else {
			std::cout << "Loaded successfully: " << "HumanFactors.dll" << std::endl;

			//
			//	At this point, you must obtain pointers to the functions you want to use/call
			//	within the HumanFactors DLL. Each example will outline the process
			//	for doing this.
			//
			//	After obtaining the pointers to the functions you want to call,
			//	you are now ready to call the desired functions and run code for your example.
			//

			std::this_thread::sleep_for(std::chrono::milliseconds(250));

			//
			//	Free libraries in order of creation.
			//	Only free the libraries once you are sure you are done using the functions within them,
			//	or else you will have to load them again.
			//
			if (FreeLibrary(dll_hf)) {
				std::cout << "Freed successfully: " << "HumanFactors.dll" << std::endl;
			}

			if (FreeLibrary(dll_embree3)) {
				std::cout << "Freed successfully: " << "embree3.dll" << std::endl;
			}

			if (FreeLibrary(dll_tbb)) {
				std::cout << "Freed successfully: " << "tbb.dll" << std::endl;
			}
		}
	\endcode
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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:
		
		// typedefs for brevity of syntax
		typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
		typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);
		typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer*);
		typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

		// Create pointers-to-functions addressed at the procedures defined in dll_hf, by using GetProcAddress()
		auto LoadOBJ = (p_LoadOBJ)GetProcAddress(dll_hf, "LoadOBJ");
		auto CreateRaytracer = (p_CreateRaytracer)GetProcAddress(dll_hf, "CreateRaytracer");
		auto DestroyRayTracer = (p_DestroyRayTracer)GetProcAddress(dll_hf, "DestroyRayTracer");
		auto DestroyMeshInfo = (p_DestroyMeshInfo)GetProcAddress(dll_hf, "DestroyMeshInfo");

		// You are now ready to call the functions above.
	\endcode

	\code
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

		///
		///	Use bvh
		///

		///
		/// Memory resource cleanup.
		///

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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		// typedefs for brevity of syntax
		typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
		typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);
		typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer*);
		typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

		// Create pointers-to-functions addressed at the procedures defined in dll_hf, by using GetProcAddress()
		auto LoadOBJ = (p_LoadOBJ)GetProcAddress(dll_hf, "LoadOBJ");
		auto CreateRaytracer = (p_CreateRaytracer)GetProcAddress(dll_hf, "CreateRaytracer");
		auto DestroyRayTracer = (p_DestroyRayTracer)GetProcAddress(dll_hf, "DestroyRayTracer");
		auto DestroyMeshInfo = (p_DestroyMeshInfo)GetProcAddress(dll_hf, "DestroyMeshInfo");

		// You are now ready to call the functions above.
	\endcode

	\code
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

		///
		///	Use bvh
		///

		///
		/// Memory resource cleanup.
		///

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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//
		
		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//

		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
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

	\code
		// TODO example
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

	\param	result_array	Output parameter conatining an ordered list of booleans set to true if the their rays hit, and
							false if their rays did not.

	\returns	HF_STATUS::OK on completion.

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//

		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
	\endcode
*/
C_INTERFACE FireMultipleRays(HF::RayTracer::EmbreeRayTracer* ert, float* origins, const float* directions, int size, float max_distance, bool* result_array);

/*!
	\brief		Fire rays from each origin point in the given direction.

	\ert			The raytracer to fire each ray from.
	\origins		A list of floats representing origin points, with each 3 floats representing one point.
	\direction		An array of 3 floats representing the X, Y, and Z coordinates respectively.
	\size			Number of points and directions, equal to the total number of floats in one array / 3.
	\max_distance	Maximum distance a ray can travel and still hit a target. 

	\result_array	Output parameter conatining an ordered list of booleans set to true if the their rays hit, 
					and false if their rays did not.

	\returns	HF_STATUS::OK on completion.

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//

		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//

		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//

		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
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

	\code
		// The usage of the variable dll_hf (type HINSTANCE)
		// is described at the top of this file.

		// The following code is required before running the example below:

		//
		// TODO function typedefs
		//

		//
		// TODO get pointers-to-functions via GetProcAddress
		//

		// You are now ready to call the functions above.
	\endcode

	\code
		//
		// TODO example
		//
	\endcode
*/
C_INTERFACE DestroyRayResultVector(
	std::vector<RayResult>* analysis
);

/**@}*/