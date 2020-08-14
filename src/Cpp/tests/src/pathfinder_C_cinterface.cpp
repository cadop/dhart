/*!
	\file		pathfinder_C_cinterface.cpp
	\brief		Unit test source file for testing pathfinder functionality (C Interface)

	\author		TBA
	\date		12 Aug 2020
*/

#include "gtest/gtest.h"

#include "objloader_C.h"
#include "raytracer_C.h"
#include "pathfinder_C.h"
#include "analysis_C.h"
#include "spatialstructures_C.h"	// Required for AlgorithmCostTitle

#include "path.h"					// Required for HF::SpatialStructures::Path, HF::SpatialStructures::PathMember
#include "node.h"					// Required for HF::SpatialStructures::Node
#include "graph.h"					// Required for HF::SpatialStructures::Graph
#include "path_finder.h"			// Required for HF::Pathfinding::CreateBoostGraph
#include "cost_algorithms.h"		// Required for CalculateEnergyExpenditure

#include <array>

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

	}

	TEST(_pathfinder_cinterface, GetPathInfo) {

	}

	TEST(_pathfinder_cinterface, DestroyPath) {

	}

	TEST(_pathfinder_cinterface, CreateAllToAllPaths) {

	}

	TEST(_pathfinder_cinterface, CalculateDistanceAndPredecessor) {

	}

	TEST(C_Pathfinder, CreatePath) {
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		// Prepare parameters for CreatePath
		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		CreatePath(&g, 0, 4, "\0", &out_size, &out_path, &out_path_member);

		// Use out_path, out_path_member

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	}

	TEST(C_Pathfinder, CreatePaths) {
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Maximum amount of paths to search
		const int MAX_SIZE = 2;

		// Create a Graph g, and compress it
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		// We want to find the shortest paths from 0 to 3, and 0 to 4.
		int start_nodes[] = { 0, 0 };
		int end_nodes[] = { 3, 4 };

		// Create dynamically-allocated arrays of pointers to Path
		// Create dynamically-allocated arrays of pointers to PathMember		
		HF::SpatialStructures::Path** out_path = new HF::SpatialStructures::Path * [MAX_SIZE];
		HF::SpatialStructures::PathMember** out_path_member = new HF::SpatialStructures::PathMember * [MAX_SIZE];

		// Sizes of paths generated by CreatePaths. Sizes of 0 mean that a path was unable to be generated.
		int* out_sizes = new int[MAX_SIZE];

		// Use CreatePaths
		CreatePaths(&g, start_nodes, end_nodes, "\0", out_path, out_path_member, out_sizes, MAX_SIZE);

		///
		/// Resource cleanup
		///

		for (int i = 0; i < MAX_SIZE; i++) {
			if (out_path[i]) {
				// Release memory for all pointers in out_path
				DestroyPath(out_path[i]);
				out_path[i] = nullptr;
			}
		}

		if (out_path) {
			// Release memory for pointer to out_path buffer
			delete[MAX_SIZE] out_path;
			out_path = nullptr;
		}

		if (out_path_member) {
			// Release memory for pointers to out_path_member buffer
			delete[MAX_SIZE] out_path_member;
			out_path_member = nullptr;
		}

		if (out_sizes) {
			// Release memory for pointer to out_sizes buffer
			delete[MAX_SIZE] out_sizes;
			out_sizes = nullptr;
		}
	}

	TEST(C_Pathfinder, CreatePathCostType) {
		// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

		// for brevity
		using HF::SpatialStructures::Node;
		using HF::SpatialStructures::Graph;
		using HF::Pathfinding::BoostGraph;
		using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;

		// Create the nodes
		Node node_0(1.0f, 1.0f, 2.0f);
		Node node_1(2.0f, 3.0f, 4.0f);
		Node node_2(11.0f, 22.0f, 140.0f);
		Node node_3(62.9f, 39.1f, 18.0f);
		Node node_4(19.5f, 27.1f, 29.9f);

		// Create a graph. No nodes/edges for now.
		Graph graph;

		// Add edges. These will have the default edge values, forming the default graph.
		graph.addEdge(node_0, node_1, 1);
		graph.addEdge(node_0, node_2, 2.5);
		graph.addEdge(node_1, node_3, 54.0);
		graph.addEdge(node_2, node_4, 39.0);
		graph.addEdge(node_3, node_4, 1.2);

		// Always compress the graph after adding edges!
		graph.Compress();

		// Retrieve a Subgraph, parent node ID 0 -- of alternate edge costs.
		// Add these alternate edges to graph.
		std::string desired_cost_type = AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE);
		auto edge_set = CalculateEnergyExpenditure(graph.GetSubgraph(0));
		graph.AddEdges(edge_set, desired_cost_type);

		// Prepare parameters for CreatePath
		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		// Use CreatePathCostType, be sure to use the .c_str() method if using a std::string for desired_cost_type
		CreatePath(&graph, 0, 4, desired_cost_type.c_str(), &out_size, &out_path, &out_path_member);

		///
		/// Use out_path, out_path_member
		///

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	}

	TEST(C_Pathfinder, CreatePathsCostType) {
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// for brevity
		using HF::SpatialStructures::Node;
		using HF::SpatialStructures::Graph;
		using HF::Pathfinding::BoostGraph;
		using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;

		// Create the nodes
		Node node_0(1.0f, 1.0f, 2.0f);
		Node node_1(2.0f, 3.0f, 4.0f);
		Node node_2(11.0f, 22.0f, 14.0f);
		Node node_3(62.9f, 39.1f, 18.0f);
		Node node_4(99.5f, 47.1f, 29.9f);

		// Create a graph. No nodes/edges for now.
		Graph graph;

		// Add edges. These will have the default edge values, forming the default graph.
		graph.addEdge(node_0, node_1, 1);
		graph.addEdge(node_0, node_2, 2.5);
		graph.addEdge(node_1, node_3, 54.0);
		graph.addEdge(node_2, node_4, 39.0);
		graph.addEdge(node_3, node_4, 1.2);

		// Always compress the graph after adding edges!
		graph.Compress();

		// Retrieve a Subgraph, parent node ID 0 -- of alternate edge costs.
		// Add these alternate edges to graph.
		std::string desired_cost_type = AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE);
		auto edge_set = CalculateEnergyExpenditure(graph);
		graph.AddEdges(edge_set, desired_cost_type);

		// Maximum amount of paths to search
		const int MAX_SIZE = 2;

		// We want to find the shortest paths from 0 to 3, and 0 to 4.
		int start_nodes[] = { 0, 0 };
		int end_nodes[] = { 3, 4 };

		// Create dynamically-allocated arrays of pointers to Path
		// Create dynamically-allocated arrays of pointers to PathMember		
		HF::SpatialStructures::Path** out_path = new HF::SpatialStructures::Path * [MAX_SIZE];
		HF::SpatialStructures::PathMember** out_path_member = new HF::SpatialStructures::PathMember * [MAX_SIZE];

		// Sizes of paths generated by CreatePaths. Sizes of 0 mean that a path was unable to be generated.
		int* out_sizes = new int[MAX_SIZE];

		// Use CreatePathsCostType, be sure to use the .c_str() method if using a std::string for desired_cost_type
		CreatePaths(&graph, start_nodes, end_nodes, desired_cost_type.c_str(), out_path, out_path_member, out_sizes, MAX_SIZE);

		///
		/// Use out_path, out_path_member
		///

		///
		/// Resource cleanup
		///

		for (int i = 0; i < MAX_SIZE; i++) {
			if (out_path[i]) {
				// Release memory for all pointers in out_path
				DestroyPath(out_path[i]);
				out_path[i] = nullptr;
			}
		}

		if (out_path) {
			// Release memory for pointer to out_path buffer
			delete[MAX_SIZE] out_path;
			out_path = nullptr;
		}

		if (out_path_member) {
			// Release memory for pointers to out_path_member buffer
			delete[MAX_SIZE] out_path_member;
			out_path_member = nullptr;
		}

		if (out_sizes) {
			// Release memory for pointer to out_sizes buffer
			delete[MAX_SIZE] out_sizes;
			out_sizes = nullptr;
		}
	}

	TEST(C_Pathfinder, GetPathInfo) {
		// Requires #include "pathfinder_C.h", #include "path.h"

		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		CreatePath(&g, 0, 4, "\0", &out_size, &out_path, &out_path_member);

		// Get out_path's info, store results in out_path_member and out_size
		GetPathInfo(out_path, &out_path_member, &out_size);

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	}

	TEST(C_Pathfinder, DestroyPath) {
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		CreatePath(&g, 0, 4, "\0", &out_size, &out_path, &out_path_member);

		// Use out_path, out_path_member

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	}

	TEST(C_Pathfinder, CreateAllToAllPaths) {
		HF::SpatialStructures::Graph g;

		// Add the edges
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(1, 4, 4);
		g.addEdge(2, 4, 4);
		g.addEdge(3, 5, 5);
		g.addEdge(4, 6, 3);
		g.addEdge(5, 6, 1);

		// Always compress the graph after adding edges
		g.Compress();

		// Create a BoostGraph (std::unique_ptr)
		auto bg = HF::Pathfinding::CreateBoostGraph(g);

		// Total paths is node_count ^ 2
		size_t node_count = g.Nodes().size();
		size_t path_count = node_count * node_count;

		// Pointer to buffer of (Path *)
		HF::SpatialStructures::Path** out_paths = new HF::SpatialStructures::Path * [path_count];
		// out_paths[i...path_count - 1] will be alloc'ed by InsertPathsIntoArray

		// Pointer to buffer of (PathMember *)
		HF::SpatialStructures::PathMember** out_path_member = new HF::SpatialStructures::PathMember * [path_count];
		// out_path_member[i...path_count - 1] points to out_paths[i...path_count - 1]->GetPMPointer();

		// Pointer to buffer of (int)
		int* sizes = new int[path_count];

		//
		// The two loops for start_points and end_points
		// are just for the output.
		//
		int curr_id = 0;
		std::vector<int> start_points(path_count);
		// Populate the start points,
		// size will be (node_count)^2
		for (int i = 0; i < node_count; i++) {
			for (int k = 0; k < node_count; k++) {
				start_points[curr_id++] = i;
			}
		}

		curr_id = 0;

		std::vector<int> end_points(path_count);
		// Populate the end points,
		// size will be (node_count)^2
		for (int i = 0; i < node_count; i++) {
			for (int k = 0; k < node_count; k++) {
				end_points[curr_id++] = k;
			}
		}

		CreateAllToAllPaths(&g, "", out_paths, out_path_member, sizes, path_count);

		for (int i = 0; i < path_count; i++) {
			if (out_paths[i]) {
				// Always check if out_paths[i] is nonnull!
				int total_cost = 0;
				std::cout << "Path from " << start_points[i] << " to " << end_points[i] << std::endl;

				HF::SpatialStructures::Path p = *out_paths[i];
				for (auto m : p.members) {
					total_cost += m.cost;
					std::cout << "node ID: " << m.node << "\tcost " << m.cost << std::endl;
				}

				std::cout << "Total cost: " << total_cost << std::endl;
				std::cout << "--------------------------" << std::endl;
			}
		}

		//
		// Resource cleanup
		//
		if (sizes) {
			delete[] sizes;
			sizes = nullptr;
		}

		if (out_path_member) {
			delete[] out_path_member;
			out_path_member = nullptr;
		}

		if (out_paths) {
			for (int i = 0; i < path_count; i++) {
				if (out_paths[i]) {
					delete out_paths[i];
					out_paths[i] = nullptr;
				}
			}
			delete[] out_paths;
		}
	}
}
