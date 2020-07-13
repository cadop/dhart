#include "gtest/gtest.h"
#include <array>
#include <gtest/gtest.h>
#include <embree_raytracer.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <edge.h>
#include <node.h>
#include <visibility_graph.h>
#include <string>
#include <array>

using namespace HF::VisibilityGraph;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;
using namespace HF::Geometry;

const std::string plane_path = "plane.obj"; // A flat plane

// A flat plane with a wall perpendicular to the y axis 
const std::string walled_plane_path = "plane_walled.obj"; 

TEST(_VisibilityGraph, NodesOnFlatPlaneAllConnect) {
	EmbreeRayTracer plane_tracer(LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE, true));
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
	EmbreeRayTracer plane_tracer(LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE, true));
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
	EmbreeRayTracer plane_tracer(LoadMeshObjects(walled_plane_path, HF::Geometry::ONLY_FILE, true));
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
	std::vector<HF::Geometry::MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

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
	std::vector<HF::Geometry::MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

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
	std::vector<HF::Geometry::MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

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
