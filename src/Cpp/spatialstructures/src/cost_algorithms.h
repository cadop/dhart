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
#include <string>

namespace HF::SpatialStructures {
	struct Node;
	struct Edge;
	struct EdgeSet;
	struct IntEdge;
	class Graph;
}

namespace HF::SpatialStructures::CostAlgorithms {
	bool is_floating_type(std::string value);

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
}


#endif /// COST_ALGORITHMS_H