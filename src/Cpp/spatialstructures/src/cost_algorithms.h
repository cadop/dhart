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

#include "Constants.h"

namespace HF::SpatialStructures {
	struct Node;
	struct Edge;
	struct EdgeSet;
	struct IntEdge;
	class Graph;
	struct Subgraph;
}

namespace HF::SpatialStructures::CostAlgorithms {
	/*!
		\summary Converts floating-point value degrees into an equivalent value in radians
		\param degrees The value (in degrees) to convert into radians
		\returns The value, degrees, in radians

		\code
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::to_radians;

		double in_degrees = 360.0;
		double in_radians = to_radians(in_degrees); // 6.2831853.., about 2 * M_PI
		\endcode
	*/
	double to_radians(double degrees);
	
	/*!
		\summary Converts floating-point value radians into an equivalent value in degrees
		\param radians The value (in radians) to convert into degrees
		\returns The value, radians, in degrees

		\code
			// Requires #include "cost_algorithms.h"

			// for brevity
			using HF::SpatialStructures::CostAlgorithms::to_degrees;

			// The macro M_PI requires
			// #define _USE_MATH_DEFINES
			// #include <cmath>
			//
			// Use M_PI in place of approximate_pi when the value pi is needed.
			double approximate_pi = 3.14159;
			double in_radians = 2 * approximate_pi;

			double in_degrees = to_degrees(in_radians);
			// 359.99969592100859, about 360.0 degrees
		\endcode
	*/
	double to_degrees(double radians);

	/*!
		\summary Determines the distance between two points, point_a and point_b
		@tparam dimension of a real coordinate space R^n, where n is dimension, an integral value (i.e. R^2 is 2D, R^3 is 3D)
		@tparam float_precision a floating-point data type, i.e. float, double, long double, etc.
		\param point_a The starting point of a line segment
		\param point_b The ending point of a line segment
		\returns The euclidean distance from point_a to point_b, by the specified dimension

		\code
			// Requires #include "cost_algorithms.h"

			// for brevity;
			using HF::SpatialStructures::CostAlgorithms::euclidean_distance;

			// 2D coordinates, { x, y }
			float pos_a[] = { 0, 0 };
			float pos_b[] = { 4, 3 };

			// There are two template parameters for euclidean_distance:
			// - dimension (2 for 2D, 3 for 3D, etc)
			// - float_precision (defaults to float if unspecified)
			float distance = euclidean_distance<2>(pos_a, pos_b);	// 5.00000
		\endcode
	*/
	template <size_t dimension, typename float_precision = float>
	float_precision euclidean_distance(float_precision point_a[], float_precision point_b[]) {
		float_precision sum = 0;
		
		for (size_t i = 0; i < dimension; i++) {
			float_precision difference = point_b[i] - point_a[i];
			sum += std::pow(difference, 2.0);
		}

		return std::sqrt(sum);
	}

	/*!
		\summary Determines the dot product of vec_U and vec_V (vectors, as components)
		@tparam dimension of a real coordinate space R^n, where n is dimension, an integral value (i.e. R^2 is 2D, R^3 is 3D)
		@tparam float_precision a floating-point data type, i.e. float, double, long double, etc.
		\param vec_U The components of vector U
		\param vec_V The components of vector V
		\returns The dot product of vec_U and vec_V, by the specified dimension

		\code
			// Requires #include "cost_algorithms.h"

			// for brevity
			using HF::SpatialStructures::CostAlgorithms::dot_product;

			// components of a 2D vector, { v_x, v_y }
			float vec_u[] = { 4, 0 };
			float vec_v[] = { 0, 4 };

			// There are two template parameters for dot_product:
			// - dimension (2 for 2D, 3 for 3D, etc)
			// - float_precision (defaults to float if unspecified)
			float dot_prod = dot_product<2>(vec_u, vec_v);	// 0.00000

			// vec_u and vec_v are perpendicular.
		\endcode
	*/
	template <size_t dimension, typename float_precision = float>
	float_precision dot_product(float_precision vec_U[], float_precision vec_V[]) {
		float_precision sum = 0;

		for (size_t i = 0; i < dimension; i++) {
			sum += vec_U[i] * vec_V[i];
		}

		return sum;
	}

	/*!
		\summary Determines if vec_U and vec_V (vectors, as components) are perpendicular, or not
		@tparam dimension of a real coordinate space R^n, where n is dimension, an integral value (i.e. R^2 is 2D, R^3 is 3D)
		@tparam float_precision a floating-point data type, i.e. float, double, long double, etc.
		\param vec_U The components of vector U
		\param vec_V The components of vector V
		\returns True, if vec_U and vec_V have a dot product close to 0, false otherwise

		\code
			// Requires #include "cost_algorithms.h"

			// for brevity
			using HF::SpatialStructures::CostAlgorithms::is_perpendicular;

			// components of a 2D vector, { v_x, v_y }
			float vec_u[] = { 4, 0 };
			float vec_v[] = { 0, 4 };

			// There are two template parameters for is_perpendicular:
			// - dimension (2 for 2D, 3 for 3D, etc)
			// - float_precision (defaults to float if unspecified)
			bool is_perp = is_perpendicular<2>(vec_u, vec_v);	// true

			// vec_u and vec_v are perpendicular.
		\endcode
	*/
	template <size_t dimension, typename float_precision = float>
	bool is_perpendicular(float_precision vec_U[], float_precision vec_V[]) {
		// Mathematically,
		// two vectors are perpendicular if their dot product is
		// equal to zero. But since it is a mortal sin to test
		// floating point numbers for equality (using operator==) --
		// we can test if the dot product of
		// vector_a and vector_b is 'close enough' to zero,
		// by determining if our dot_product calculation
		// is less than our ROUNDING_PRECISION constant.
		// (which is 0.0001)    
		return std::abs(dot_product<dimension, float_precision>(vec_U, vec_V)) 
			< HF::SpatialStructures::ROUNDING_PRECISION;
	}

	/*!
		\summary Obtains a container of Edge that are perpendicular to the vector formed by parent_node and child_node_a
		\param sg A Subgraph that consists of a parent node and a container of Edges by that parent node
		\paramchild_node_a The child node that forms a vector (edge) with parent_node, that will be compared with all Edge in edges
		\returns A container of Edge that were found to be perpendicular to the edge formed by parent_node and child_node_a

		\code
			// Requires #include "cost_algorithms.h"

			// for brevity
			using HF::SpatialStructures::CostAlgorithms::GetPerpendicularEdges;

			// Create 7 nodes
			Node n0(2, 6, 6);
			Node n1(0, 0, 0);
			Node n2(-5, 5, 4);
			Node n3(-1, 1, 1);
			Node n4(2, 2, 2);
			Node n5(5, 3, 2);
			Node n6(-2, -5, 1);

			Graph g;

			// Adding 9 edges
			g.addEdge(n0, n1);	// [ -2, -6, -6 ]
			g.addEdge(n1, n2);	// [ -5,  5,  4 ]
			g.addEdge(n1, n3);  // [ -1,  1,  1 ]
			g.addEdge(n1, n4);  // [  2,  2,  2 ]
			g.addEdge(n2, n4);  // [ -9, -3, -2 ]
			g.addEdge(n3, n5);  // [ -6,  2,  1 ]
			g.addEdge(n5, n6);  // [ -7, -8, -1 ]
			g.addEdge(n4, n6);  // [ -6, -7, -1 ]

			// Always compress the graph after adding edges!
			g.Compress();

			// We will find all edges in g that are perpendicular to the edge (vector)
			// formed by n1 and n4 -- they will be stored in perp_edges.
			// Note that we have specifically chosen to check for 2D perpendicularity,
			// as seen by the template parameter, despite these nodes being 3D.
			auto perp_edges = GetPerpendicularEdges<2>(g.GetSubgraph(n1), n4);

			// The edge formed by n1 and n4 is perpendicular to
			// - the edge formed by n1 and n2
			// - the edge formed by n1 and n3
		\endcode
	*/
	template <size_t dimension, typename float_precision = float>
	std::vector<Edge> GetPerpendicularEdges(const Subgraph& sg, const Node& child_node_a) {
		std::vector<Edge> perpendicular_edges;

		auto& parent_node = sg.m_parent;
		auto& edges = sg.m_edges;

		for (Edge edge_b : edges) {
			Node child_node_b = edge_b.child;
			// We iterate over all children by parent_node.
			// The goal is to compare the vector formed by parent_node and child_a,
			// with the vectors formed by parent_node and all other child node by this parent, all other child_b's.

			/*
			std::cout << "parent " << parent_node.id << " has child " << edge_b.child.id
				<< " with data " << edge_b.score << std::endl;
			*/

			if (child_node_a != child_node_b) {
				//
				//	Here -- we should assess the step_type field for edge_b.
				//
				//	/// <summary> Describes the type of step an edge connects to. </summary>
				//	enum STEP {
				//		NOT_CONNECTED = 0, ///< No connection between parent and child.
				//		NONE = 1,		 ///< Parent and child are on the same plane and no step is required.
				//		UP = 2,			///< A step up is required to get from parent to child.
				//		DOWN = 3,		///< A step down is required to get from parent to child.
				//		OVER = 4		///< A step over something is required to get from parent to child.
				//	};
				//

				// Retrieve the { x, y, z } components of the vectors formed by
				//	parent_node and child_node_a
				//	parent_node and child_node_b
				auto vector_a = parent_node.directionTo(child_node_a);
				auto vector_b = parent_node.directionTo(child_node_b);

				if (is_perpendicular<dimension, float_precision>(vector_a.data(), vector_b.data())) {
					// If this evaluates true,
					// we add edge_b to perpendicular_edges.
					perpendicular_edges.push_back(edge_b);
				}
			}
			else {
				/*
				// If child_node_b is the same as the child_node we passed in, skip it.
				std::cout << " *** SKIPPED ***" << std::endl;
				*/
			}
		}

		return perpendicular_edges;
	}

	/*!
		\summary Calculates cross slope for this subgraph (a parent node and all edges by this parent)
		\param sg A Subgraph type, which consists of a Node (parent node) and a std::vector<Edge> (all edges by this parent)
		\returns A container of IntEdge

		\code
			// for brevity
			using HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope;

			// Create 7 nodes
			Node n0(2, 6, 6);
			Node n1(0, 0, 0);
			Node n2(-5, 5, 4);
			Node n3(-1, 1, 1);
			Node n4(2, 2, 2);
			Node n5(5, 3, 2);
			Node n6(-2, -5, 1);

			Graph g;

			// Adding 9 edges
			g.addEdge(n0, n1);	// [ -2, -6, -6 ]
			g.addEdge(n1, n2);	// [ -5,  5,  4 ]
			g.addEdge(n1, n3);  // [ -1,  1,  1 ]
			g.addEdge(n1, n4);  // [  2,  2,  2 ]
			g.addEdge(n2, n4);  // [ -9, -3, -2 ]
			g.addEdge(n3, n5);  // [ -6,  2,  1 ]
			g.addEdge(n5, n6);  // [ -7, -8, -1 ]
			g.addEdge(n4, n6);  // [ -6, -7, -1 ]

			// Always compress the graph after adding edges!
			g.Compress();

			// Retrieve a subgraph of your choice, provide a parent node or parent node ID.
			Subgraph sg = g.GetSubgraph(n1);

			// Get a container of vector<IntEdge>, ordered by parent ID.
			// These consist of alternate edge costs for Subgraph sg.
			std::vector<IntEdge> edge_costs = CalculateCrossSlope(sg);
		\endcode
	*/
	std::vector<IntEdge> CalculateCrossSlope(const Subgraph& sg);

	/*!
		\summary Calculates cross slope for all subgraphs in Graph g
		\param g The Graph to calculate cross slopes with
		\returns A container of vector<IntEdge>, ordered by parent node ID

		\code
			// for brevity
			using HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope;

			// Create 7 nodes
			Node n0(0, 0, 0);
			Node n1(1, 3, 5);
			Node n2(3, -1, 2);
			Node n3(1, 2, 1);
			Node n4(4, 5, 7);
			Node n5(5, 3, 2);
			Node n6(-2, -5, 1);

			Graph g;

			// Adding 9 edges
			g.addEdge(n0, n1);
			g.addEdge(n1, n2);
			g.addEdge(n1, n3);
			g.addEdge(n1, n4);
			g.addEdge(n2, n4);
			g.addEdge(n3, n5);
			g.addEdge(n5, n6);
			g.addEdge(n4, n6);

			// Always compress the graph after adding edges!
			g.Compress();

			// Get a container of vector<IntEdge>, ordered by parent ID.
			// These consist of alternate edge costs for all subgraphs in g.
			std::vector<std::vector<IntEdge>> all_edge_costs = CalculateCrossSlope(g);
		\endcode
	*/
	std::vector<std::vector<IntEdge>> CalculateCrossSlope(const Graph& g);


	
	/*!
		\summary Calculate energy expenditure for this subgraph (a parent node and all edges by this parent)
		\param sg A subgraph type which consists of a Node (parent node) and a std::vector<Edge> (all edges by this parent)
		\returns A container of EdgeSet

		\code
			// For brevity
			using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;

			// Create 7 nodes
			Node n0(0, 0, 0);
			Node n1(1, 3, 5);
			Node n2(3, -1, 2);
			Node n3(1, 2, 1);
			Node n4(4, 5, 7);
			Node n5(5, 3, 2);
			Node n6(-2, -5, 1);

			Graph g;

			// Adding 9 edges
			g.addEdge(n0, n1);
			g.addEdge(n1, n2);
			g.addEdge(n1, n3);
			g.addEdge(n1, n4);
			g.addEdge(n2, n4);
			g.addEdge(n3, n5);
			g.addEdge(n5, n6);
			g.addEdge(n4, n6);

			// Always compress the graph after adding edges!
			g.Compress();

			// Retrieve a subgraph of your choice, provide a parent node or parent node ID.
			Subgraph sg = g.GetSubgraph(n1);

			// Get a container of vector<EdgeSet>, ordered by parent ID.
			// These consist of alternate edge costs for Subgraph sg.
			std::vector<EdgeSet> edge_costs = CalculateEnergyExpenditure(sg);
		\endcode
	*/
	EdgeSet CalculateEnergyExpenditure(const Subgraph& sg);

	/*!
		\summary Calculates energy expenditure for all subgraphs in Graph g
		\param g The graph to calculate energy expenditures with
		\returns A container of vector<EdgeSet>, ordered by parent node ID

		\code
			// for brevity
			using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;

			// Create 7 nodes
			Node n0(0, 0, 0);
			Node n1(1, 3, 5);
			Node n2(3, -1, 2);
			Node n3(1, 2, 1);
			Node n4(4, 5, 7);
			Node n5(5, 3, 2);
			Node n6(-2, -5, 1);

			Graph g;

			// Adding 9 edges
			g.addEdge(n0, n1);
			g.addEdge(n1, n2);
			g.addEdge(n1, n3);
			g.addEdge(n1, n4);
			g.addEdge(n2, n4);
			g.addEdge(n3, n5);
			g.addEdge(n5, n6);
			g.addEdge(n4, n6);

			// Always compress the graph after adding edges!
			g.Compress();

			// Get a container of vector<EdgeSet>, ordered by parent ID.
			// These consist of alternate edge costs for all subgraphs in g.
			std::vector<std::vector<EdgeSet>> all_edge_costs = CalculateEnergyExpenditure(g);
		\endcode
	*/
	std::vector<EdgeSet> CalculateEnergyExpenditure(const Graph& g);

	/*
		\summary Calculates the slope between two nodes 
		\param parent A Node
		\param child A Node
		\returns A double of the Angle
	*/
	double CalculateSlope(Node& parent, Node& child);
}

#endif /// COST_ALGORITHMS_H
