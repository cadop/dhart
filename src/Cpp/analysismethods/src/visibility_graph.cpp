///
///	\file		visibility_graph.cpp
///	\brief		Contains implementation for the <see cref = "HF::VisibilityGraph">VisibilityGraph</cref> class
///
///	\author		TBA
///	\date		17 Jun 2020

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
#include <Constants.h>

using namespace HF;
using namespace HF::SpatialStructures;
using HF::RayTracer::EmbreeRayTracer;
using std::vector;
using std::array;

namespace HF::VisibilityGraph {
	/*!
		\brief Check if Node can be raised by height without clipping into any geometry.
		
		\param Node Node to perform height check on.
		\param height Distance to check above the node in meters. Default = 1.7m.
		\param ert Raytracer containing geometry to intersect with.

		\returns true if Node can be raised by height without intersecting with any geometry.
		False otherwise.

		\details
		Cast an occlusion ray straight up with a distance of height to determine if 
		it will intersect with any geometry.

		\remarks
		This is useful to prevent nodes from going through ceilings or other geometry
		when comparing the scores of all nodes in a model.
	*/
	inline bool HeightCheck(const Node& Node, float height, EmbreeRayTracer& ert) {
		const array<float, 3> up{ 0,0,1 };
		// Create a copy of this node that's slightly offset off of the ground to ensure
		// it doesn't intersect with the ground
		auto node_copy = array<float, 3>{Node[0], Node[1], Node[2] + ROUNDING_PRECISION};
		
		// Cast an occlusion ray straight up with a distance of height.
		return !ert.FireAnyOcclusionRay(node_copy, up, height);
	}


	/*!
		\brief Obtain the indexes of all nodes that pass the HeightCheck.

		\param nodes_to_filter Nodes to perform the height check on
		\param height Height to check above the nodes
		\param ert Raytracer containing the geometry to use as obstacles

		\returns The indexes in nodes_to_filter of all nodes that pass the height check.

		\see HeightCheck
	*/
	vector<int> HeightCheckAllNodes(const vector<Node>& nodes_to_filter, float height, EmbreeRayTracer& ert) {
		vector<int> valid_nodes;

		// Loop through every node in nodes_to_filter
		for (int i = 0; i < nodes_to_filter.size(); i++) {

			// If the node passes the height check, insert its index
			// into valid_nodes.
			const auto& node = nodes_to_filter[i];
			if (HeightCheck(node, height, ert))
				valid_nodes.push_back(i);
		}
		return valid_nodes;
	}

	/*!
		\brief Perform a line of sight check between two nodes.

		\param node_a Node to check occlusion from.
		\param node_b Node to check occlusion to.
		\param ert Raytracer containing the geometry to use as obstacles.
		\param height Distance to offset a node in the z-direction before performing the check.
		\param pre_calculated_distance Distance from node_a to node_b. If set to zero then this
		will be calculated automatically.

		\returns True if there is an obstacle blocking line of sight between node_a and node_b
		when raised to height.

		\details 
		Calculate the distance between node_a and node_b if required, then calculate the direction between
		them. Cast an occlusion ray in the direction between node_a and node_b with a maximum distance
		equal to the distance between them. If the ray intersects anything then the nodes don't have a line 
		of sight. 
	*/
	inline bool IsOcclusionBetween(
		const Node& node_a,
		const Node& node_b,
		EmbreeRayTracer& ert,
		float height = 1.7f,
		float pre_calculated_distance = 0.0f
	) {
		// Raise node A (this only matters for the origin);
		auto heightened_node = node_a;
		heightened_node[2] += height;

		// Only calculate distance if it wasn't already passed to us
		float distance;
		if (pre_calculated_distance == 0.0f)
			distance = node_a.distanceTo(node_b);
		else
			distance = pre_calculated_distance;

		// Calculate the direction between node_a and node_b
		auto direction = node_a.directionTo(node_b);

		// Cast the ray and return the result.
		return ert.FireAnyOcclusionRay(heightened_node, direction, distance);
	}

	Graph AllToAll(EmbreeRayTracer& ert, const vector<Node>& nodes, float height) {

		// Create a jagged array for edges and costs
		const int n = nodes.size();
		vector<vector<int>> edges(n);
		vector<vector<float>> costs(n);

		// Set the degree of parallelism to the number of cores of the client machine
		int cores = -1;
		if (cores < 0) {
			cores = std::thread::hardware_concurrency();
			omp_set_num_threads(cores);
		}
		else if (cores > 0)
			omp_set_num_threads(cores);

		// Discard nodes that don't pass the height check. 
		auto valid_nodes = HeightCheckAllNodes(nodes, height, ert);
		
		// Calculate edges for every node in parallel
#pragma omp parallel for schedule(static)
		for (int i = 0; i < valid_nodes.size(); i++) {
			
			// Acquire the id of the node we're calculating this for
			int node_id = valid_nodes[i];
			
			// Get the node we're calculating the edges for
			const Node& node_a = nodes[node_id];
		
			// Get references to the cost and edge arrays for this node
			// to save an index operation  for every check.
			vector<int>& edge_list = edges[node_id];
			vector<float>& cost_list = costs[node_id];

			// Check connection between this node and every other valid node
			for (int k = 0; k < valid_nodes.size(); k++) {
				
				// Don't check this node against itself
				if (i == k) continue;
				
				// Get the next node from its index in valid_nodes
				int node_b_id = valid_nodes[k];

				// Calculate distance between node_a and node_b
				const Node& node_b = nodes[node_b_id];
				float distance = node_a.distanceTo(node_b);

				// Check if they have a clear line of sight. If they do, then add the distance
				// between them, and node_b's id to node_a's edge and cost array. 
				if (!IsOcclusionBetween(node_a, nodes[node_b_id], ert, height, distance)) {
					edge_list.push_back(node_b_id);
					cost_list.push_back(distance);
				}
			}
		}
		// Construct and return a graph from the set of nodes, the edge array
		// and the node array.
		return Graph(edges, costs, nodes);
	}


	Graph GroupToGroup(EmbreeRayTracer& ert, const vector<Node>& from, const vector<Node>& to, float height) {
		// Determine how many nodes are in both arrays
		const int from_count = from.size();
		const int to_count = to.size();

		// Create an jagged array of edges and costs based on how many nodes
		// are in from and to. Note that elements in the to part of the array
		// will not be used. since edges are only created from nodes in from
		vector<vector<int>> edges(from_count + to_count);
		vector<vector<float>> costs(from_count + to_count);


		// Use as many cores as the host machine can
		int cores = -1;
		if (cores < 0) {
			cores = std::thread::hardware_concurrency();
			omp_set_num_threads(cores);
		}
		else if (cores > 0) omp_set_num_threads(cores);

		// Perform height check on every node in nodes.
		auto valid_nodes = HeightCheckAllNodes(from, height, ert);
		auto valid_to_nodes = HeightCheckAllNodes(to, height, ert);

		// Iterate through every node in valid_nodes in parallel
#pragma omp parallel for schedule(static)
		for (int i = 0; i < valid_nodes.size(); i++) {
			
			// Get the id, node, and references to its arrays.
			int id = valid_nodes[i];
			const Node& node_a = from[id];
			vector<int>& edge_list = edges[id];
			vector<float>& cost_list = costs[id];

			// Check if it has a connection to every node in to
			for (int k = 0; k < valid_to_nodes.size(); k++) {

				// Get the node at this ID in to
				int to_id = valid_to_nodes[k];
				const Node& node_b = to[to_id];

				// Calculate the distance between node_a and node_b
				float distance = node_a.distanceTo(node_b);

				// Check if there's an occlusion between node_a and node_b. If they have
				// a clear line of sight, then add the edge and cost to node_a's arrays.
				if (!IsOcclusionBetween(node_a, node_b, ert, height, distance)) {

					edge_list.push_back(to_id + from_count);
					cost_list.push_back(distance);
				}
			}
		}

		// Copy all nodes into a single array.
		vector<Node> graph_nodes(from.size() + to.size());
		std::copy(from.begin(), from.end(), graph_nodes.begin());
		std::copy(to.begin(), to.end(), graph_nodes.begin() + from.size());

		// Create a new graph. 
		return Graph(edges, costs, graph_nodes);
	}

	Graph AllToAllUndirected(EmbreeRayTracer& ert, const vector<Node>& nodes, float height, int cores) {
		const int n = nodes.size();
		vector<vector<int>> edges(n);
		vector<vector<float>> costs(n);

		// Use as many cores as this machine has or the number of cores in cores
		if (cores < 0) {
			cores = std::thread::hardware_concurrency();
			omp_set_num_threads(cores);
		}
		else if (cores > 0)
			omp_set_num_threads(cores);

		// Perform a height check on every node
		const auto valid_nodes = HeightCheckAllNodes(nodes, height, ert);

		// Iterate through every node in nodes
#pragma omp parallel
		{
#pragma omp for schedule(dynamic)
			for (int i = 0; i < valid_nodes.size(); i++) {
				
				// Get the node and a reference to its edge/cost arrays
				int node_a_id = valid_nodes[i];
				const Node& node_a = nodes[node_a_id];
				vector<int>& edge_list = edges[node_a_id];
				vector<float>& cost_list = costs[node_a_id];


				// Check if it has a line of sight between every other node
				// in nodes.
				for (int k = i + 1; k < valid_nodes.size(); k++) {
					
					// Get the second node in nodes
					int node_b_id = valid_nodes[k];
					const Node& node_b = nodes[node_b_id];
					float distance = node_a.distanceTo(node_b);
					
					// Add an edge to this node's arrays if it has a line of sight
					if (!IsOcclusionBetween(node_a, nodes[k], ert, height, distance)) {
						edge_list.emplace_back(node_b_id);
						cost_list.emplace_back(distance);
					}
				}
			}
		}
		// Create and return a new graph from this information.
		return Graph(edges, costs, nodes);
	}
}