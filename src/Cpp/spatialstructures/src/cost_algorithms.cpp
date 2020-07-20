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
			auto angle_y_axis = to_degrees(std::acos(dir[1] / magnitude));
			//auto angle_z_axis = to_degrees(std::acos(dir[2] / magnitude));

			auto angle = angle_y_axis;
			auto slope = std::clamp(std::tanf(angle), -0.4f, -0.4f);

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
}
