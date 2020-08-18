/*!
	\file		view_analysis_cinterface.cpp
	\brief		Unit test source file for C interface functions related to view analysis

	\author		TBA
	\date		11 Aug 2020
*/

#include "gtest/gtest.h"

#include "view_analysis_C.h"
#include "objloader_C.h"

#include "node.h"			// Required for definition of struct HF::SpatialStructures::Node
#include "HFExceptions.h"	// Required for definition of HF::Exceptions::HF_STATUS

using HF::Geometry::MeshInfo;
using HF::RayTracer::EmbreeRayTracer;
using HF::SpatialStructures::Node;

namespace CInterfaceTests {
	TEST(_view_analysis_cinterface, SphereicalViewAnalysisAggregate) {
		// Status code variable
		// Determines if a C Interface function ran OK, or returned an error code.
		int status = 1;

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}


		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_setup_0]
		// Preparing the parameters for SphereicalViewAnalysisAggregate

		// Define point to start ray
		// These are Cartesian coordinates.
		std::array<Node, 3> p1{ Node(0.0f, 0.0f, 2.0f) };

		// Define direction to cast ray
		// These are vector components, not Cartesian coordinates.
		std::array<float, 3> dir { 0.0f, 0.0f, -1.0f };

		// This is a container of nodes to be analyzed.
		// SphereicalViewAnalysisAggregate accepts a (Node *) parameter,
		// so a raw stack-allocated array will suffice.
		//
		// This may also be a pointer to a heap-allocated buffer
		// (which is owned by a vector<Node>, accessed via the vector<Node>::data() method).
		//
		// ...or, it can be a pointer to memory allocated by the caller via operator new[node_count]
		// (if this route is taken, be sure to release the memory with operator delete[] after use)

		const int node_count = static_cast<int>(p1.size());	// should be the element count in nodes
		int ray_count = 1000;				// will be mutated by SphereicalViewAnalysisAggregate
		const float height = 1.7f;			// height to offset nodes from the ground, in meters
		const float upward_fov = 50.0f;		// default parameter is 50.0f in Python code
		const float downward_fov = 70.0f;	// default parameter is 70.0f in Python code
		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_setup_0]

		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_setup_1]
		// Declare a pointer to vector<float>, named aggregate_results.
		// This pointer will point to memory on the free store,
		// allocated within SphereicalViewAnalysisAggregate.
		//
		// aggregate_results_data will point to the underlying buffer within *aggregate_results,
		// which will be assigned inside SphereicalViewAnalysisAggregate.
		//
		// Note that we must call operator delete on aggregate_results when we are finished with it.
		std::vector<float>* aggregate_results = nullptr;
		float* aggregate_results_data = nullptr;

		// Will be equal to aggregate_results->size();
		int aggregate_results_size = -1;

		// Select the aggregate type.
		// This determines how to aggregate the edges within the results of the view analysis.
		//
		// AGGREGATE_TYPE::AVERAGE means that the edges will be aggregated
		// by the maximum distance from the origin point to its hit points.
		AGGREGATE_TYPE agg_type = AGGREGATE_TYPE::AVERAGE;
		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_setup_1]

		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate]
		status = SphereicalViewAnalysisAggregate(bvh,
			p1.data(), node_count, ray_count,
			upward_fov, downward_fov, height,
			agg_type,
			&aggregate_results, &aggregate_results_data, &aggregate_results_size);

		if (status != 1) {
			// Error!
			std::cerr << "Error at SphereicalViewAnalysisAggregate, code: " << status << std::endl;
		}
		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate]

		// We must verify that aggregate_results is non-null after calling SphereicalViewAnalysisAggregate,
		// and even if it is non-null, we must verify that it is not empty.
		ASSERT_TRUE(aggregate_results != nullptr);
		ASSERT_FALSE(aggregate_results->empty());

		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_results]
		// Print aggregate results vector (vector<float>)
		std::cout << "[";
		int i = 0;
		for (auto agg_result : *aggregate_results) {
			std::cout << agg_result;

			if (i < aggregate_results->size() - 1) {
				std::cout << ", ";
			}

			++i;
		}
		std::cout << "]" << std::endl;
		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_results]

		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_destroy]
		//
		// Memory resource cleanup.
		//

		// destroy vector<float>
		if (aggregate_results) {
			delete aggregate_results;
		}
		//! [snippet_view_analysis_SphereicalViewAnalysisAggregate_destroy]

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

	TEST(_view_analysis_cinterface, SphereicalViewAnalysisAggregateFlat) {
		// Status code variable
		// Determines if a C Interface function ran OK, or returned an error code.
		int status = 1;

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_setup_0]
		// Preparing the parameters for SphericalViewAnalysisAggregateFlat

		// Define point to start ray
		// These are Cartesian coordinates.
		std::array<float, 3> p1 { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray
		// These are vector components, not Cartesian coordinates.
		std::array<float, 3> dir { 0.0f, 0.0f, -1.0f };

		// This is a container of nodes to be analyzed.
		// SphericalViewAnalysisAggregateFlat accepts a (Node *) parameter,
		// so a raw stack-allocated array will suffice.
		//
		// This may also be a pointer to a heap-allocated buffer
		// (which is owned by a vector<Node>, accessed via the vector<Node>::data() method).
		//
		// ...or, it can be a pointer to memory allocated by the caller via operator new[node_count]
		// (if this route is taken, be sure to release the memory with operator delete[] after use)

		const int node_count = 1;			// should be the element count in nodes
		int ray_count = 1000;				// will be mutated by SphericalViewAnalysisAggregateFlat
		const float height = 1.7f;			// height to offset nodes from the ground, in meters
		const float upward_fov = 50.0f;		// default parameter is 50.0f in Python code
		const float downward_fov = 70.0f;	// default parameter is 70.0f in Python code
		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_setup_0]

		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_setup_1]
		// Declare a pointer to vector<float>, named aggregate_results.
		// This pointer will point to memory on the free store,
		// allocated within SphericalViewAnalysisAggregateFlat.
		//
		// aggregate_results_data will point to the underlying buffer within *aggregate_results,
		// which will be assigned inside SphericalViewAnalysisNoAggregateFlat.
		//
		// Note that we must call operator delete on aggregate_results when we are finished with it.
		std::vector<float>* aggregate_results = nullptr;
		float* aggregate_results_data = nullptr;

		// Will be equal to aggregate_results->size();
		int aggregate_results_size = -1;

		// Select the aggregate type.
		// This determines how to aggregate the edges within the results of the view analysis.
		//
		// AGGREGATE_TYPE::AVERAGE means that the edges will be aggregated
		// by the maximum distance from the origin point to its hit points.
		AGGREGATE_TYPE agg_type = AGGREGATE_TYPE::AVERAGE;
		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_setup_1]

		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat]
		status = SphereicalViewAnalysisAggregateFlat(bvh,
			p1.data(), node_count, ray_count,
			upward_fov, downward_fov, height,
			agg_type,
			&aggregate_results, &aggregate_results_data, &aggregate_results_size);

		if (status != 1) {
			// Error!
			std::cerr << "Error at SphereicalViewAnalysisAggregateFlat, code: " << status << std::endl;
		}
		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat]

		// We must verify that aggregate_results is non-null after calling SphereicalViewAnalysisAggregate,
		// and even if it is non-null, we must verify that it is not empty.
		ASSERT_TRUE(aggregate_results != nullptr);
		ASSERT_FALSE(aggregate_results->empty());

		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_results]
		// Print aggregate results vector (vector<float>)
		std::cout << "[";
		int i = 0;
		for (auto agg_result : *aggregate_results) {
			std::cout << agg_result;

			if (i < aggregate_results->size() - 1) {
				std::cout << ", ";
			}

			++i;
		}
		std::cout << "]" << std::endl;
		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_results]

		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_destroy]
		//
		// Memory resource cleanup.
		//

		// destroy vector<float>
		if (aggregate_results) {
			delete aggregate_results;
		}
		//! [snippet_view_analysis_SphericalViewAnalysisAggregateFlat_destroy]

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

	TEST(_view_analysis_cinterface, SphericalViewAnalysisNoAggregate) {
		// Status code variable
		// Determines if a C Interface function ran OK, or returned an error code.
		int status = 1;

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_setup_0]
		// Preparing the parameters for SphericalViewAnalysisNoAggregate

		// Define point to start ray
		// These are Cartesian coordinates.
		std::array<Node, 1> p1{ Node(0.0f, 0.0f, 2.0f) };

		// Define direction to cast ray
		// These are vector components, not Cartesian coordinates.
		std::array<float, 3> dir { 0.0f, 0.0f, -1.0f };

		// This is a container of nodes to be analyzed.
		// SphericalViewAnalysisNoAggregate accepts a (Node *) parameter,
		// so a raw stack-allocated array will suffice.
		//
		// This may also be a pointer to a heap-allocated buffer
		// (which is owned by a vector<Node>, accessed via the vector<Node>::data() method).
		//
		// ...or, it can be a pointer to memory allocated by the caller via operator new[node_count]
		// (if this route is taken, be sure to release the memory with operator delete[] after use)

		const int node_count = static_cast<int>(p1.size());	// should be the element count in nodes
		int ray_count = 1000;				// will be mutated by SphericalViewAnalysisNoAggregate
		const float height = 1.7f;			// height to offset nodes from the ground, in meters
		const float upward_fov = 50.0f;		// default parameter is 50.0f in Python code
		const float downward_fov = 70.0f;	// default parameter is 70.0f in Python code
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_setup_0]

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_setup_1]
		// Declare a pointer to vector<RayResult>, named results.
		// This pointer will point to memory on the free store,
		// allocated within SphericalViewAnalysisNoAggregate.
		//
		// results_data will point to the underlying buffer within *results,
		// which will be assigned inside SphericalViewAnalysisNoAggregate.
		//
		// Note that we must call operator delete on results when we are finished with it.
		std::vector<RayResult>* results = nullptr;
		RayResult* results_data = nullptr;
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_setup_1]

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate]
		// Conducting a view analysis on the node at position p1.
		status = SphericalViewAnalysisNoAggregate(bvh,
			p1.data(), node_count, &ray_count,
			upward_fov, downward_fov, height,
			&results, &results_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at SphericalViewAnalysisNoAggregate, code: " << status << std::endl;
		}
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate]

		// We must verify that  results is non-null after calling SphereicalViewAnalysisNoAggregate,
		// and even if it is non-null, we must verify that it is not empty.
		ASSERT_TRUE(results != nullptr);
		ASSERT_FALSE(results->empty());

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_results]
		// Print results vector (vector<RayResult>)
		const int start_range = 15;
		const int end_range = 20;

		std::cout << "[";
		for (int i = start_range; i < end_range; i++) {
			auto result = (*results)[i];

			std::cout << "(" << result.distance << ", " << result.meshid << ")";

			if (i < end_range - 1) {
				std::cout << ", ";
			}
		}
		std::cout << "]" << std::endl;
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_results]

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_destroy]
		//
		// Memory resource cleanup.
		//

		// destroy vector<RayResult>
		if (results) {
			delete results;
		}
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregate_destroy]

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

	TEST(_view_analysis_cinterface, SphericalViewAnalysisNoAggregateFlat) {
		// Status code variable
		// Determines if a C Interface function ran OK, or returned an error code.
		int status = 1;

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
		const std::array<float, 3> rot { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
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
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_setup_0]
		// Preparing the parameters for SphericalViewAnalysisNoAggregateFlat

		// Define point to start ray
		// These are Cartesian coordinates.
		std::array<float, 3> p1 { 0.0f, 0.0f, 2.0f };

		// Define direction to cast ray
		// These are vector components, not Cartesian coordinates.
		std::array<float, 3> dir { 0.0f, 0.0f, -1.0f };

		// This is a container of nodes to be analyzed.
		// SphericalViewAnalysisNoAggregateFlat accepts a (Node *) parameter,
		// so a raw stack-allocated array will suffice.
		//
		// This may also be a pointer to a heap-allocated buffer
		// (which is owned by a vector<Node>, accessed via the vector<Node>::data() method).
		//
		// ...or, it can be a pointer to memory allocated by the caller via operator new[node_count]
		// (if this route is taken, be sure to release the memory with operator delete[] after use)

		const int node_count = 1;			// should be the element count in nodes
		int ray_count = 1000;				// will be mutated by SphericalViewAnalysisNoAggregateFlat
		const float height = 1.7f;			// height to offset nodes from the ground, in meters
		const float upward_fov = 50.0f;		// default parameter is 50.0f in Python code
		const float downward_fov = 70.0f;	// default parameter is 70.0f in Python code
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_setup_0]

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_setup_1]
		// Declare a pointer to vector<RayResult>, named results.
		// This pointer will point to memory on the free store,
		// allocated within SphericalViewAnalysisNoAggregateFlat.
		//
		// results_data will point to the underlying buffer within *results,
		// which will be assigned inside SphericalViewAnalysisNoAggregateFlat.
		//
		// Note that we must call operator delete on results when we are finished with it.
		std::vector<RayResult>* results = nullptr;
		RayResult* results_data = nullptr;
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_setup_1]

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat]
		// Conducting a view analysis on the node at position p1.
		status = SphericalViewAnalysisNoAggregateFlat(bvh,
			p1.data(), node_count, &ray_count,
			upward_fov, downward_fov, height,
			&results, &results_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at SphericalViewAnalysisNoAggregateFlat, code: " << status << std::endl;
		}
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat]

		// We must verify that aggregate_results is non-null after calling SphereicalViewAnalysisNoAggregateFlat,
		// and even if it is non-null, we must verify that it is not empty.
		ASSERT_TRUE(results != nullptr);
		ASSERT_FALSE(results->empty());

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_results]
		// Print results vector (vector<RayResult>)
		const int start_range = 15;
		const int end_range = 20;

		std::cout << "[";
		for (int i = start_range; i < end_range; i++) {
			auto result = (*results)[i];

			std::cout << "(" << result.distance << ", " << result.meshid << ")";

			if (i < end_range - 1) {
				std::cout << ", ";
			}
		}
		std::cout << "]" << std::endl;
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_results]

		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_destroy]
		//
		// Memory resource cleanup.
		//

		// destroy vector<RayResult>
		if (results) {
			delete results;
		}
		//! [snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_destroy]

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

	TEST(_view_analysis_cinterface, SphericalDistribute) {
		//! [snippet_view_analysis_SphericalDistribute_setup]
		//
		// Set arguments
		//
		int num_rays = 10;							// number of points to distribute
		std::vector<float>* out_float = nullptr;	// output parameter for points generated
		float* out_float_data;						// pointer to buffer within *out_float
		float up_fov = 90.0f;						// max degrees upward from viewer's eye level to consider
		float down_fov = 90.0f;						// max degrees downward from viewer's eye level to consider
		//! [snippet_view_analysis_SphericalDistribute_setup]

		//! [snippet_view_analysis_SphericalDistribute]
		//
		// Call function
		//
		auto status = SphericalDistribute(
			&num_rays,
			&out_float,
			&out_float_data,
			up_fov,
			down_fov
		);

		if (status != 1) {
			std::cerr << "Error at SphericalDistribute, code: " << status << std::endl;
		}
		//! [snippet_view_analysis_SphericalDistribute]

		// We must ensure that out_float points to a valid address after calling SphericalDistribute.
		ASSERT_TRUE(out_float != nullptr);

		// We must also ensure that *out_float is not empty.
		ASSERT_FALSE(out_float->empty());

		//! [snippet_view_analysis_SphericalDistribute_output]
		//
		// Print results
		//
		std::cout << "Number of rays: " << num_rays << std::endl;
		for (int i = 0; i < num_rays; i++) {
			int os = i * 3;

			std::cout << "("
				<< out_float_data[os] << ", "
				<< out_float_data[os + 1] << ", "
				<< out_float_data[os + 2] << ")"
				<< std::endl;
		}
		//! [snippet_view_analysis_SphericalDistribute_output]

		//! [snippet_view_analysis_SphericalDistribute_destroy]
		//
		// Deallocate Memory
		//
		DestroyFloatVector(out_float);
		//! [snippet_view_analysis_SphericalDistribute_destroy]
	}
}