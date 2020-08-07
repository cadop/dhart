/*!
	\file		embree_raytracer_cinterface.cpp
	\brief		Unit test source file for testing the functions declared in raytracer_C.h

	\author		Gem Aludino
	\date		04 Aug 2020
*/
#include <gtest/gtest.h>

#include "objloader_C.h"
#include "raytracer_C.h"

namespace CInterfaceTests {
	TEST(_raytracer_CInterface, CreateRaytracer) {
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

		if (status != -1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

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
	}

	TEST(_raytracer_CInterface, DestroyRaytracer) {
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

		//! [snippet_DestroyRayTracer]
		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}
		//! [snippet_DestroyRayTracer]

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_raytracer_CInterface, FireRaysDistance) {
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
		//! [snippet_FireRaysDistance]
		// Define points for rays
		// These are Cartesian coordinates.
		float points[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = 9;
		const int count_points = size_points / 3;

		// Define directions for casting rays
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
		//! [snippet_FireRaysDistance]
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
	}

	TEST(_raytracer_CInterface, FireSingleRayDistance) {
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
		//! [snippet_FireSingleRayDistance]
		// Define point to start ray
		// These are Cartesian coordinates.
		float p1[] = { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray
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
		//! [snippet_FireSingleRayDistance]
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
	}

	TEST(_raytracer_CInterface, FireRay) {
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
		//! [snippet_FireRay]
		// Define point to start ray
		// These are Cartesian coordinates.
		float p1[] = { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray
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
		//! [snippet_FireRay]
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
	}

	TEST(_raytracer_CInterface, FireMultipleRays) {
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
		//! [snippet_FireMultipleRays]
		// Define points for rays
		// These are Cartesian coordinates.
		float points[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = 9;
		const int count_points = size_points / 3;

		// Define directions for casting rays
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
		//! [snippet_FireMultipleRays]
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
	}

	TEST(_raytracer_CInterface, FireMultipleOriginsOneDirection) {
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
		//! [snippet_FireMultipleOriginsOneDirection]
		// Define points to start rays
		// These are Cartesian coordinates.
		float p1[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 4.0f };
		const int size_p1 = 9;
		const int count_points = size_p1 / 3;

		// Define one direction to cast rays
		// These are vector components, not Cartesian coordinates.
		const float dir[] = { 0.0f, 0.0f, -1.0f };
		const int size_dir = 3;
		const int count_dir = size_dir / 3;

		// Maximum distance a ray can travel and still hit a target
		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from p1[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.

		// count_points rays will be fired, from the coordinates described at the array p1.
		// results[i] is true if a ray fired from p1[i], p1[i + 1], p1[i + 2] via direction dir
		// makes a hit.
		bool results[count_points];

		// results will be mutated by FireMultipleOriginsOneDirection.
		status = FireMultipleOriginsOneDirection(bvh, p1, dir, count_points, max_distance, results);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleDirectionsOneOrigin, code: " << status << std::endl;
		}

		//
		// Review results:
		//
		for (int i = 0, k = 0; i < count_points; i++, k += 3) {
			std::string label = results[i] ? "hit" : "miss";

			std::cout << "result[" << i << "]: " << label << std::endl;
			std::cout << "[" << dir[0] << ", " << dir[1] << ", " << dir[2]
				<< "], from point [" << p1[k] << ", " << p1[k + 1] << ", " << p1[k + 2] << "]"
				<< std::endl;
		}
		//! [snippet_FireMultipleOriginsOneDirection]
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
	}

	TEST(_raytracer_CInterface, FireMultipleDirectionsOneOrigin) {
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
		//! [snippet_FireMultipleDirectionsOneOrigin]
		// Define point to start ray
		// These are Cartesian coordinates.
		const float p1[] = { 0.0f, 0.0f, 2.0f };

		// Define directions to cast rays
		// These are vector components, not Cartesian coordinates.
		float dir[] = { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = 9;
		const int count_dir = size_dir / 3;

		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from points[i]),
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
		//! [snippet_FireMultipleDirectionsOneOrigin]
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
	}

	TEST(_raytracer_CInterface, FireOcclusionRays) {
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
		//! [snippet_FireOcclusionRays]
		// Define point to start ray
		// These are Cartesian coordinates.
		const int size_p1_occl = 3;
		const float p1_occl[size_p1_occl] = { 0.0f, 0.0f, 2.0f };

		// count_origin represents how many sets of origin coordinates we are dealing with.
		const int count_origin = size_p1_occl / 3;

		// All of the direction coordinates, inline, one after another.
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
		//! [snippet_FireOcclusionRays]
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
	}

	TEST(_raytracer_CInterface, DestroyRayResultVector) {
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

		// Define points for rays
		// These are Cartesian coordinates.
		float points[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = 9;
		const int count_points = size_points / 3;

		// Define directions for casting rays
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
		//! [snippet_DestroyRayResultVector]
		//
		// Memory resource cleanup.
		//

		// destroy vector<RayResult>
		status = DestroyRayResultVector(ray_result);

		if (status != 1) {
			std::cerr << "Error at DestroyRayResultVector, code: " << status << std::endl;
		}
		//! [snippet_DestroyRayResultVector]
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
	}
}
