///
///	\file		boost_graph.cpp
/// \brief		Contains implementation for the <see cref="HF::Pathfinding::BoostGraph">BoostGraph</see> class
///
///	\author		TBA
///	\date		17 Jun 2020

#include <boost_graph.h>

#include <graph.h>
#include <node.h>
#include <Edge.h>
#include <assert.h>

using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using std::vector;

namespace HF::Pathfinding {
	BoostGraph::BoostGraph(const HF::SpatialStructures::Graph& graphg)
	{

		// Allocate a vector of integers with an element for every node in
		// nodes.
		int num_nodes = graphg.size();
		vector<int> nodes(num_nodes);

		std::vector<pair> edges;
		std::vector<Edge_Cost> weights;

		// Iterate through every edgeset in the graph
		auto edge_sets = graphg.GetEdges();

		for (const auto& edgeset : edge_sets) {
			const int parent_id = edgeset.parent;

			// Iterate through all children of this parent
			for (const auto& edge : edgeset.children) {
				
				// Add the edge to the boost graph's edge array and the weight
				// to the boost graph's weights array
				int child_id = edge.child;
				float cost = edge.weight;
				weights.emplace_back(Edge_Cost{ edge.weight });
				edges.emplace_back(pair{ parent_id,child_id });
			}
		}

		// Calculate the maximum id held by the graph.
		unsigned int max_node = graphg.Nodes()[num_nodes - 1].id + 1;

		// Create the boost graph from the two input arrays
		g = graph_t(boost::edges_are_unsorted, edges.begin(), edges.end(), weights.begin(), max_node);

		int n = num_vertices(g);

		// Resize predecessor and distance arrays to maximum size.
		p.resize(n);
		d.resize(n);
	}

	BoostGraph::~BoostGraph() = default;
}
