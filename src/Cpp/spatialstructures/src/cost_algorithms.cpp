///
///	\file		cost_algorithms.h
///	\brief		Contains implementation for the <see cref="HF::SpatialStructures::CostAlgorithms">HF::SpatialStructures::CostAlgorithms</cref> namespace
///
///	\author		TBA
///	\date		03 Jul 2020
///

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

#include "cost_algorithms.h"
#include "graph.h"
#include "Constants.h"

#include <iostream>

using HF::SpatialStructures::Node;
using HF::SpatialStructures::IntEdge;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Subgraph;
using HF::SpatialStructures::EdgeSet;

namespace HF::SpatialStructures::CostAlgorithms {
	double to_radians(double degrees) {
		return degrees * (M_PI / 180);
	}

	double to_degrees(double radians) {
		return radians * (180 / M_PI);
	}

	std::vector<IntEdge> CalculateCrossSlope(Subgraph& sg) {
		// All cross slope data for Subgraph sg will be stored here 
		// and returned from this function.
		std::vector<IntEdge> result;

		Node parent_node = sg.m_parent;
		std::vector<Edge> edges = sg.m_edges;

		for (Edge edge_a : edges) {
			// We iterate over all edges that extend from parent_node. 
			Node child_node_a = edge_a.child;
			float edge_data_a = edge_a.score;

			//
			//	Here -- we should assess the step_type field for edge_a.
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

			// We must have a container to store all perpendicular edges found.
			// This container will have all edges that are perpendicular to edge_a --
			// or rather, the vector formed by parent_node and child_node_a.

			// Note that we are checking for perpendicularity in 2D space only.
			std::vector<Edge> perpendicular_edges = GetPerpendicularEdges<2>(sg, child_node_a);

			float weight = 0.0;
			float cross_slope = 0.0;
			float a_z = 0.0;
			float b_z = 0.0;
			float c_z = 0.0;

			switch (perpendicular_edges.size()) {
			case 0:
				// No edges were found to be perpendicular to the edge
				// formed by node parent_node and node child_node_a.
				// The IntEdge to be created will use the existing edge data
				// of parent_node and child_node_a (edge_data_a).
				weight = edge_data_a;
				break;
			case 1:
				// One edge formed by node parent_node and another child_node
				// was found to be perpendicular to the edge formed by
				// node parent_node and node child_node_a.

				a_z = child_node_a.z;

				// z value of other child_node
				b_z = perpendicular_edges[0].child.z;

				cross_slope = std::abs(a_z - b_z);

				// add the existing weight value
				weight = cross_slope + perpendicular_edges[0].score;
				break;
			case 2:
				// Two edges -- each formed by node parent_node and two separate child node
				// were found to be perpendicular to the edge formed by
				// node parent_node and node child_node_a.

				a_z = child_node_a.z;

				// z value of the first other child_node
				b_z = perpendicular_edges[0].child.z;

				// z value of the second other child node
				c_z = perpendicular_edges[1].child.z;

				cross_slope = std::abs(b_z - c_z);

				// add the existing weight value
				weight = std::abs(b_z - c_z) + perpendicular_edges[0].score;
				break;
			default:
				break;
			}

			// Create the IntEdge using child_node_a.id
			// and the (cross slope value + existing edge score)
			// -- then add it to our result container.
			IntEdge ie = { child_node_a.id, weight };
			result.push_back(ie);
		}
	
		return result;
	}

	std::vector<std::vector<IntEdge>> CalculateCrossSlope(Graph& g) {
		// Retrieve all nodes from g so we can obtain subgraphs.
		std::vector<Node> nodes = g.Nodes();

		// The result container will always be, at most, the node count of g.
		// We can preallocate this memory so we do not have to resize during the loop below.
		std::vector<std::vector<IntEdge>> result(nodes.size());
		auto it_result = result.begin();

		for (Node parent_node : nodes) {
			// For all Node in g...

			// Get the Subgraph via parent_node
			Subgraph sg = g.GetSubgraph(parent_node);

			// Call CalculateCrossSlope using the returned Subgraph,
			// get a vector<IntEdge> back
			std::vector<IntEdge> cross_slopes = CalculateCrossSlope(sg);

			// Dereference it_result, assign it to the cross_slopes container,
			// then advance it_result (std::vector<std::vector<IntEdge>>::iterator)
			*(it_result++) = cross_slopes;
		}

		return result;
	}
}
