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
	struct EdgeSet;
	struct IntEdge;
	class Graph;
}

namespace HF::SpatialStructures::CostAlgorithms {
	double to_radians(double degrees);
	double to_degrees(double radians);

	std::vector<EdgeSet> CalculateEnergyExpenditure(Graph& g);

	float calculate_magnitude(std::array<float, 3> dir_a, std::array<float, 3> dir_b);
	float calculate_dot_product(std::array<float, 3> dir_a, std::array<float, 3> dir_b);
	
	/// <summary> Determines if dir_a and dir_b (vectors) are perpendicular, or not </summary>
	/// <param name="dir_a">The starting position</param>
	/// <param name="dir_b">The ending position</param>
	/// <returns>True, if dir_a and and dir_b have a dot product close to 0, false otherwise</returns>

	/*!
		\code
			// TODO example
		\endcode
	*/
	bool is_perpendicular(std::array<float, 3> dir_a, std::array<float, 3> dir_b);

	/// <summary>Calculates cross slope for all edges in Graph g</summary>
	/// <param name="g">The Graph to calculate cross slopes with</param>
	/// <returns>A container of IntEdge, ordered by parent node ID</returns>

	/*!
		\code
			// TODO example
		\endcode
	*/
	std::vector<IntEdge> CalculateCrossSlope(Graph& g);

	/// <summary>Calculates cross slope for all edges in Graph g (utilizes CSR representation)</summary>
	/// <param name="g">The Graph to calculate cross slopes with</param>
	/// <returns>A container of IntEdge, ordered by parent node ID</returns>

	/*!
		\code
			// TODO example
		\endcode
	*/
	std::vector<IntEdge> CalculateCrossSlopeCSR(Graph& g);

	/// <summary> Obtains a container of Edge that are perpendicular to the vector formed by parent_node and child_node_a</summary>
	/// <param name="parent_node">The parent node</param>
	/// <param name="edges">The Edge that extend from parent_node</param>
	/// <param name="child_node_a">The child node that forms a vector (edge) with parent_node, that will be compared with all Edge in edges</param>
	/// <returns>A container of Edge that were found to be perpendicular to the edge formed by parent_node and child_node_a</returns>

	/*!
		\code
			// TODO example
		\endcode
	*/
	std::vector<Edge> GetPerpendicularEdges(Node& parent_node, std::vector<Edge> edges, Node& child_node_a);
}


#endif /// COST_ALGORITHMS_H