/*!
	\file		analysis_C_cinterface.cpp
	\brief		Unit test source file for functions tested from analysis_C.h

	\author		TBA
	\date		11 Aug 2020
*/

#include "gtest/gtest.h"

#include "analysis_C.h"
#include "objloader_C.h"
#include "raytracer_C.h"
#include "spatialstructures_C.h"
#include "node.h"

namespace CInterfaceTests {
	TEST(_analysis_cinterface, GenerateGraph) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
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
		const float rot[] = { 90.0f, 0.0f, 0.0f };	// Y up to Z up
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

		//! [snippet_analysis_cinterface_GenerateGraph_setup_0]
		// Define start point.
		// These are Cartesian coordinates.
		float start_point[] = { -1.0f, -6.0f, 1623.976928f };

		// Define spacing.
		// This is the spacing between nodes, with respect to each axis.
		float spacing[] = { 0.5f, 0.5f, 0.5f };

		// Set max nodes.
		const int max_nodes = 500;
		//! [snippet_analysis_cinterface_GenerateGraph_setup_0]

		//! [snippet_analysis_cinterface_GenerateGraph]
		// Generate graph.
		// Notice that we pass the address of the graph pointer into GenerateGraph.
		//
		// GenerateGraph will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.

		float up_step = 1;						// maximum height of a step the graph can traverse
		float up_slope = 1;						// maximum upward slope the graph can traverse in degrees
		float down_step = 1;					// maximum step down that the graph can traverse
		float down_slope = 1;					// maximum downward slope that the graph can traverse
		int maximum_step_connection = 1;		// multiplier for number of children to generate for each node
		int core_count = -1;					// CPU core count, -1 uses all available cores on the machine

		HF::SpatialStructures::Graph* graph = nullptr;

		status = GenerateGraph(bvh,
			start_point, spacing, max_nodes,
			up_step, up_slope,
			down_step, down_slope,
			maximum_step_connection,
			core_count,
			&graph);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GenerateGraph, code: " << status << std::endl;
		}
		//! [snippet_analysis_cinterface_GenerateGraph]

		//! [snippet_analysis_cinterface_GenerateGraph_compress]
		// Always compress the graph after generating a graph/adding new edges
		status = Compress(graph);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
		//! [snippet_analysis_cinterface_GenerateGraph_compress]
		
		//! [snippet_analysis_cinterface_GenerateGraph_GetNodes]
		// Get all nodes from graph first (the container of nodes)
		// The address of the local variable node_vector will be passed to GetAllNodesFromGraph;
		// it will be dereferenced inside that function and assigned memory via operator new.
		//
		// We will have to call DestroyNodes on node_vector to properly release this memory.
		// node_vector_data points to the internal buffer that resides within *(node_vector).
		//
		// When we call DestroyNodes, node_vector_data's memory will also be released.
		std::vector<HF::SpatialStructures::Node>* node_vector = nullptr;
		HF::SpatialStructures::Node* node_vector_data = nullptr;

		status = GetAllNodesFromGraph(graph, &node_vector, &node_vector_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetAllNodesFromGraph, code: " << status << std::endl;
		}

		// Get size of node vector
		int node_vector_size = -1;
		status = GetSizeOfNodeVector(node_vector, &node_vector_size);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetSizeOfNodeVector, code: " << status << std::endl;
		}

		// Print number of nodes in the graph
		std::cout << "Node count: " << node_vector_size << std::endl;
		//! [snippet_analysis_cinterface_GenerateGraph_GetNodes]

		//! [snippet_analysis_cinterface_GenerateGraph_output]
		// Output 3 of the nodes within *node_vector.
		std::cout << "[";
		for (int i = 0; i < 3; i++) {
			auto n = (*node_vector)[i];
			std::cout << "("
				<< n.x << ", " << n.y << ", " << n.z << ", "
				<< n.id << ")";

			if (i < 2) {
				std::cout << " ";
			}
		}
		std::cout << "]" << std::endl;
		//! [snippet_analysis_cinterface_GenerateGraph_output]

		//! [snippet_analysis_cinterface_GenerateGraph_destroy]
		///
		/// Memory resource cleanup.
		///

		// destroy vector<Node>
		status = DestroyNodes(node_vector);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
		}

		// destroy graph
		status = DestroyGraph(graph);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		//! [snippet_analysis_cinterface_GenerateGraph_destroy]

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
