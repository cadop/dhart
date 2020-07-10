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
using HF::SpatialStructures::EdgeSet;

bool HF::SpatialStructures::CostAlgorithms::is_floating_type(std::string value) {
    char* ptr = nullptr;
    std::strtof(value.c_str(), &ptr);
    return *ptr == '\0';
}

double HF::SpatialStructures::CostAlgorithms::to_radians(double degrees) {
    return degrees * (M_PI / 180);
}

double HF::SpatialStructures::CostAlgorithms::to_degrees(double radians) {
    return radians * (180 / M_PI);
}

std::vector<EdgeSet> HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure(Graph& g) {
    // Energy expenditure data will be stored here and returned from this function.
    std::vector<EdgeSet> edge_set;

    CSRPtrs csr = g.GetCSRPointers();

    const int last_index = csr.rows - 1;

    for (int parent_id = 0; parent_id < csr.rows; parent_id++) {
        bool at_last_parent = parent_id == last_index;

        int curr_row_index = csr.outer_indices[parent_id];
        int next_row_index = csr.outer_indices[parent_id + 1];

        const int row_end_index = at_last_parent ? csr.nnz : next_row_index;

        Node parent_node = g.NodeFromID(parent_id);

        std::vector<Edge> edge_list = g[parent_node];
        std::vector<IntEdge> children;

        for (Edge link_a : edge_list) {
            auto dir = parent_node.directionTo(link_a.child);
            auto dz = link_a.child.z - parent_node.z;
            auto dy = link_a.child.y - parent_node.y;
            auto dx = link_a.child.x - parent_node.x;

            // angle formed by parent_node and link_a.child
            auto angle = std::atan(std::sqrtf(std::pow(dz / dx, 2) + std::pow(dz / dy, 2)));

            auto slope = std::clamp(std::tanf(angle), -0.4f, -0.4f);

            auto e = 280.5 
                * (std::pow(slope, 5)) - 58.7 
                * (std::pow(slope, 4)) - 76.8 
                * (std::pow(slope, 3)) + 51.9 
                * (std::pow(slope, 2)) + 19.6 
                * (slope) + 2.5;

            assert(e > 0);
            link_a.score = e * parent_node.distanceTo(link_a.child);
        }

        for (Edge e : edge_list) {
            IntEdge ie = { e.child.id, e.score };
            children.push_back(ie);
        }

        EdgeSet es = { parent_node.id, children };
        edge_set.push_back(es);
    }

    return edge_set;
}


float HF::SpatialStructures::CostAlgorithms::calculate_magnitude(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    return std::sqrtf(std::pow(dir_a[0] - dir_b[0], 2.0) + std::pow(dir_a[1] - dir_b[1], 2.0) + std::pow(dir_a[2] - dir_b[2], 2.0));
}

float HF::SpatialStructures::CostAlgorithms::calculate_dot_product(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    return (dir_a[0] * dir_b[0]) + (dir_a[1] * dir_b[1]) + (dir_a[2] * dir_b[2]);
}

bool HF::SpatialStructures::CostAlgorithms::is_perpendicular(std::array<float, 3> dir_a, std::array<float, 3> dir_b) {
    float dot_product = calculate_dot_product(dir_a, dir_b);

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

            /*
                Here -- we should assess the step_type field for edge_a.

                /// <summary> Describes the type of step an edge connects to. </summary>
                enum STEP {
                    NOT_CONNECTED = 0, ///< No connection between parent and child.
                    NONE = 1,		 ///< Parent and child are on the same plane and no step is required.
                    UP = 2,			///< A step up is required to get from parent to child.
                    DOWN = 3,		///< A step down is required to get from parent to child.
                    OVER = 4		///< A step over something is required to get from parent to child.
                };


            */

            // Retrieve all Edge that are perpendicular to the following vector: 
            // parent_node.directionTo(child_node_a)
            std::vector<Edge> perpendicular_edges = GetPerpendicularEdges(parent_node, edges, child_node_a);

            std::cout << "====== End of comparing with other edges ======"
                << std::endl;

            float weight = 0.0;
            float a_x = 0.0;
            float b_x = 0.0;
            float c_x = 0.0;
            float a_z = 0.0;
            float b_z = 0.0;
            float c_z = 0.0;

            float angle_radians = 0.0;

            /*
                The cant of a railway track or camber of a road 
                (also referred to as superelevation, cross slope or cross fall) 
                is the rate of change in elevation (height) between the two rails or edges.

                This is normally greater where the railway or road is curved; 
                raising the outer rail or the outer edge of the road providing a banked turn, 
                thus allowing vehicles to maneuver through the curve at higher speeds 
                than would otherwise be possible if the surface is flat or level.
            
                https://en.wikipedia.org/wiki/Cant_(road/rail)#Off-camber_corner
            */

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
                a_x = child_node_a.x;
                b_x = perpendicular_edges[0].child.x;

                a_z = child_node_a.z;
                b_z = perpendicular_edges[0].child.z;

                // add the existing weight value to the delta of the z values
                //weight = std::abs(a_z - b_z) + perpendicular_edges[0].score;

                
                weight = (a_z - b_z) / (a_x - b_x);
                angle_radians = std::atan(weight);
                

                /*
                    The vector formed by parent_node and child_node_a
                    was found to have one perpendicular edge,
                    formed by parent_node and perpendicular_edges[0].child.

                    We therefore want to find the angle of the following vectors:
                        - child_node_a to parent_node
                        - child_node_a to perpendicular_edges[0].child
                
                    This ratio will yield the cosine of the cross slope angle.
                */
                
                /*
                auto vector_a = child_node_a.directionTo(parent_node);
                auto vector_b = child_node_a.directionTo(perpendicular_edges[0].child);

                // angle formed by vector_a and vector_b
                float adjacent = calculate_dot_product(vector_a, vector_b);
                float hypotenuse = calculate_magnitude(vector_a) * calculate_magnitude(vector_b);

                weight = adjacent / hypotenuse;
                angle_radians = std::acos(weight);
                */
                break;
            case 2:
                // Two edges -- each formed by node parent_node and two separate
                // child node by node parent_node were found to be perpendicular to the edge
                // formed by node parent_node and node child_node_a.

                a_z = child_node_a.z;

                b_x = perpendicular_edges[0].child.x;
                c_x = perpendicular_edges[1].child.x;

                b_z = perpendicular_edges[0].child.z;
                c_z = perpendicular_edges[1].child.z;

                // add the existing weight value to the delta of the z values
                //weight = std::abs(b_z - c_z) + perpendicular_edges[0].score;
                
                /*
                    The vector formed by parent_node and child_node_a
                    was found to have two perpendicular edges,
                    each formed by parent_node and:
                        - perpendicular_edges[0].child
                        - perpendicular_edges[1].child

                    We therefore want to find the angle of the following vectors:
                        - perpendicular_edges[1] to perpendicular_edges[0]
                */

                weight = (b_z - c_z) / (b_x - c_x);
                angle_radians = std::atan(weight);
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

            /*
                Here -- we should assess the step_type field for edge_a.

                /// <summary> Describes the type of step an edge connects to. </summary>
                enum STEP {
                    NOT_CONNECTED = 0, ///< No connection between parent and child.
                    NONE = 1,		 ///< Parent and child are on the same plane and no step is required.
                    UP = 2,			///< A step up is required to get from parent to child.
                    DOWN = 3,		///< A step down is required to get from parent to child.
                    OVER = 4		///< A step over something is required to get from parent to child.
                };


            */

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


                auto vector_a = child_node_a.directionTo(parent_node);
                auto vector_b = child_node_a.directionTo(perpendicular_edges[0].child);

                // angle formed by vector_a and vector_b

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
            /*
                Here -- we should assess the step_type field for edge_b.

                /// <summary> Describes the type of step an edge connects to. </summary>
                enum STEP {
                    NOT_CONNECTED = 0, ///< No connection between parent and child.
                    NONE = 1,		 ///< Parent and child are on the same plane and no step is required.
                    UP = 2,			///< A step up is required to get from parent to child.
                    DOWN = 3,		///< A step down is required to get from parent to child.
                    OVER = 4		///< A step over something is required to get from parent to child.
                };

                
            */

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
