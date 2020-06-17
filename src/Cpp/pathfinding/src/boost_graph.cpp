///
///	\file		boost_graph.cpp
///	\brief		Source file for BoostGraph
///
///	\author		TBA
///	\date		17 Jun 2020
///
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
		int num_nodes = graphg.size();
		vector<int> nodes(num_nodes);

		std::vector<pair> edges;
		std::vector<Edge_Cost> weights;

		auto edge_sets = graphg.GetEdges();
		for (const auto& edgeset : edge_sets) {
			const int parent_id = edgeset.parent;
			for (const auto& edge : edgeset.children) {
				int child_id = edge.child;
				float cost = edge.weight;

				weights.emplace_back(Edge_Cost{ edge.weight });
				edges.emplace_back(pair{ parent_id,child_id });
			}
		}

		unsigned int max_node = graphg.Nodes()[num_nodes - 1].id + 1;
		g = graph_t(boost::edges_are_unsorted, edges.begin(), edges.end(), weights.begin(), max_node);

		int n = num_vertices(g);
		p.resize(n);
		d.resize(n);
	}

	BoostGraph::~BoostGraph() = default;
}
