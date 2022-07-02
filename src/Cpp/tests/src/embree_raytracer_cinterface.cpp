/*!
	\file		embreeEX_raytracer_cinterface.cpp
	\brief		Unit test source file for testing the functions declared in raytracer_C.h

	\author		Gem Aludino
	\date		04 Aug 2020
*/
#include <gtest/gtest.h>

#include <array>

#include "objloader_C.h"
#include "raytracer_C.h"
/*
namespace CInterfaceTests {
	TEST(EX_raytracer_cinterface, CreateRaytracer) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot{ 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);
		
		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if 
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());
	
		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//
		// Use bvh
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

	TEST(EX_raytracer_cinterface, DestroyRaytracer) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot{ 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if 
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

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

	TEST(EX_raytracer_cinterface, FireRaysDistance) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot{ 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if 
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//! [snippet_FireRaysDistance_points_components]
		// Define points for rays
		// These are Cartesian coordinates.
		std::array<float, 9> points{ 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = points.size();
		const int count_points = size_points / 3;

		// Define directions for casting rays
		// These are vector components, not Cartesian coordinates.
		std::array<float, 9> dir{ 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = dir.size();
		const int count_dir = size_dir / 3;
		//! [snippet_FireRaysDistance_points_components]

		// size_points represents the member count of the array points.
		// The member count must be a multiple of 3.
		ASSERT_TRUE(size_points % 3 == 0);

		// The value of count_points is dependent upon size_points -- 
		// count_points represents how many actual points we have.
		// count_points should be a multiple of 3, or be 1 (meaning size_points == 3)
		ASSERT_TRUE(count_points % 3 == 0 || count_points == 1);

		// dir represents vector components in R3; the member count should be a multiple of 3.
		ASSERT_TRUE(size_dir % 3 == 0);

		// Valid input accepted by FireRaysDistance:
		//		count_points == count_dir (one direction per origin)
		//		count_points == 1 && count_directions > 1 (one origin, multiple directions)
		//		count_points > 1 && count_directions == 1 (multiple origins, one direction)
		// In this particular example, since we are firing one direction per origin, 
		// we must ensure the following:
		ASSERT_EQ(count_points, count_dir);

		//! [snippet_FireRaysDistance]
		// Declare a pointer to vector<RayResult>.
		// FireRaysDistance will allocate memory for this pointer,
		// we must call DestroyRayResultVector on ray_result when we are done with it.
		// ray_result_data will refer to the address of (*ray_result)'s internal buffer.
		// As such, we do NOT call operator delete on ray_result_data.
		std::vector<RayResult>* ray_result = nullptr;
		RayResult* ray_result_data = nullptr;

		status = FireRaysDistance(bvh, points.data(), count_points, dir.data(), count_dir, &ray_result, &ray_result_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireRaysDistance, code: " << status << std::endl;
		}
		//! [snippet_FireRaysDistance]

		// If invalid values for count_points and count_dir are given to FireRaysDistance,
		// ray_result will not be assigned a valid address -- which means ray_result_data,
		// which addresses (*ray_result)'s internal buffer, will also not be assigned a valid address.
		ASSERT_TRUE(ray_result != nullptr);
		ASSERT_TRUE(ray_result_data != nullptr);

		// *ray_result should not be an empty container.
		ASSERT_FALSE(ray_result->empty());

		// ray_result->size() should be equal to the amount of rays that will be fired.
		ASSERT_EQ(ray_result->size(), count_dir);

		//! [snippet_FireRaysDistance_results]
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
		//! [snippet_FireRaysDistance_results]

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

	TEST(EX_raytracer_cinterface, FireSingleRayDistance) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot{ 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if 
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//! [snippet_FireSingleRayDistance_points_dir]
		// Define point to start ray
		// These are Cartesian coordinates.
		std::array<float, 3> p1 = { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray
		// These are vector components, not Cartesian coordinates.
		std::array<float, 3> dir = { 0.0f, 0.0f, -1.0f };
		//! [snippet_FireSingleRayDistance_points_dir]

		// p1 should represent one point in R3; it should have three members.
		ASSERT_TRUE(p1.size() == 3);

		// dir should represent vector components in R3; it should have three members
		// since dir represents one vector.
		ASSERT_TRUE(dir.size() == 3);

		//! [snippet_FireSingleRayDistance]
		float max_distance = -1;
		float distance = 0.0f;
		int mesh_id = -1;

		// Cast a ray for the distance/meshid (Cast a ray, get a distance/mesh ID back)
		status = FireSingleRayDistance(bvh, p1.data(), dir.data(), max_distance, &distance, &mesh_id);

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireSingleRayDistance, code: " << status << std::endl;
		}
		//! [snippet_FireSingleRayDistance]

		// If mesh_id returns -1, this indicates a miss.
		// This ray (based on parameters) is meant to hit.
		ASSERT_NE(mesh_id, -1);

		//! [snippet_FireSingleRayDistance_results]
		std::cout << "Distance is " << distance << ", " << "meshid is " << mesh_id << std::endl;
		//! [snippet_FireSingleRayDistance_results]

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

	TEST(EX_raytracer_cinterface, FireRay) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		//! [snippet_FireRay_points_direction]
		// Define point to start ray
		// These are Cartesian coordinates.
		std::array<float, 3> p1 { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray
		// These are vector components, not Cartesian coordinates.
		std::array<float, 3> dir { 0.0f, 0.0f, -1.0f };
		//! [snippet_FireRay_points_direction]

		// p1 represents a single point in R3. Member count should be 3.
		ASSERT_TRUE(p1.size() == 3);

		// dir represents a single vector (in component form) in R3. Member count should be 3.
		ASSERT_TRUE(dir.size() == 3);

		//! [snippet_FireRay_distance_hit_point]
		// Cast a ray for the hitpoint (Cast a ray, get a hit point back)
		float max_distance = -1;
		bool did_hit = false;

		// We copy the contents of p1 into hit_point.
		// hit_point will be initialized to the origin point values,
		// and if a hit occurs, hit_point will be set to the hit coordinate values.
		//
		// We will know if a hit occurs if did_hit is set 'true' by FireRay.
		std::array<float, 3> hit_point = { p1[0], p1[1], p1[2] };
		//! [snippet_FireRay_distance_hit_point]

		// hit_point represents a single point in R3. Member count should be 3.
		ASSERT_TRUE(hit_point.size() == 3);

		//! [snippet_FireRay]
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

		// Based on the parameters provided, the ray will hit its destination.
		ASSERT_TRUE(did_hit);

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

	TEST(EX_raytracer_cinterface, FireMultipleRays) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		//! [snippet_FireMultipleRays_points_directions]
		// Define points for rays
		// These are Cartesian coordinates.
		std::array<float, 9> points { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = points.size();
		const int count_points = size_points / 3;

		// Define directions for casting rays
		// These are vector components, not Cartesian coordinates.
		std::array<float, 9> dir { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = dir.size();
		const int count_dir = 3;
		//! [snippet_FireMultipleRays_points_directions]

		// size_points represents the member count of the array points.
		// The member count must be a multiple of 3.
		ASSERT_TRUE(size_points % 3 == 0);

		// The value of count_dir is dependent upon size_dir --
		// count_dir represents how many vectors we have.
		// count_dir should still be a multiple of 3, because three vector components consistute one vector --
		// and there should be one vector per point.
		ASSERT_TRUE(count_dir % 3 == 0);

		//! [snippet_FireMultipleRays]
		// Maximum distance a ray can travel and still hit its target.
		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from points[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.
		std::array<bool, count_dir> results;

		status = FireMultipleRays(bvh, points.data(), dir.data(), count_points, max_distance, results.data());

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleRays, code: " << status << std::endl;
		}
		//! [snippet_FireMultipleRays]

		// results was initialized to be an empty container, 
		// but it should not be empty after calling FireMultipleRays.
		ASSERT_FALSE(results.empty());

		// results.size() should be equal to the amount of rays to cast.
		ASSERT_EQ(results.size(), count_dir);

		//! [snippet_FireMultipleRays_results]
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
		//! [snippet_FireMultipleRays_results]

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

	TEST(EX_raytracer_cinterface, FireMultipleOriginsOneDirection) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//! [snippet_FireMultipleOriginsOneDirection_start_point]
		// Define points to start rays
		// These are Cartesian coordinates.
		std::array<float, 9> p1 { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 4.0f };
		const int size_p1 = p1.size();
		const int count_p1 = size_p1 / 3;
		//! [snippet_FireMultipleOriginsOneDirection_start_point]

		// size_p1 represents the member count of the array p1.
		// Member count should be a multiple of 3.
		ASSERT_TRUE(size_p1 % 3 == 0);

		//! [snippet_FireMultipleOriginsOneDirection_direction]
		// Define one direction to cast rays
		// These are vector components, not Cartesian coordinates.
		const std::array<float, 3> dir { 0.0f, 0.0f, -1.0f };
		const int size_dir = dir.size();
		const int count_dir = size_dir / 3;
		//! [snippet_FireMultipleOriginsOneDirection_direction]

		// size_dir represents the member count of the array dir.
		// There should be one vector for every point, and since we need at least one triangle,
		// and a triangle is composed of three points -- we should have at least 9 members in dir.
		// Just as with size_points, if we have more than 9 members in dir, the member count must be a multiple of 3.
		ASSERT_TRUE(size_p1 % 3 == 0);

		//! [snippet_FireMultipleOriginsOneDirection]
		// Maximum distance a ray can travel and still hit a target
		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from p1[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.

		// count_points rays will be fired, from the coordinates described at the array p1.
		// results[i] is true if a ray fired from p1[i], p1[i + 1], p1[i + 2] via direction dir
		// makes a hit.
		std::array<bool, count_p1> results;

		// results will be mutated by FireMultipleOriginsOneDirection.
		status = FireMultipleOriginsOneDirection(bvh, p1.data(), dir.data(), count_p1, max_distance, results.data());

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleDirectionsOneOrigin, code: " << status << std::endl;
		}

		//! [snippet_FireMultipleOriginsOneDirection]

		// results should not be empty after calling FireMultipleOriginsOneDirection.
		ASSERT_FALSE(results.empty());

		// results.size() should be equal to the amount of rays to cast.
		ASSERT_EQ(results.size(), count_p1);

		//! [snippet_FireMultipleOriginsOneDirection_results]
		//
		// Review results:
		//
		for (int i = 0, k = 0; i < count_p1; i++, k += 3) {
			std::string label = results[i] ? "hit" : "miss";

			std::cout << "result[" << i << "]: " << label << std::endl;
			std::cout << "[" << dir[0] << ", " << dir[1] << ", " << dir[2]
				<< "], from point [" << p1[k] << ", " << p1[k + 1] << ", " << p1[k + 2] << "]"
				<< std::endl;
		}
		//! [snippet_FireMultipleOriginsOneDirection_results]

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

	TEST(EX_raytracer_cinterface, FireMultipleDirectionsOneOrigin) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		//! [snippet_FireMultipleDirectionsOneOrigin_start_point_direction]
		// Define point to start ray
		// These are Cartesian coordinates.
		const std::array<float, 3> p1 { 0.0f, 0.0f, 2.0f };

		// Define directions to cast rays
		// These are vector components, not Cartesian coordinates.
		std::array<float, 9> dir { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = dir.size();
		const int count_dir = size_dir / 3;
		//! [snippet_FireMultipleDirectionsOneOrigin_start_point_direction]

		// p1 represents a point in R3, the member count should be 3.
		ASSERT_EQ(p1.size(), 3);

		// size_dir represents the member count of the array dir.
		// It should be a multiple of 3.
		ASSERT_TRUE(size_dir % 3 == 0);

		//! [snippet_FireMultipleDirectionsOneOrigin]
		const int max_distance = -1;

		// If a given ray i hits a target (dir[i] is a vector extending from points[i]),
		// results[i] will be set true. Otherwise, results[i] will be set false.
		std::array<bool, count_dir> results;

		// dir, and results will be mutated by FireMultipleDirectionsOneOrigin.
		// if results[i] is true, representing a hit,
		// dir[i], dir[i + 1], dir[i + 2] represents a hit point.
		status = FireMultipleDirectionsOneOrigin(bvh, p1.data(), dir.data(), count_dir, max_distance, results.data());

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireMultipleDirectionsOneOrigin, code: " << status << std::endl;
		}
		//! [snippet_FireMultipleDirectionsOneOrigin]

		// results should not be an empty container after calling FireMultipleDirectionsOneOrigin.
		ASSERT_FALSE(results.empty());

		// results.size() should be equal to the amount of rays to cast.
		ASSERT_EQ(results.size(), count_dir);

		//!	[snippet_FireMultipleDirectionsOneOrigin_results]
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
		//!	[snippet_FireMultipleDirectionsOneOrigin_results]

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

	TEST(EX_raytracer_cinterface, FireOcclusionRays) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		//! [snippet_FireOcclusionRays_start_point]
		// Define point to start ray
		// These are Cartesian coordinates.
		const std::array<float, 3> p1_occl { 0.0f, 0.0f, 2.0f };
		const int size_p1_occl = static_cast<int>(p1_occl.size());

		// count_origin represents how many sets of origin coordinates we are dealing with.
		const int count_origin = size_p1_occl / 3;
		//! [snippet_FireOcclusionRays_start_point]

		// p1_occl represents inline coordinates, in R3. size_p1_occl should be a multiple of 3.
		ASSERT_TRUE(size_p1_occl % 3 == 0);

		//! [snippet_FireOcclusionRays_components]
		// All of the direction components, inline, one after another.
		const std::array<float, 9> dir_occl { 0.0f, 0.0f, -1.0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };
		const int size_dir_occl = static_cast<int>(dir_occl.size());

		// count_dir_occl represents how many sets of directions we are dealing with.
		const int count_dir_occl = size_dir_occl / 3;
		//! [snippet_FireOcclusionRays_components]

		// dir_occl represents direction components in R3, size_dir_occl should be a multiple of 3 --
		// or, if dir_occl has three members, count_dir_occl should be 1 (only one ray fired)
		ASSERT_TRUE(size_dir_occl % 3 == 0 || (size_dir_occl == 3 && count_dir_occl == 1));

		//! [snippet_FireOcclusionRays]
		// The array results should be the amount of rays we are firing, i.e. the value of count_dir_occl.
		std::array<bool, count_dir_occl> results;
		float max_distance_occl = 9999.0f;

		status = FireOcclusionRays(bvh, p1_occl.data(), dir_occl.data(), count_origin, count_dir_occl, max_distance_occl, results.data());

		if (status != 1) {
			// Error!
			std::cerr << "Error at FireOcclusionRays, code: " << status << std::endl;
		}
		//! [snippet_FireOcclusionRays]

		// results should not be empty after calling FireOcclusionRays.
		ASSERT_FALSE(results.empty());

		// results.size() should be equal to the amount of occlusion rays to cast.
		ASSERT_EQ(results.size(), count_dir_occl);

		//! [snippet_FireOcclusionRays_result]
		bool does_occlude = results[0];
		std::cout << "Does the ray connect? " << (does_occlude ? std::string("True") : std::string("False")) << std::endl;
		//! [snippet_FireOcclusionRays_result]

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

	TEST(EX_raytracer_cinterface, DestroyRayResultVector) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// User input should not be an empty string.
		ASSERT_STRNE(obj_path_str.c_str(), "");

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up

		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);
		
		// If LoadOBJ assigns loaded_obj the address to a MeshInfo,
		// loaded_obj should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// loaded_obj will be nullptr if 
		//		obj_length <= 0
		//		LoadMeshObjects (called within LoadOBJ) throws an exception
		// because loaded_obj will never be assigned a valid address.
		ASSERT_TRUE(loaded_obj != nullptr);
		
		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		// Even if loaded_obj is non-null, *loaded_obj should not be an empty container.
		ASSERT_FALSE(loaded_obj->empty());

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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		// If CreateRaytracer assigns bvh the address to an EmbreeRayTracer,
		// bvh should not be nullptr, which is what it was initialized to
		// prior to calling LoadOBJ.
		//
		// bvh will be nullptr if
		//		loaded_obj == nullptr (we check for this above)
		//		loaded_obj->empty()
		ASSERT_TRUE(bvh != nullptr);

		// Define points for rays
		// These are Cartesian coordinates.
		std::array<float, 9> points { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f, 0.0, 0.0, 4.0f };
		const int size_points = points.size();
		const int count_points = size_points / 3;

		// size_points represents the member count of the array points.
		// Should be a multiple of 3.
		ASSERT_TRUE(size_points % 3 == 0);

		// Define directions for casting rays
		// These are vector components, not Cartesian coordinates.
		std::array<float, 9> dir { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -2.0f, 0.0f, 0.0f, -3.0f };
		const int size_dir = dir.size();
		const int count_dir = size_dir / 3;

		// size_dir represents the member count of the array dir.
		// Should be a multiple of 3.
		ASSERT_TRUE(size_dir % 3 == 0);

		// The value of count_dir is dependent upon size_dir --
		// count_dir represents how many vectors we have.
		// count_dir should still be a multiple of 3, because three vector components consistute one vector --
		// and there should be one vector per point.
		ASSERT_TRUE(count_dir % 3 == 0);

		// Declare a pointer to vector<RayResult>.
		// FireRaysDistance will allocate memory for this pointer,
		// we must call DestroyRayResultVector on ray_result when we are done with it.
		// ray_result_data will refer to the address of (*ray_result)'s internal buffer.
		// As such, we do NOT call operator delete on ray_result_data.
		std::vector<RayResult>* ray_result = nullptr;
		RayResult* ray_result_data = nullptr;

		// Valid input accepted by FireRaysDistance:
		//		count_points == count_dir (one direction per origin)
		//		count_points == 1 && count_directions > 1 (one origin, multiple directions)
		//		count_points > 1 && count_directions == 1 (multiple origins, one direction)

		// Since we are firing one direction per origin, we must ensure the following:
		ASSERT_EQ(count_points, count_dir);

		status = FireRaysDistance(bvh, points.data(), count_points, dir.data(), count_dir, &ray_result, &ray_result_data);

		// If invalid values for count_points and count_dir are given to FireRaysDistance,
		// ray_result will not be assigned a valid address -- which means ray_result_data,
		// which addresses (*ray_result)'s internal buffer, will also not be assigned a valid address.
		ASSERT_TRUE(ray_result != nullptr);
		ASSERT_TRUE(ray_result_data != nullptr);

		// ray_result->size() should be equal to the amount of rays to cast.
		ASSERT_EQ(ray_result->size(), count_dir);

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
*/
