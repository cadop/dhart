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

	double CalculateSlope(Node& parent, Node& child)
	{
		// Calculates the Slope between two nodes as an angle in degrees.
		// This could be split into two functions later since the first commented part is simply rise/run
		// The second part is a generic cosine angle between vectors 

		// Calculate slope with Rise over Run using atan2
		/*
		double run = std::sqrt( std::pow(parent[0]-child[0],2) + std::pow(parent[1]-child[1],2) );
		double rise = parent[2] - child[2];
		double slope = to_degrees(std::atan2(rise, run));
		*/

		// Calculate slope with cosine of two vectors

		// Get the vector between the points for the first vector
		double n1 = child[0] - parent[0];
		double n2 = child[1] - parent[1];
		double n3 = child[2] - parent[2];

		// Second vector is the z unit vector
		double u1 = 0;
		double u2 = 0;
		double u3 = 1;

		// double numerator = std::abs((n1 * u1) + (n2 * u2) + (n3 * u3));
		// Which reduces to:
		double numerator = std::abs(n3);

		// Distance of first vector 
		double denom = std::sqrt(std::pow(n1, 2) + std::pow(n2, 2) + std::pow(n3, 2));

		// Distance of second vector 
		// double denom_2 = std::abs(std::sqrt(std::pow(u1, 2) + std::pow(u2, 2) + std::pow(u3, 2)));
		// Which reduces to:
		//	double denom_2 = std::sqrt(std::pow(u3, 2));
		//  and then
		//  double denom_2 = u3;
		// Which is 1

		double res = std::asin(numerator/denom);

		int direc;
		if (child[2] > parent[2]){
			direc = 1;
		}

		else{
			direc = -1;
		}

		double slope = to_degrees(res) * direc;

		return slope;
		
	}

	std::vector<EdgeSet> CalculateEnergyExpenditure(Subgraph& sg) {
		// Energy expenditure data will be stored here and returned from this function.
		std::vector<EdgeSet> edge_set;

		// From Subgraph sg
		Node parent_node = sg.m_parent;
		std::vector<Edge> edge_list = sg.m_edges;

		// We can preallocate this container to have edge_list.size()
		// blocks since we know exactly how many children make up the subgraph.
		std::vector<IntEdge> children(edge_list.size());
		auto it_children = children.begin();

		for (Edge link_a : edge_list) {
			// Retrieve vector components of the vector formed by
			// parent_node and link_a

			auto dir = parent_node.directionTo(link_a.child);
			auto magnitude = parent_node.distanceTo(link_a.child);

			//
			//	Angle formula in R3 is:
			//	acos(dot_product(vector_U, vector_V) / magnitude(vector_U) * magnitude(vector_V));
			//
			//	vector_V will be a unit vector i, j, or k for x, y, and z axes respectively.
			//	Therefore, we simplify these angle formulas to:
			//	acos(component_value / magnitude(vector_U))
			//	component_value will be dir[0], dir[1], or dir[2] for x, y, or z components respectively.
			//
			//auto angle_x_axis = to_degrees(std::acos(dir[0] / magnitude));
			//auto angle_y_axis = to_degrees(std::acos(dir[1] / magnitude));
			//auto angle_z_axis = to_degrees(std::acos(dir[2] / magnitude));

			//auto angle = angle_y_axis;
			//auto slope = std::clamp(std::tanf(angle), -0.4f, -0.4f);

			double slope = CalculateSlope(parent_node, link_a.child);

			auto e = 280.5
				* (std::pow(slope, 5)) - 58.7
				* (std::pow(slope, 4)) - 76.8
				* (std::pow(slope, 3)) + 51.9
				* (std::pow(slope, 2)) + 19.6
				* (slope) + 2.5;

			assert(e > 0);

			// Calculate the new score/distance for the IntEdge
			auto expenditure = e * magnitude;

			// Create the resulting IntEdge from the current child ID and calculation
			IntEdge ie = { link_a.child.id, expenditure }; 

			// Dereference the vector<IntEdge> iterator, assign it to the IntEdge created,
			// then advance the iterator.
			*(it_children++) = ie;
		}
		
		EdgeSet es = { parent_node.id, children };
		edge_set.push_back(es);

		return edge_set;
	}

	std::vector<std::vector<EdgeSet>> CalculateEnergyExpenditure(Graph& g) {
		// Retrieve all nodes from g so we can obtain subgraphs.
		std::vector<Node> nodes = g.Nodes();

		// The result container will always be, at most, the node count of g.
		// We can preallocate this memory so we do not have to resize during the loop below.
		std::vector<std::vector<EdgeSet>> result(nodes.size());
		auto it_result = result.begin();

		for (Node parent_node : nodes) {
			// For all Node in g...

			// Get the Subgraph via parent_node
			Subgraph sg = g.GetSubgraph(parent_node);

			// Call CalculateEnergyExpenditure using the returned Subgraph,
			// get a vector<EdgeSet> back
			std::vector<EdgeSet> energy_expenditures = CalculateEnergyExpenditure(sg);

			// Dereference it_result, assign it to the energy_expenditures container,
			// then advance it_result (std::vector<std::vector<EdgeSet>>::iterator)
			*(it_result++) = energy_expenditures;
		}

		return result;
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
