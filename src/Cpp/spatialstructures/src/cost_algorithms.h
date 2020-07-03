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
	struct IntEdge;
	class Graph;
}

namespace HF::SpatialStructures::CostAlgorithms {
	/// <summary>Calculates cross slope for all edges in Graph g</summary>
	/// <param name="g">The Graph to calculate cross slopes with</param>
	/// <returns>A container of IntEdge, ordered by parent node ID</return>

	/*!
		\code
			// TODO example
		\endcode
	*/
	std::vector<IntEdge> CalculateCrossSlope(Graph& g);
}


#endif /// COST_ALGORITHMS_H