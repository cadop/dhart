///
///	\file		cost_algorithms.h
///	\brief		Contains implementation for the <see cref="HF::SpatialStructures::CostAlgorithms">HF::SpatialStructures::CostAlgorithms</cref> namespace
///
///	\author		TBA
///	\date		03 Jul 2020
///

#include "cost_algorithms.h"
#include "graph.h"
#include "Constants.h"

#include <iostream>

using HF::SpatialStructures::IntEdge;
using HF::SpatialStructures::Graph;

bool HF::SpatialStructures::CostAlgorithms::is_perpendicular(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    bool perpendicular = false;

    float dot_product =
        ((dir_a[0] * dir_b[0])
            + (dir_a[1] * dir_b[1])
            + (dir_a[2] * dir_b[2]));

    // Mathematically,
    // two vectors are perpendicular if their dot product is
    // equal to zero. But since it is a mortal sin to test
    // floating point numbers for equality (using operator==) --
    // we can test if the dot product of
    // vector_a and vector_b is 'close enough' to zero,
    // by determining if our dot_product calculation
    // is less than our ROUNDING_PRECISION constant.
    // (which is 0.0001)    
    perpendicular = std::abs(dot_product) < HF::SpatialStructures::ROUNDING_PRECISION;

    return perpendicular;
}

std::vector<IntEdge> HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope(Graph& g) {
    // All cross slope data will be stored here and returned from this function.
    std::vector<IntEdge> result;

    // Retrieve all parent nodes from g at once.
    std::vector<Node> parents = g.Nodes();

    for (Node parent_node : parents) {
        // We iterate through all parent nodes,
        // and retrieve all children in the process --
        // as well as the edges they form.
        std::vector<Node> children = g.GetChildren(parent_node);
        std::vector<Edge> edges = g[parent_node];

        for (Edge edge_a : edges) {
            // We iterate over all edges that extend from parent_node.
            Node child_node_a = edge_a.child;
            float edge_data_a = edge_a.score;

            std::cout << "parent " << parent_node.id << " has child " << child_node_a.id
                << " with data " << edge_data_a << std::endl;

            std::cout << "====== Comparing with other edges ======"
                << std::endl;

            // We must have a container to store all perpendicular edges found.
            std::vector<Edge> perpendicular_edges = GetPerpendicularEdges(parent_node, g[parent_node], child_node_a);

            std::cout << "====== End of comparing with other edges ======"
                << std::endl;

            float weight = 0.0;
            float a_z = 0.0;
            float b_z = 0.0;
            float c_z = 0.0;

            switch (perpendicular_edges.size()) {
            case 0:
                // No edges were found to be perpendicular to the edge
                // formed by node parent_node and node child_node_a.
                // The IntEdge to be created will use the existing edge data
                // of parent_node and child_node_a (edge_data_a)

                weight = edge_data_a;
                break;
            case 1:
                // One edge formed by node parent_node and one other child_node by parent_node
                // was found to be perpendicular to the edge formed by
                // node parent_node and node child_node_a.

                a_z = child_node_a.z;

                // z value of other child_node
                b_z = perpendicular_edges[0].child.z;

                // add the existing weight value to the delta of the z values
                weight = std::abs(a_z - b_z) + perpendicular_edges[0].score;
                break;
            case 2:
                // Two edges -- each formed by node parent_node and two separate
                // child node by node parent_node were found to be perpendicular to the edge
                // formed by node parent_node and node child_node_a.

                a_z = child_node_a.z;

                // z value of the first other child_node
                b_z = perpendicular_edges[0].child.z;

                // z value of the second other child_node
                c_z = perpendicular_edges[1].child.z;

                // add the existing weight value to the delta of the z values
                weight = std::abs(b_z - c_z) + perpendicular_edges[0].score;
                break;
            default:
                break;
            }

            // Create the IntEdge using child_node_a.id
            // and the cross slope value stored in weight --
            // then add it to our result container.
            IntEdge ie = { child_node_a.id, weight };
            result.push_back(ie);
        }

        std::cout << "***** ALL CHILDREN FOR THIS PARENT DONE *****\n"
            << std::endl;
    }

    return result;
}

std::vector<HF::SpatialStructures::Edge> HF::SpatialStructures::CostAlgorithms::GetPerpendicularEdges(HF::SpatialStructures::Node& parent_node, std::vector<Edge> edges, Node& child_node_a) {
    std::vector<HF::SpatialStructures::Edge> perpendicular_edges;

    for (Edge edge_b : edges) {
        Node child_node_b = edge_b.child;
        // We iterate over all children for parent_node,
        // and create a vector_b from parent_node and the current child,
        // child_node_b. 
        // The goal is to compare vector_a with every other
        // vector_b created by the common parent_node and its children.
        std::cout << "parent " << parent_node.id << " has child " << edge_b.child.id
            << " with data " << edge_b.score << std::endl;

        if (child_node_a == child_node_b) {
            // If child_node_b is the same as the child_node we passed in, skip it.
            std::cout << " *** SKIPPED ***" << std::endl;
        }
        else {
            auto vector_a = parent_node.directionTo(child_node_a);
            auto vector_b = parent_node.directionTo(child_node_b);

            if (is_perpendicular(vector_a, vector_b)) {
                // If this evaluates true,
                // we add edge_b to perpendicular_edges.
                perpendicular_edges.push_back(edge_b);
            }
        }
    }

    return perpendicular_edges;
}
