#include "gtest/gtest.h"

#include <memory>

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/exception/exception.hpp>

#include <path_finder.h>
#include <boost_graph.h>
#include <graph.h>
#include <node.h>
#include <edge.h>
#include <path.h>
#include <HFExceptions.h>

#include "pathfinder_C.h"
#include "cost_algorithms.h"
#include "spatialstructures_C.h"

using namespace HF::SpatialStructures;
using namespace HF::Pathfinding;
using std::vector;

TEST(_Pathfinding, BoostGraphCreation) {

	Graph g;
	g.addEdge(0, 1, 100);
	g.addEdge(0, 2, 50);
	g.addEdge(1, 3, 10);
	g.addEdge(2, 3, 10);

	g.Compress();
    std::unique_ptr<BoostGraph> bg = std::make_unique<BoostGraph>(BoostGraph(g));
	ASSERT_EQ(num_vertices(bg.get()->g),4);
}

TEST(_Pathfinding, SinglePath) {

	Graph g;
	g.addEdge(0, 1, 100);
	g.addEdge(0, 2, 50);
	g.addEdge(1, 3, 10);
	g.addEdge(2, 3, 10);
	g.Compress();

	//The optimal path here is 0->2->3
	std::vector<PathMember> OptimalPath = {
		{50,0},
		{10,2},
		{0,3}
	};
	Path OP(OptimalPath);

	BoostGraph* bg = new BoostGraph(g);
	Path p = FindPath(bg, 0, 3);
	delete bg;

	ASSERT_EQ(OP, p);
	
}

TEST(_Pathfinding, MultiplePaths) {

	Graph g;
	g.addEdge(0, 1, 100);
	g.addEdge(0, 2, 50);
	g.addEdge(1, 3, 10);
	g.addEdge(2, 3, 10);
	g.Compress();

	//The optimal path here is 0->2->3
	std::vector<PathMember> OptimalPath = {
		{50,0},
		{10,2},
		{0,3}
	};

	Path OP(OptimalPath);

	BoostGraph* bg = new BoostGraph(g);

	std::vector<int> start_points(100, 0);
	std::vector<int> end_points(100, 3);

	auto paths = FindPaths(bg, start_points, end_points);
	delete bg;


	for (const auto& path : paths)
		ASSERT_EQ(OP, path);
}

// A quick copy of the method to construct a path from a distnace
// and predecessor matrix in the pathfinder with some modifications.
// Using this to test distance and predecessor matricies. 
inline Path QuickConstructPath(
	int start,
	int end,
	const std::vector<int> &  pred,
	const std::vector<float> & distances
) {

	// Create a new path and add the end point.
	Path p;
	p.AddNode(end, 0);

	int current_node = end;
	if (pred[current_node] == current_node) return Path{};

	float last_cost = distances[current_node];

	while (current_node != start) {
		int next_node = pred[current_node];
		float current_cost = distances[next_node];
		float de_facto_cost = last_cost - current_cost;
	
		p.AddNode(next_node, de_facto_cost);
		
		last_cost = current_cost;
		current_node = next_node;
	}

	// Flip the order of this since this algorithm generates it from end to start
	p.Reverse();
	return p;
}

/*! Ensures taht no runtime errors occur and the array if of the correct length.*/
TEST(_Pathfinding, DistanceAndPredecessorMatrices) {

	//! [EX_DistPred]

	// Create a graph with some edges
	Graph g;

	vector<Node> nodes = {
		Node(1,2,3), Node(4, 5, 6),
		Node(7, 8, 9), Node(10, 1, 2)
	};
	g.addEdge(nodes[0], nodes[1], 10); g.addEdge(nodes[1], nodes[2], 20);
	g.addEdge(nodes[0], nodes[2], 5); g.addEdge(nodes[1], nodes[0], 10);
	g.Compress();

	// Turn it into a boost graph
	auto bg = CreateBoostGraph(g);

	// Create distance/predecessor matricies from the boost graph
	auto matricies = GenerateDistanceAndPred(*bg.get());

	// print output
	std::cerr << "DIST PRED " << g.size() << std::endl;
	std::cerr << matricies << std::endl;
	
	// get matricies from the output
	vector<float>* distance_matrix = matricies.dist;
	vector<int>* predecessor_matrix = matricies.pred;

	//! [EX_DistPred]

	// Generate a path using both this predecessor/distance matrix
	// and compare it to a standard path.
	auto dist_pred_path = QuickConstructPath(0, 2, *predecessor_matrix, *distance_matrix);
	auto actual_path = FindPath(bg.get(), 0, 2);

	// Compare the path from distance and predecessor matricies, to the actual path
	ASSERT_EQ(actual_path, dist_pred_path);

	//! [EX_DistPred_2]

	// Free them since it's our responsibility.
	delete distance_matrix;
	delete predecessor_matrix;
	
	//! [EX_DistPred_2]
}


// Performs the same task as the C++ DistanceAndPredecessor Matricies
// using the C-Interface, then compares the results to the results
// from using the C++ functions. 
TEST(C_Pathfinder, DistanceAndPredecessorMatrices_C) {

	//! [EX_DistPred_C]
	// Create a graph
	Graph * g;
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
	auto status = CalculateDistanceAndPredecessor(g,"", &dist_vector, &dist_data, &pred_vector, &pred_data);
		
	//! [EX_DistPred_C]
	
	ASSERT_EQ(HF::Exceptions::HF_STATUS::OK, status);
	// Calculate the matricies using the C++ function, to ensure the results are identical
	// Turn it into a boost graph
	auto bg = CreateBoostGraph(*g);

	// Create distance/predecessor matricies from the boost graph
	auto matricies = GenerateDistanceAndPred(*bg.get());
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
		std::cout << dist_vector->at(i) << (i ==  array_length - 1 ? "]\r\nPredecessor Mattrix: [" : ", ");
	for (int i = 0; i < array_length; i++)
		std::cout << pred_vector->at(i) << (i == array_length - 1 ? "]\r\n" : ", ");

	// Cleanup memory
	DestroyIntVector(pred_vector);
	DestroyFloatVector(dist_vector);

	//! [EX_DistPred_C_2]
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Pathfinding
///

TEST(_boostGraph, Constructor) {
	// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

	// In order to create a BoostGraph, we must first create a Graph instance first.
	// We must prepare the nodes, their edges, and the weights (distances) of each edge.

	// Create the nodes
	HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
	HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
	HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

	// Create a container (vector) of nodes
	std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

	// Create matrices for edges and distances, edges.size() == distances().size()
	std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
	std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

	// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
	HF::SpatialStructures::Graph graph(edges, distances, nodes);

	// Passing Graph graph to BoostGraph bg, by reference
	HF::Pathfinding::BoostGraph bg(graph);
}

TEST(_boostGraph, ConstructorCostName) {
	// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

	// for brevity
	using HF::SpatialStructures::Node;
	using HF::SpatialStructures::Graph;
	using HF::Pathfinding::BoostGraph;
	using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;


	// Create the nodes
	Node node_0(1.0f, 1.0f, 2.0f);
	Node node_1(2.0f, 3.0f, 4.0f, 5);
	Node node_2(11.0f, 22.0f, 140.0f);

	// Create a graph. No nodes/edges for now.
	Graph graph;

	// Add edges. These will have the default edge values, forming the default graph.
	graph.addEdge(node_0, node_1, 1);
	graph.addEdge(node_0, node_2, 2.5);
	graph.addEdge(node_1, node_2, 54.0);
	graph.addEdge(node_2, node_1, 39.0);

	// Always compress the graph after adding edges!
	graph.Compress();

	// Retrieve a Subgraph, parent node ID 0 -- of alternate edge costs.
	// Add these alternate edges to graph.
	std::string desired_cost_type = "cross slope";
	auto edge_set = CalculateEnergyExpenditure(graph.GetSubgraph(0));
	graph.AddEdges(edge_set, desired_cost_type);

	// Creating a BoostGraph.
	HF::Pathfinding::BoostGraph bg(graph, desired_cost_type);
}

TEST(_boostGraph, Destructor) {
	// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

	// In order to create a BoostGraph, we must first create a Graph instance first.
	// We must prepare the nodes, their edges, and the weights (distances) of each edge.

	// Create the nodes
	HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
	HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
	HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

	// Create a container (vector) of nodes
	std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

	// Create matrices for edges and distances, edges.size() == distances().size()
	std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
	std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

	// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
	HF::SpatialStructures::Graph graph(edges, distances, nodes);

	// Begin scope
	{
		// Create a BoostGraph bg, from a Graph graph (passing g by reference)
		HF::Pathfinding::BoostGraph bg(graph);
	}
	// End scope

	// When bg goes out of scope, BoostGraph::~BoostGraph is called
	// An explicit call to BoostGraph::~BoostGraph is also made when
	// invoking operator delete on a (BoostGraph *)
}

TEST(_boostGraphDeleter, OperatorFunction) {
	// be sure to #include "path_finder.h", #include "boost_graph.h", 
	// #include "node.h", #include "graph.h", and #include <vector>

	// For this example, we must have a BoostGraph instance to use with BoostGraphDeleter.
	// In order to create a BoostGraph, we must first create a Graph instance first.
	// We must prepare the nodes, their edges, and the weights (distances) of each edge.

	// Create the nodes
	HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
	HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
	HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

	// Create a container (vector) of nodes
	std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

	// Create matrices for edges and distances, edges.size() == distances().size()
	std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
	std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

	// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
	HF::SpatialStructures::Graph graph(edges, distances, nodes);

	// Passing Graph graph to BoostGraph bg, by reference
	HF::Pathfinding::BoostGraph bg(graph);

	//
	// Read below on BoostGraphDeleter -- this is important.
	//

	// BoostGraphDeleter is needed by std::unique_ptr to destroy a BoostGraph; it is not meant to be called directly.
	// You do not want to pass the address of a stack-allocated BoostGraph to BoostGraphDeleter::operator().

	// BoostGraphDeleter's operator() calls operator delete on the (BoostGraph *) argument,
	// and passing the address of a stack-allocated BoostGraph for said argument
	// will result in undefined behavior.

	//
	// In other words, do __not__ do the following:
	//
	/* 
		This code compiles and builds, but this is an example of what NOT to do --
		so the remainder of this test is commented out.
	
	HF::Pathfinding::BoostGraphDeleter bg_deleter;

	// Create a stack-allocated BoostGraph from a HF::SpatialStructures::Graph graph
	HF::Pathfinding::BoostGraph boostGraph(graph);	// not created using operator new

	// Using a BoostGraphDeleter on the (address of a) stack-allocated BoostGraph
	bg_deleter(&boostGraph);						// calls operator delete for a (BoostGraph *)

	// The type BoostGraphDeleter is only for use with std::unique_ptr.
	// See BoostGraph::CreateBoostGraph for BoostGraphDeleter's intended use.
	*/
}

TEST(_pathFinding, CreateBoostGraph) {
	// be sure to #include "path_finder.h", #include "boost_graph.h",
	// #include "node.h", and #include "graph.h"

	// For this example, we must have a BoostGraph instance to use with BoostGraphDeleter.
	// In order to create a BoostGraph, we must first create a Graph instance first.
	// We must prepare the nodes, their edges, and the weights (distances) of each edge.

	// Create the nodes
	HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
	HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
	HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

	// Create a container (vector) of nodes
	std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

	// Create matrices for edges and distances, edges.size() == distances().size()
	std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
	std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

	// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
	HF::SpatialStructures::Graph graph(edges, distances, nodes);

	// Now we can create a smart pointer to a BoostGraph
	// Note the type of boostGraph - it is a
	//		std::unique_ptr<HF::Pathfinding::BoostGraph, HF::Pathfinding::BoostGraphDeleter>.
	// Use the auto keyword for type inference, or your choice of using statements/typedef to make
	// the use of the type described above easier.
	auto boostGraph = HF::Pathfinding::CreateBoostGraph(graph);
}

TEST(_pathFinding, CreateBoostGraphCostName) {
	// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

	// for brevity
	using HF::SpatialStructures::Node;
	using HF::SpatialStructures::Graph;
	using HF::Pathfinding::BoostGraph;
	using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;


	// Create the nodes
	Node node_0(1.0f, 1.0f, 2.0f);
	Node node_1(2.0f, 3.0f, 4.0f, 5);
	Node node_2(11.0f, 22.0f, 140.0f);

	// Create a graph. No nodes/edges for now.
	Graph graph;

	// Add edges. These will have the default edge values, forming the default graph.
	graph.addEdge(node_0, node_1, 1);
	graph.addEdge(node_0, node_2, 2.5);
	graph.addEdge(node_1, node_2, 54.0);
	graph.addEdge(node_2, node_1, 39.0);

	// Always compress the graph after adding edges!
	graph.Compress();

	// Retrieve a Subgraph, parent node ID 0 -- of alternate edge costs.
	// Add these alternate edges to graph.
	std::string desired_cost_type = "cross slope";
	auto edge_set = CalculateEnergyExpenditure(graph.GetSubgraph(0));
	graph.AddEdges(edge_set, desired_cost_type);

	// Creating a BoostGraph smart pointer (std::unique_ptr<BoostGraph, BoostGraphDeleter>)
	auto boostGraph = HF::Pathfinding::CreateBoostGraph(graph, desired_cost_type);
}


TEST(_pathFinding, FindPath) {
	// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"
	
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

	// Get the path from node (id 0) to node (id 3)
	HF::SpatialStructures::Path path = HF::Pathfinding::FindPath(boostGraph.get(), 0, 3);

	// Print the nodes along the shortest path
	std::cout << "Shortest path from node id 0 to node id 3:" << std::endl;
	for (auto p : path.members) {
		std::cout << p.node << std::endl;
	}
}

TEST(_pathFinding, FindPaths) {
	// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"

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

	// Prepare the parents and children vectors --
	// We will be searching for the shortest path from node 0 to node 3,
	// as well as the shortest path from node 0 to node 4.
	std::vector<int> parents = { 0, 0 };
	std::vector<int> children = { 3, 4 };

	std::vector<HF::SpatialStructures::Path> paths 
		= HF::Pathfinding::FindPaths(boostGraph.get(), parents, children);

	// Get the shortest paths, which are already stored in paths
	auto path_0_3 = paths[0];
	auto path_0_4 = paths[1];

	// Print the shortest path from node 0 to node 3
	std::cout << "Shortest path from node id 0 to node id 3:" << std::endl;
	for (auto p : path_0_3.members) {
		std::cout << p.node << std::endl;
	}

	// Print the shortest path from node 0 to node 4
	std::cout << "Shortest path from node id 0 to node id 4:" << std::endl;
	for (auto p : path_0_4.members) {
		std::cout << p.node << std::endl;
	}
}
TEST(_pathFinding, InsertAllToAllPathsIntoArray) {
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
	auto bg = CreateBoostGraph(g);

	// Total paths is node_count ^ 2
	size_t node_count = g.Nodes().size();
	size_t path_count = node_count * node_count;

	// Pointer to buffer of (Path *)
	Path** out_paths = new Path* [path_count];
	// out_paths[i...path_count - 1] will be alloc'ed by InsertPathsIntoArray

	// Pointer to buffer of (PathMember *)
	PathMember** out_path_member = new PathMember* [path_count];
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

	InsertAllToAllPathsIntoArray(bg.get(),  out_paths, out_path_member, sizes);

	for (int i = 0; i < path_count; i++) {
		if (out_paths[i]) {
			// Always check if out_paths[i] is nonnull!
			int total_cost = 0;
			std::cout << "Path from " << start_points[i] << " to " << end_points[i] << std::endl;

			Path p = *out_paths[i];
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

/* This appears to always corrupt the heap when deallocating paths
   due to differing compiler options between google test and humanfactors.
TEST(_pathFinding, InsertPathsIntoArray) {
	// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"

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

	// Prepare the parents and children vectors --
	// We will be searching for the shortest path from node 0 to node 3,
	// as well as the shortest path from node 0 to node 4.
	std::vector<int> parents = { 0, 0 };
	std::vector<int> children = { 3, 4 };

	// Create smart pointers to hold Path, PathMember and sizes
	const int MAX_SIZE = 2;

	HF::SpatialStructures::Path** path_ptrs = new Path*;
	HF::SpatialStructures::PathMember** path_members_ptrs = new PathMember*;
	int* out_sizes = new int[2];

	//std::unique_ptr<HF::SpatialStructures::Path[]> result_paths(new Path[MAX_SIZE]);
	//std::unique_ptr<HF::SpatialStructures::PathMember[]> result_path_members(new PathMember[MAX_SIZE]);
	//std::unique_ptr<int[]> result_sizes(new int[MAX_SIZE]);

	// Retrieve raw pointers so their addresses can be passed to InsertPathsIntoArray
	//HF::SpatialStructures::Path* ppath = result_paths.get();
	//HF::SpatialStructures::PathMember* pmembers = result_path_members.get();
	//int* psizes = result_sizes.get();

	// Use InsertPathsIntoArray
	HF::Pathfinding::InsertPathsIntoArray(boostGraph.get(), parents, children, path_ptrs, path_members_ptrs, out_sizes);

	// Delete paths
	if (path_ptrs[0])
		delete (path_ptrs)[0];
	if (path_ptrs[1])
		delete (path_ptrs)[1];
	delete[2] path_ptrs;

	delete[2] out_sizes;

}
*/

namespace CInterfaceTests {
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
		//! [snippet_pathfinder_C_CreatePaths]
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// for brevity
		using HF::SpatialStructures::Graph;
		using HF::SpatialStructures::Path;
		using HF::SpatialStructures::PathMember;

		Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Maximum amount of paths to search
		const int MAX_PATHS = 2;
		const int MAX_PATHMEMBERS = 2;

		// Create a Graph g, and compress it
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		// We want to find the shortest paths from 0 to 3, and 0 to 4.
		std::array<int, 2> start_nodes{ 0, 0 };
		std::array<int, 2> end_nodes{ 3, 4 };

		// Create arrays of pointers to Path. We have MAX_PATHS total paths to assess.
		// Create arrays of pointers to PathMember. The anticipated maximum amount of "hops" per path is MAX_PATHMEMBERS.
		std::array<Path*, MAX_PATHS> out_path;
		std::array<PathMember*, MAX_PATHMEMBERS> out_path_member;

		// Sizes of paths generated by CreatePaths. Sizes of 0 mean that a path was unable to be generated.
		// Should be the same as
		std::array<int, MAX_PATHS> out_sizes;

		// Use CreatePaths
		CreatePaths(&g, start_nodes.data(), end_nodes.data(), "\0", out_path.data(), out_path_member.data(), out_sizes.data(), MAX_PATHS);

		//
		// Resource cleanup.
		//
		for (auto& p : out_path) {
			DestroyPath(p);
			p = nullptr;
		}
		//! [snippet_pathfinder_C_CreatePaths]
	}

	TEST(C_Pathfinder, CreatePathCostType) {
		//! [snippet_pathfinder_C_CreatePathCostType]
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
		//! [snippet_pathfinder_C_CreatePathCostType]
	}

	TEST(C_Pathfinder, CreatePathsCostType) {
		//! [snippet_pathfinder_C_CreatePathsCostType]
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
		const int MAX_PATHS = 2;
		const int MAX_PATHMEMBERS = 2;

		// Create a Graph g, and compress it
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(graph);

		// We want to find the shortest paths from 0 to 3, and 0 to 4.
		std::array<int, 2> start_nodes{ 0, 0 };
		std::array<int, 2> end_nodes{ 3, 4 };

		// Create arrays of pointers to Path. We have MAX_PATHS total paths to assess.
		// Create arrays of pointers to PathMember. The anticipated maximum amount of "hops" per path is MAX_PATHMEMBERS.
		std::array<Path*, MAX_PATHS> out_path;
		std::array<PathMember*, MAX_PATHMEMBERS> out_path_member;

		// Sizes of paths generated by CreatePaths. Sizes of 0 mean that a path was unable to be generated.
		// Should be the same as
		std::array<int, MAX_PATHS> out_sizes;

		// Use CreatePathsCostType, be sure to use the .c_str() method if using a std::string for desired_cost_type
		CreatePaths(&graph, start_nodes.data(), end_nodes.data(), desired_cost_type.c_str(), out_path.data(), out_path_member.data(), out_sizes.data(), MAX_PATHS);

		//
		// Use out_path, out_path_member
		//

		//
		// Resource cleanup
		//
		for (auto& p : out_path) {
			DestroyPath(p);
			p = nullptr;
		}
		//! [snippet_pathfinder_C_CreatePathsCostType]
	}

	TEST(C_Pathfinder, GetPathInfo) {
		//! [snippet_pathfinder_C_GetPathInfo]
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
		//! [snippet_pathfinder_C_GetPathInfo]
	}

	TEST(C_Pathfinder, DestroyPath) {
		//! [snippet_pathfinder_C_DestroyPath]
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
		//! [snippet_pathfinder_C_DestroyPath]
	}

	TEST(C_Pathfinder, CreateAllToAllPaths) {
		//! [snippet_pathfinder_C_CreateAllToAllPaths]
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
		auto bg = CreateBoostGraph(g);

		// Total paths is node_count ^ 2
		const size_t node_count = g.Nodes().size();
		const size_t path_count = node_count * node_count;

		// Buffer of (Path *). Each Path * must be freed using DestroyPath.
		std::vector<Path*> out_paths(path_count);

		// Buffer of (PathMember *). These pointers address the vector<PathMember> buffers in all *p in out_paths. 
		std::vector<PathMember*> out_path_member(path_count);

		// Pointer to buffer of (int)
		std::vector<int> sizes(path_count);

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

		CreateAllToAllPaths(&g, "", out_paths.data(), out_path_member.data(), sizes.data(), path_count);

		auto start_points_it = start_points.begin();
		auto end_points_it = end_points.begin();

		for (auto p : out_paths) {
			if (p) {
				// Always check if out_paths[i] is nonnull!
				int total_cost = 0;
				std::cout << "Path from " << *(start_points_it++) << " to " << *(end_points_it++) << std::endl;

				for (auto m : p->members) {
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
		for (auto& p : out_paths) {
			DestroyPath(p);
			p = nullptr;
		}
		//! [snippet_pathfinder_C_CreateAllToAllPaths]
	
		/*! [snippet_pathfinder_C_CreateAllToAllPaths_output]
		Path from 0 to 1
		node ID : 0      cost 1
		node ID : 1      cost 0
		Total cost : 1
		--------------------------
		Path from 0 to 2
		node ID : 0      cost 2
		node ID : 2      cost 0
		Total cost : 2
		--------------------------
		Path from 0 to 3
		node ID : 0      cost 1
		node ID : 1      cost 3
		node ID : 3      cost 0
		Total cost : 4
		--------------------------
		Path from 0 to 4
		node ID : 0      cost 1
		node ID : 1      cost 4
		node ID : 4      cost 0
		Total cost : 5
		--------------------------
		Path from 0 to 5
		node ID : 0      cost 1
		node ID : 1      cost 3
		node ID : 3      cost 5
		node ID : 5      cost 0
		Total cost : 9
		--------------------------
		Path from 0 to 6
		node ID : 0      cost 1
		node ID : 1      cost 4
		node ID : 4      cost 3
		node ID : 6      cost 0
		Total cost : 8
		--------------------------
		Path from 1 to 3
		node ID : 1      cost 3
		node ID : 3      cost 0
		Total cost : 3
		--------------------------
		Path from 1 to 4
		node ID : 1      cost 4
		node ID : 4      cost 0
		Total cost : 4
		--------------------------
		Path from 1 to 5
		node ID : 1      cost 3
		node ID : 3      cost 5
		node ID : 5      cost 0
		Total cost : 8
		--------------------------
		Path from 1 to 6
		node ID : 1      cost 4
		node ID : 4      cost 3
		node ID : 6      cost 0
		Total cost : 7
		--------------------------
		Path from 2 to 4
		node ID : 2      cost 4
		node ID : 4      cost 0
		Total cost : 4
		--------------------------
		Path from 2 to 6
		node ID : 2      cost 4
		node ID : 4      cost 3
		node ID : 6      cost 0
		Total cost : 7
		--------------------------
		Path from 3 to 5
		node ID : 3      cost 5
		node ID : 5      cost 0
		Total cost : 5
		--------------------------
		Path from 3 to 6
		node ID : 3      cost 5
		node ID : 5      cost 1
		node ID : 6      cost 0
		Total cost : 6
		--------------------------
		Path from 4 to 6
		node ID : 4      cost 3
		node ID : 6      cost 0
		Total cost : 3
		--------------------------
		Path from 5 to 6
		node ID : 5      cost 1
		node ID : 6      cost 0
		Total cost : 1
		--------------------------
		[snippet_pathfinder_C_CreateAllToAllPaths_output] */
	}
}