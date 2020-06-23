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

using namespace HF::SpatialStructures;
using namespace HF::Pathfinding;

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

}

TEST(_pathFinding, CreateBoostGraph) {

}

TEST(_pathFinding, FindPath) {

}

TEST(_pathFinding, FindPaths) {

}

TEST(_pathFinding, FindAllPaths) {

}

TEST(_pathFinding, InsertPathsIntoArray) {

}
