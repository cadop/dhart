#include "gtest/gtest.h"
#include <array>
#include <gtest/gtest.h>
#include <embree_raytracer.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <Edge.h>
#include <node.h>
#include <visibility_graph.h>
#include <string>
#include <array>
#include <visibility_graph_C.h>

using namespace HF::VisibilityGraph;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::RayTracer::EmbreeRayTracer;
using MeshInfo =  HF::Geometry::MeshInfo<float>;
using std::vector;

const std::string plane_path = "plane.obj"; // A flat plane

// A flat plane with a wall perpendicular to the y axis 
const std::string walled_plane_path = "plane_walled.obj"; 

EmbreeRayTracer CreatePlaneTracer() {
	vector<MeshInfo> meshInfos = LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE, true);
	return EmbreeRayTracer(meshInfos);
}

TEST(_VisibilityGraph, NodesOnFlatPlaneAllConnect) {
	vector<MeshInfo> meshInfos = LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE, true);
	EmbreeRayTracer plane_tracer(meshInfos);
	std::vector<Node> nodes;
	nodes.reserve(100);
	
	for (int i = -5; i < 5; i++) {
		for (int k = -5; k < 5; k++)
			nodes.emplace_back(Node(i, k, 0));
	}
	
	auto graph = AllToAll(plane_tracer, nodes);
	
	for (const auto& node : nodes)
		ASSERT_EQ(graph[node].size(), nodes.size()-1);
}

TEST(_VisibilityGraph, ParallelNodesOnFlatPlaneAllConnect) {
	vector<MeshInfo> meshInfos = LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE, true);
	EmbreeRayTracer plane_tracer(meshInfos);
	std::vector<Node> nodes;
	nodes.reserve(100);

	for (int i = -5; i < 5; i++) {
		for (int k = -5; k < 5; k++)
			nodes.emplace_back(Node(i, k, 0));
	}

	auto graph = AllToAllUndirected(plane_tracer, nodes, 1.7f);

	for (const auto& node : nodes)
		ASSERT_EQ(graph[node].size() + graph.getID(node), nodes.size() - 1);
}



TEST(_VisibilityGraph, NodesOnFlatPlaneWithWallDontConnect) {
	vector<MeshInfo> meshInfos = LoadMeshObjects(walled_plane_path, HF::Geometry::ONLY_FILE, true);
	EmbreeRayTracer plane_tracer(meshInfos);
	std::vector<Node> nodes{ Node(0,-1,0), Node(0,1,0) };
	auto graph = AllToAll(plane_tracer, nodes);

	for (const auto& node : nodes) {
		try {
			ASSERT_EQ(graph[node].size(), 0);
		}
		catch (std::out_of_range e) {
			continue;
		}

	}
}


// This is testing whether or not the group to group algorithm
// Produces a valid graph
TEST(_VisibilityGraph, GroupConstructsValidGraph) {
	auto raytracer = CreatePlaneTracer();
	
	vector<Node> nodes = {
		{0,0,1},
		{1,1,1},
		{1,0,1},
		{0,0,-1},
	};

	vector<Node> Group1 = { nodes[0], nodes[1] };
	vector<Node> Group2 = { nodes[2], nodes[3] };

	auto graph = GroupToGroup(raytracer, Group1, Group2);

	// Ensure that the graph's size equals the number of nodes that were created for it.
	ASSERT_EQ(4, graph.size());
	auto sums = graph.AggregateGraph(HF::SpatialStructures::COST_AGGREGATE::COUNT);
	
	// Assert that these values meet the expectation
	ASSERT_EQ(1, sums[0]);
	ASSERT_EQ(1, sums[1]); // both of these should only have a connection to node 3
	ASSERT_EQ(0, sums[2]);
	ASSERT_EQ(0, sums[3]);

	for (const auto& node : nodes) {
		std::cerr << node << std::endl;
	}

}

// Tests if the above holds for the C_INterface
TEST(C_VisibilityGraph, GroupConstructsValidGraph) {
	auto raytracer = CreatePlaneTracer();

	// Defin enodes
	vector<Node> nodes = {
		{0,0,1},
		{1,1,1},
		{1,0,1},
		{0,0,-1},
	};

	// Put each group into a seperate vector of floats. These will be
	// Passed to the C_Interface
	vector<float> group_a = {
		nodes[0].x, nodes[0].y, nodes[0].z,
		nodes[1].x, nodes[1].y, nodes[1].z,
	};

	vector<float> group_b = {
		nodes[2].x, nodes[2].y, nodes[2].z,
		nodes[3].x, nodes[3].y, nodes[3].z,
	};

	// Define a graph to use as an output parameter
	Graph* G;

	// Call the C_Interface Visibility graph
	auto res = CreateVisibilityGraphGroupToGroup(
		&raytracer,
		group_a.data(),
		group_a.size()/3,
		group_b.data(),
		group_b.size()/3,
		&G,
		1.7
	);
	
	// Create a reference to G for easier use
	const auto & out_graph = *G;

	// Assert that it is of the correct size
	ASSERT_EQ(4, out_graph.size());

	// Assert that the connections are correct
	auto sums = out_graph.AggregateGraph(HF::SpatialStructures::COST_AGGREGATE::SUM);
	ASSERT_EQ(1, sums[0]);
	ASSERT_EQ(1, sums[1]); // both of these should only have a connection to node 3
	ASSERT_EQ(0, sums[2]);
	ASSERT_EQ(0, sums[3]);

	// Assert that all nodes are finite to prove it isn't reading outside of the array's bounds
	for (const auto& node : out_graph.Nodes()) {
		ASSERT_TRUE(
			std::isfinite(node.x) &&
			std::isfinite(node.y) &&
			std::isfinite(node.z)
		);
		std::cerr << node << std::endl;
	}
}

///
///	The following are tests for the code samples for HF::VisibilityGraph
///

TEST(_visibilityGraph, AllToAll) {
	// be sure to #include "objloader.h"

	// path to OBJ file for a flat plane
	std::string plane_path = "plane.obj";

	// Create a vector of MeshInfo from plane_path, using LoadMeshObjects.
	// Note that LoadMeshObjects has two more arguments after plane_path -
	//		an enum HF::Geometry::GROUP_METHOD (defaults to GROUP_METHOD::ONLY_FILE)
	//		a bool (defaults to true, used to convert OBJ coordinate system to Rhino coordinates)
	std::vector<MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

	// Create an EmbreeRayTracer.
	HF::RayTracer::EmbreeRayTracer tracer(meshInfo);

	std::vector<HF::SpatialStructures::Node> node_vec;		// container of nodes
	const int reserve_count = 100;							// pre-defined reserve size
	node_vec.reserve(reserve_count);						// reserve reserve_count blocks for node_vec

	// Construct reserve_count Node and insert each of them into node_vec
	for (float i = -5.0; i < 5.0; i++) {
		// runs 10 times
		for (float j = -5; j < 5.0; j++) {
			// runs 10 times
			node_vec.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
		}
	}

	// AllToAll constructs and returns a Graph 
	// consisting of Node (from node_vec) that do not occlude each other
	float desired_height = 2.0f;		// Height of graph
	HF::SpatialStructures::Graph graph = HF::VisibilityGraph::AllToAll(tracer, node_vec, desired_height);
}

TEST(_visibilityGraph, GroupToGroup) {
	// be sure to #include "objloader.h"

	// path to OBJ file for a flat plane
	std::string plane_path = "plane.obj";

	// Create a vector of MeshInfo from plane_path, using LoadMeshObjects.
	// Note that LoadMeshObjects has two more arguments after plane_path -
	//		an enum HF::Geometry::GROUP_METHOD (defaults to GROUP_METHOD::ONLY_FILE)
	//		a bool (defaults to true, used to convert OBJ coordinate system to Rhino coordinates)
	std::vector<MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

	// Create an EmbreeRayTracer.
	HF::RayTracer::EmbreeRayTracer tracer(meshInfo);

	std::vector<HF::SpatialStructures::Node> node_vec_0;		// First container of Node
	std::vector<HF::SpatialStructures::Node> node_vec_1;		// Second container of Node

	const int reserve_count = 100;		// Pre-defined reserve size
	node_vec_0.reserve(reserve_count);	// Reserve reserve_count blocks for both vectors
	node_vec_1.reserve(reserve_count);

	// 0) Construct reserve_count Node and insert each of them into node_vec_0
	for (float i = -5.0; i < 5.0; i++) {
		// runs 10 times
		for (float j = -5.0; j < 5.0; j++) {
			// runs 10 times
			node_vec_0.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
		}
	}

	// 1) Construct reserve_count Node and insert each of them into node_vec_1
	for (float i = 0.0; i < 10.0; i++) {
		for (float j = 0.0; j < 10.0; j++) {
			node_vec_1.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
		}
	}

	// GroupToGroup constructs and returns a Graph consisting of Node
	// (between node_vec_0 and node_vec_1) such that the nodes do not occlude each other
	float desired_height = 2.0f;		// Height of graph
	HF::SpatialStructures::Graph graph = HF::VisibilityGraph::GroupToGroup(tracer, node_vec_0, node_vec_1, desired_height);
}

TEST(_visibilityGraph, AllToAllUndirected) {
	// be sure to #include "objloader.h"

	// path to OBJ file for a flat plane
	std::string plane_path = "plane.obj";

	// Create a vector of MeshInfo from plane_path, using LoadMeshObjects.
	// Note that LoadMeshObjects has two more arguments after plane_path -
	//		an enum HF::Geometry::GROUP_METHOD (defaults to GROUP_METHOD::ONLY_FILE)
	//		a bool (defaults to true, used to convert OBJ coordinate system to Rhino coordinates)
	std::vector<MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

	// Create an EmbreeRayTracer.
	HF::RayTracer::EmbreeRayTracer tracer(meshInfo);

	std::vector<HF::SpatialStructures::Node> node_vec;		// container of nodes
	const int reserve_count = 100;							// pre-defined reserve size
	node_vec.reserve(reserve_count);						// reserve reserve_count blocks for node_vec

	// Construct reserve_count Node and insert each of them into node_vec
	for (float i = -5.0; i < 5.0; i++) {
		// runs 10 times
		for (float j = -5; j < 5.0; j++) {
			// runs 10 times
			node_vec.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
		}
	}

	// AllToAllUndirected constructs and returns a Graph 
	// consisting of Node (from node_vec) that do not occlude each other
	float desired_height = 2.0f;		// Height of graph
	int core_count = 4;					// For omp_set_num_threads(int num_threads), CPU core count
	HF::SpatialStructures::Graph graph = 
		HF::VisibilityGraph::AllToAllUndirected(tracer, node_vec, desired_height, core_count);
}
