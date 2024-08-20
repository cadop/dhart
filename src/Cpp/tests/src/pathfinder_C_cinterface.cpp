/*!
	\file		pathfinder_C_cinterface.cpp
	\brief		Unit test source file for testing pathfinder functionality (C Interface)

	\author		TBA
	\date		12 Aug 2020
*/

#include "gtest/gtest.h"

#include <memory>

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/exception/exception.hpp>

#include "path_finder.h"
#include "objloader.h"
#include "boost_graph.h"
#include "graph.h"
#include "node.h"
#include "edge.h"
#include "path.h"
#include "HFExceptions.h"

#include "analysis_C.h"
#include "objloader_C.h"
#include "raytracer_C.h"
#include "pathfinder_C.h"
#include "cost_algorithms.h"
#include "spatialstructures_C.h"

namespace CInterfaceTests {
	TEST(_pathfinder_cinterface, CreatePath) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "energy_blob_zup.obj";

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());

		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

		// Load mesh.
		// Notice that we pass the address of the loaded_obj pointer
		// to LoadOBJ. We do not want to pass loaded_obj by value, but by address --
		// so that we can dereference it and assign it to the address of (pointer to)
		// the free store memory allocated within LoadOBJ.
		const std::array<float, 3> rot { 0.0f, 0.0f, 0.0f };	// No rotation.
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
			// Error!
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
		}

		//
		// Set the graph parameters
		//

		// Define start point.
		// These are Cartesian coordinates.
		// If not above solid ground, no nodes will be generated.
		const std::array<float, 3> start_point{ -30.0f, 0.0f, 20.0f };

		// Define spacing.
		// This is the spacing between nodes, with respect to each axis.
		// Lower values create more nodes, yielding a higher resolution graph.
		const std::array<float, 3> spacing{ 2.0f, 2.0f, 180.0f };

		// Value of - 1 will generate infinitely many nodes.
		// Final node count may be greater than this value.
		const int max_nodes = 5000;

		const int up_step = 30;					// Maximum step height the graph can traverse. 
												// Steps steeper than this are considered to be inaccessible.

		const int down_step = 70;				// Maximum step down the graph can traverse.
												// Steps steeper than this are considered to be inaccessible.

		const int up_slope = 60;				// Maximum upward slope the graph can traverse (in degrees).
												// Slopes steeper than this are considered to be inaccessible.

		const int down_slope = 60;				// Maximum downward slope the graph can traverse (in degrees).
												// Slopes steeper than this are considered to be inaccessible.

		const int max_step_connections = 1;		// Multiplier for number of children to generate for each node.
												// Increasing this value increases the number of edges in the graph,
												// therefore increasing the memory footprint of the algorithm overall.

		const int core_count = -1;				// CPU core count. A value of (-1) will use all available cores.

		// Generate graph.
		// Notice that we pass the address of the graph pointer into GenerateGraph.
		//
		// GenerateGraph will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
		HF::SpatialStructures::Graph* graph = nullptr;

		status = GenerateGraph(bvh, start_point.data(), spacing.data(), max_nodes,
			up_step, down_step, up_slope, down_slope,
			max_step_connections, core_count, &graph);

		if (status != 1) {
			std::cerr << "Error at GenerateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "Generate graph ran successfully - graph stored at address " << graph << ", code: " << status << std::endl;
		}

		// Always compress the graph after generating a graph/adding new edges
		status = Compress(graph);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}

		// graph should not be null after calling GenerateGraph.
		ASSERT_TRUE(graph != nullptr);

		// Get nodes.
		//
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

		// node_vector should not be null after calling GetAllNodesFromGraph.
		ASSERT_TRUE(node_vector != nullptr);

		// node_vector should not be empty after calling GetAllNodesFromGraph.
		ASSERT_FALSE(node_vector->empty());

		// Get size of node vector
		int node_vector_size = -1;
		status = GetSizeOfNodeVector(node_vector, &node_vector_size);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetSizeOfNodeVector code: " << status << std::endl;
		}
	
		// Print number of nodes in the graph
		const int max_node = node_vector_size - 1;	// This is the max index of *node_vector
		std::cout << "Graph Generated with " << node_vector_size << " nodes" << std::endl;

		//! [snippet_CreatePath]
		//
		// Call Dijkstra's Shortest Path Algorithm
		//

		// Retrieve the first ID and the last ID indexed in *node_vector.
		const int start_id = 0;
		const int end_id = node_vector->size() - 1;

		// empty string means to use the cost type that the graph was constructed with
		const char* cost_type = "";

		// Will be set to the size of the found path. Output value of 0 means no path was constructed.
		int path_size = -1;

		// If a path is found, path will address memory for a Path.
		// DestroyPath must be called on path when finished with use.
		HF::SpatialStructures::Path* path = nullptr;

		// Will address the underlying buffer of a member container within *path.
		// When DestroyPath is called on path, path_data will no longer be valid.
		// Likewise, if path remains null after attempting to create a path,
		// path_data will also be null.
		HF::SpatialStructures::PathMember* path_data = nullptr;

		// Create a path.
		status = CreatePath(graph, start_id, end_id, cost_type, &path_size, &path, &path_data);

		if (status != 1) {
			std::cerr << "Error at CreatePath, code: " << status << std::endl;
		}
		else {
			if (path) {
				std::cout << "CreatePath stored path successfully - path stored at address " << path << ", code: " << status << std::endl;

				// Get total path cost & output the result.
				auto path_sum = 0.0f;
				for (auto& member : path->members) { path_sum += member.cost; }
				std::cout << "Total path cost: " << path_sum << std::endl;
			}
		}
		//! [snippet_CreatePath]

		// path should not be null after calling CreatePath.
		ASSERT_TRUE(path != nullptr);
		ASSERT_NE(path_size, 0);

		//
		// Memory resource cleanup.
		//

		// destroy Path
		status = DestroyPath(path);

		if (status != 1) {
			std::cerr << "Error at DestroyPath, code: " << status << std::endl;
		}

		// destroy vector<Node>
		status = DestroyNodes(node_vector);

		if (status != 1) {
			std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
		}

		// destroy graph
		status = DestroyGraph(graph);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
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

		std::cout << "\nEnd status: " << status << std::endl;
		if (status == 1) {
			std::cout << "[OK]" << std::endl;
		}
		else {
			std::cout << "[Error occurred]" << std::endl;
		}
		std::cout << "\n--- End Example ---\n" << std::endl;
	}

	TEST(_pathfinder_cinterface, CreatePaths) {
		//! [snippet_CreatePaths]
		// TODO
		//! [snippet_CreatePaths]
	}

	TEST(_pathfinder_cinterface, GetPathInfo) {
		//! [snippet_GetPathInfo]
		// TODO
		//! [snippet_GetPathInfo]
	}

	TEST(_pathfinder_cinterface, DestroyPath) {
		//! [snippet_DestroyPath]
		// TODO
		//! [snippet_DestroyPath]
	}

	TEST(_pathfinder_cinterface, CreateAllToAllPaths) {
		//! [snippet_CreateAllToAllPaths]
		// TODO
		//! [snippet_CreateAllToAllPaths]
	}

	TEST(_pathfinder_cinterface, CalculateDistanceAndPredecessor) {
		using std::vector;
		using HF::SpatialStructures::Graph;

		//! [EX_DistPred_C]
		// Create a graph
		Graph* g;
		CreateGraph(NULL, -1, &g);

		// Create some nodes and add edges to the graph
		vector<vector<float>> nodes = {
			{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 1, 2}
		};
		AddEdgeFromNodes(g, nodes[0].data(), nodes[1].data(), 10, "");
		AddEdgeFromNodes(g, nodes[1].data(), nodes[2].data(), 20, "");
		AddEdgeFromNodes(g, nodes[0].data(), nodes[2].data(), 5, "");
		AddEdgeFromNodes(g, nodes[1].data(), nodes[0].data(), 10, "");
		Compress(g);

		// Create output parameters
		std::vector<float>* dist_vector; std::vector<int>* pred_vector;
		float* dist_data; int* pred_data;

		// Call into the new function
		auto status = CalculateDistanceAndPredecessor(g, "", &dist_vector, &dist_data, &pred_vector, &pred_data);

		//! [EX_DistPred_C]

		ASSERT_EQ(HF::Exceptions::HF_STATUS::OK, status);
		// Calculate the matricies using the C++ function, to ensure the results are identical
		// Turn it into a boost graph
		auto bg = HF::Pathfinding::CreateBoostGraph(*g);

		// Create distance/predecessor matricies from the boost graph
		// auto matricies = HF::Pathfinding::GenerateDistanceAndPred(*bg.get());
		auto cpp_pred = matricies.pred; auto cpp_dist = matricies.dist;

		// Compare to C-Interface  generated results
		for (int i = 0; i < g->size() * g->size(); i++) {

			// Comparisons between nans will always fail, so handle this before
			// doing the equality check. 
			const bool cpp_dist_is_nan = isnan(cpp_dist->at(i));
			const bool dist_is_nan = isnan(dist_vector->at(i));
			if (cpp_dist_is_nan && dist_is_nan) continue;

			ASSERT_EQ(cpp_pred->at(i), pred_vector->at(i));
			ASSERT_EQ(cpp_dist->at(i), dist_vector->at(i));
		}
		delete matricies.dist;
		delete matricies.pred;

		//! [EX_DistPred_C_2]

		// Print both matricies
		const int array_length = dist_vector->size();
		std::cout << "Distance Matrix: [";
		for (int i = 0; i < array_length; i++)
			std::cout << dist_vector->at(i) << (i == array_length - 1 ? "]\r\nPredecessor Mattrix: [" : ", ");
		for (int i = 0; i < array_length; i++)
			std::cout << pred_vector->at(i) << (i == array_length - 1 ? "]\r\n" : ", ");

		// Cleanup memory
		DestroyIntVector(pred_vector);
		DestroyFloatVector(dist_vector);

		//! [EX_DistPred_C_2]
	}
}
