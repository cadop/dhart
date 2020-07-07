///
///	\file		cost_algorithms.h
///	\brief		Contains declarations for the <see cref="HF::SpatialStructures::CostAlgorithms">HF::SpatialStructures::CostAlgorithms</cref> namespace
///
///	\author		TBA
///	\date		03 Jul 2020
///

#ifndef COST_ALGORITHMS_H
#define COST_ALGORITHMS_H

#include <vector>

namespace HF::SpatialStructures {
	struct Node;
	struct Edge;
	struct IntEdge;
	class Graph;
}

namespace HF::SpatialStructures::CostAlgorithms {
	bool is_perpendicular(std::array<float, 3> dir_a, std::array<float, 3> dir_b);

	/// <summary>Calculates cross slope for all edges in Graph g</summary>
	/// <param name="g">The Graph to calculate cross slopes with</param>
	/// <returns>A container of IntEdge, ordered by parent node ID</return>

	/*!
		\code
			// TODO example
		\endcode
	*/
	std::vector<IntEdge> CalculateCrossSlope(Graph& g);

	//std::vector<Edge> GetPerpendicularEdges(Node& parent_node, std::vector<Node> children);
	std::vector<Edge> GetPerpendicularEdges(Node& parent_node, std::vector<Edge> edges, Node& child_node_a);
}


#endif /// COST_ALGORITHMS_H