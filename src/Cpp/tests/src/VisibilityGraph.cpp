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

using namespace HF::AnalysisMethods::VisibilityGraph;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;
using namespace HF::Geometry;

const std::string plane_path = "plane.obj"; // A flat plane

// A flat plane with a wall perpendicular to the y axis 
const std::string walled_plane_path = "plane_walled.obj"; 

TEST(_VisibilityGraph, NodesOnFlatPlaneAllConnect) {
	EmbreeRayTracer plane_tracer(LoadMeshObjects(plane_path));
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
	EmbreeRayTracer plane_tracer(LoadMeshObjects(plane_path));
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
	EmbreeRayTracer plane_tracer(LoadMeshObjects(walled_plane_path));
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

