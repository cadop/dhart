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

}

TEST(_boostGraph, Destructor) {

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
