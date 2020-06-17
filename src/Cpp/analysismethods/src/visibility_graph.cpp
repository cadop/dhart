///
///	\file		visibility_graph.cpp
///	\brief		Source file for VisibilityGraph namespace (procedures)
///
///	\author		TBA
///	\date		17 Jun 2020
///
#include <visibility_graph.h>

#include <array>
#include <thread>
#include <omp.h>
#include<algorithm>

#include <robin_hood.h>
#include <graph.h>
#include <embree_raytracer.h>
#include <Edge.h>
#include <node.h>

using namespace HF::AnalysisMethods;
using namespace HF::SpatialStructures;
using HF::RayTracer::EmbreeRayTracer;
using std::vector;

namespace HF::AnalysisMethods::VisibilityGraph {
	inline bool HeightCheck(const Node& Node, float height, EmbreeRayTracer& ert) {
		const std::array<float, 3> up{ 0,0,1 };
		auto node_copy = std::array<float, 3>{Node[0], Node[1], Node[2] + 0.01f};
		return !ert.FireAnyOcclusionRay(node_copy, up, height);
	}

	std::vector<int> HeightCheckAllNodes(const std::vector<Node>& nodes_to_filter, float height, EmbreeRayTracer& ert) {
		std::vector<int> valid_nodes;

		for (int i = 0; i < nodes_to_filter.size(); i++) {
			auto& node = nodes_to_filter[i];
			if (HeightCheck(node, height, ert))
				valid_nodes.push_back(i);
		}
		return valid_nodes;
	}

	/// <summary>
	/// Checks to see if there's an occlusion between two nodes at height level
	/// </summary>
	/// <returns>True if there is an obstacle between nodes a and b, false otherwise</returns>
	inline bool IsOcclusionBetween(
		const Node& node_a,
		const Node& node_b,
		EmbreeRayTracer& ert,
		float height = 1.7f,
		float pre_calculated_distance = 0.0f
	) {
		auto heightened_node = node_a;
		heightened_node[2] += height;

		// Only calculate distance if it wasn't already passed to us
		float distance;
		if (pre_calculated_distance == 0.0f)
			distance = node_a.distanceTo(node_b);
		else
			distance = pre_calculated_distance;

		auto direction = node_a.directionTo(node_b);

		return ert.FireAnyOcclusionRay(heightened_node, direction, distance);
	}

	Graph AllToAll(EmbreeRayTracer& ert, const vector<Node>& nodes, float height) {
		const int n = nodes.size();
		std::vector<std::vector<int>> edges(n);
		std::vector<std::vector<float>> costs(n);

		int cores = -1;
		if (cores < 0) {
			cores = std::thread::hardware_concurrency();
			omp_set_num_threads(cores);
		}
		else if (cores > 0)
			omp_set_num_threads(cores);

		auto valid_nodes = HeightCheckAllNodes(nodes, height, ert);
		
#pragma omp parallel for schedule(static)
		for (int i = 0; i < valid_nodes.size(); i++) {
			int node_id = valid_nodes[i];
			const Node& node_a = nodes[node_id];
			vector<int>& edge_list = edges[node_id];
			vector<float>& cost_list = costs[node_id];

			for (int k = 0; k < valid_nodes.size(); k++) {
				if (i == k) continue;
				int node_b_id = valid_nodes[k];
				const Node& node_b = nodes[node_b_id];
				float distance = node_a.distanceTo(node_b);
				if (!IsOcclusionBetween(node_a, nodes[node_b_id], ert, height, distance)) {
					edge_list.push_back(node_b_id);
					cost_list.push_back(distance);
				}
			}
		}
		return Graph(edges, costs, nodes);
	}


	Graph GroupToGroup(EmbreeRayTracer& ert, const vector<Node>& from, const vector<Node>& to, float height) {
		const int from_count = from.size();
		const int to_count = to.size();

		std::vector<std::vector<int>> edges(from_count + to_count);
		std::vector<std::vector<float>> costs(from_count + to_count);

		int cores = -1;
		if (cores < 0) {
			cores = std::thread::hardware_concurrency();
			omp_set_num_threads(cores);
		}
		else if (cores > 0) omp_set_num_threads(cores);

		auto valid_nodes = HeightCheckAllNodes(from, height, ert);
		auto valid_to_nodes = HeightCheckAllNodes(to, height, ert);
#pragma omp parallel for schedule(static)
		for (int i = 0; i < valid_nodes.size(); i++) {
			int id = valid_nodes[i];
			const Node& node_a = from[id];
			// Skip this node if it would clip into a ceiling at this height
			vector<int>& edge_list = edges[id];
			vector<float>& cost_list = costs[id];

			for (int k = 0; k < valid_to_nodes.size(); k++) {
				int to_id = valid_to_nodes[k];
				const Node& node_b = to[to_id];
				float distance = node_a.distanceTo(node_b);
				if (!IsOcclusionBetween(node_a, node_b, ert, height, distance)) {
					edge_list.push_back(to_id + from_count);
					cost_list.push_back(distance);
				}
			}
		}

		// Copy all nodes into a single array for the graph.
		std::vector<Node> graph_nodes(from.size() + to.size());
		std::copy(from.begin(), from.end(), graph_nodes.begin());
		std::copy(to.begin(), to.end(), graph_nodes.begin() + from.size());

		return Graph(edges, costs, graph_nodes);
	}

	Graph AllToAllUndirected(EmbreeRayTracer& ert, const vector<Node>& nodes, float height, int cores) {
		const int n = nodes.size();
		std::vector<std::vector<int>> edges(n);
		std::vector<std::vector<float>> costs(n);

		if (cores < 0) {
			cores = std::thread::hardware_concurrency();
			omp_set_num_threads(cores);
		}
		else if (cores > 0)
			omp_set_num_threads(cores);

		const auto valid_nodes = HeightCheckAllNodes(nodes, height, ert);
#pragma omp parallel
		{
#pragma omp for schedule(dynamic)
			for (int i = 0; i < valid_nodes.size(); i++) {
				int node_a_id = valid_nodes[i];

				const Node& node_a = nodes[node_a_id];
				vector<int>& edge_list = edges[node_a_id];
				vector<float>& cost_list = costs[node_a_id];

				for (int k = i + 1; k < valid_nodes.size(); k++) {
					int node_b_id = valid_nodes[k];
					const Node& node_b = nodes[node_b_id];
					float distance = node_a.distanceTo(node_b);
					if (!IsOcclusionBetween(node_a, nodes[k], ert, height, distance)) {
						edge_list.emplace_back(node_b_id);
						cost_list.emplace_back(distance);
					}
				}
			}
		}
		return Graph(edges, costs, nodes);
	}
}