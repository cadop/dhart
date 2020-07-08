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
    return std::abs(dot_product) < HF::SpatialStructures::ROUNDING_PRECISION;
}

std::vector<IntEdge> HF::SpatialStructures::CostAlgorithms::CalculateCrossSlopeCSR(Graph& g) {
    // All cross slope data will be stored here and returned from this function.
    std::vector<IntEdge> result;

    // Retrieve the CSR representation of Graph g
    CSRPtrs csr = g.GetCSRPointers();
    
    // last_index is the last node parent ID in g
    const int last_index = csr.rows - 1;

    for (int parent_id = 0; parent_id < csr.rows; parent_id++) {
        // From [parent_id = 0, csr.rows); iterating over all parent IDs

        // True if parent_id is the last index for the csr.rows buffer
        bool at_last_parent = parent_id == last_index;

        // Denotes the beginning index for a row (parent_id) within csr.outer_indices/csr.data
        int curr_row_index = csr.outer_indices[parent_id];

        // May denote the beginning index for the next row (parent_id + 1) within csr.outer_indices/csr.data
        // ***if at_last_parent == true, then csr.nnz is used instead of next_row_index.
        // We do not want the loops below to dereference an unwanted address.
        int next_row_index = csr.outer_indices[parent_id + 1];

        // row_end_index == csr.nnz if we are at the last parent index
        // otherwise row_end_index == next_row_index
        const int row_end_index = at_last_parent ? csr.nnz : next_row_index;

        // Retrieve the parent node with the current parent_id.
        // This is equivalent to g.ordered_nodes[parent_id],
        // since Graph::NodeFromID is literally g.ordered_nodes[index]
        Node parent_node = g.NodeFromID(parent_id);
   
        if (row_end_index < csr.nnz) {
            std::cout << "***** row_begin = " << csr.data[curr_row_index] << "\t"
                << "row_end = " << csr.data[row_end_index] << " *****"
                << std::endl;
        }
        else {
            std::cout << "***** row_begin = " << csr.data[curr_row_index] << "\t"
                << "row_end = "
                << "null"
                << " *****" << std::endl;
        }

        for (int i = curr_row_index; i < row_end_index; i++) {
            // From [i = curr_row_index, row_end_index); all children belonging to parent_id

            // Retrieve the current child ID from inner_indices.
            int child_id = csr.inner_indices[i];

            // Retrieve the current child node with child_id.
            // This is equivalent to g.ordered_nodes[child_id],
            // since Graph::NodeFromID is literally g.ordered_nodes[index]
            Node child_node_a = g.NodeFromID(child_id);

            // Retrieve all Edge that extend from parent_node.
            // We do this, as opposed to referencing the CSR (using csr.data[i], which is score)
            // because Edge objects store an enum STEP, named step_type,
            // which describes the type of step an edge connects to --
            // in addition to a Node child field, as well as a float score field.
            std::vector<Edge> edges = g[parent_node];
            
            std::cout << "parent " << parent_id << " has child " << child_id
                << " with data " << csr.data[i] << std::endl;

            std::cout << "====== Comparing with other edges ======"
                << std::endl;

            // Retrieve all Edge that are perpendicular to the following vector: 
            // parent_node.directionTo(child_node_a)
            std::vector<Edge> perpendicular_edges = GetPerpendicularEdges(parent_node, edges, child_node_a);

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
                weight = csr.data[i];
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
            }

            // Create the IntEdge using child_id
            // and the cross slope value stored in weight --
            // then add it to our result container.
            IntEdge ie = { child_id, weight };
            result.push_back(ie);
        }

        std::cout << "***** ALL CHILDREN FOR THIS PARENT DONE *****\n"
            << std::endl;
    }

    return result;
}

std::vector<IntEdge> HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope(Graph& g) {
    // All cross slope data will be stored here and returned from this function.
    std::vector<IntEdge> result;

    // Retrieve all parent nodes from g at once.
    // This makes for an easy iteration of all nodes within g.
    std::vector<Node> parents = g.Nodes();

    for (Node parent_node : parents) {
        // We iterate through all parent nodes,
        // and retrieve the edges that extend from the current parent_node.
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
            // This container will have all edges that are perpendicular to edge_a --
            // or rather, the vector formed by parent_node and child_node_a.
            std::vector<Edge> perpendicular_edges = GetPerpendicularEdges(parent_node, edges, child_node_a);

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
