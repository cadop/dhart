/// \file graph.h \brief Header file for a graph ADT
///
/// \author TBA \date 06 Jun 2020
#pragma once

#include <robin_hood.h>
#include <vector>
#include <Edge.h>
#include <Node.h>
#include <Eigen>

// TODO: Forward Declares? Later.
namespace Eigen {
}

/// <summary> Output CSR for interop with other languages </summary>
namespace HF {
	namespace SpatialStructures {

		/*! How to aggregate the graph's edge costs.

		\see 
		
		
		*/
		enum class COST_AGGREGATE : int {
			SUM = 0,
			AVERAGE = 1,
			COUNT = 2
		};

		/// <summary> TODO summary </summary>
		struct CSRPtrs {
			int nnz;	///< TODO description
			int rows;	///< row count
			int cols;	///< column count

			float* data;			///< TODO description
			int* outer_indices;		///< TODO description
			int* inner_indices;		///< TODO description

			/// <summary>
			/// Determines if member pointers data, outer_indices, and inner_indices are non-null,
			/// or not
			/// </summary>
			/// <returns>
			/// True if data, outer_indices, and inner_indices are non-null, false otherwise
			/// </returns>

			/*!
				\code
					// be sure to #include "graph.h", and #include <memory>
			
					std::unique_ptr<float[]> data(new float[16]);
					std::unique_ptr<int[]> outer_indices(new int[16]);
					std::unique_ptr<int[]> inner_indices(new int[16]);

					float* p_data = data.get();
					int* p_outer_indices = outer_indices.get();
					int* p_inner_indices = inner_indices.get();

					HF::SpatialStructures::CSRPtrs csr = { 16, 16, 16, p_data, p_outer_indices, p_inner_indices };

					bool validity = csr.AreValid();	// validity == true, since all pointer fields are non-null
				\endcode
			*/
			inline bool AreValid() {
				return (data && outer_indices && inner_indices);
			}
		};

		/// <summary>
		/// A HF Graph similar to the one stored in python Invariants:
		/// 1) Always stores a valid graph
		/// 2) Every node added will be assigned a unique ID
		/// </summary>
		class Graph {
		private:
			// TODO: Should there be a reverse map?
			std::vector<Node> ordered_nodes;				///< A ordered list of nodes with ids
			std::vector<int> id_to_nodes;					///< a list that indexes ordered_nodes with ids
			robin_hood::unordered_map<Node, int> idmap;		///< Maps a list of X,Y,Z positions to ids
			Eigen::SparseMatrix<float, 1> edge_matrix;		///< Contains all edges between nodes
			int next_id = 0;								///< The ID of the next unique node
			std::vector<Eigen::Triplet<float>> triplets;	///< vector of Triplet, type float
			bool needs_compression = true;					///< The CSR is inaccurate and requires compression

			/// <summary>
			/// Get the unique ID for this x, y, z position. If one does not exist yet for this node
			/// then assign it one.
			/// </summary>

			/*!
				\code
					// definition of Graph::addEdge(const Node& parent, const Node& child, float score)
				\endcode
			*/
			/// \snippet spatialstructures\src\graph.cpp GetOrAssignID_Node
			int getOrAssignID(const Node& input_node);

			/// <summary>
			/// Get the unique ID for this x, y, z position. If one does not exist yet for this node
			/// then assign it one.
			/// </summary>

			/*!
				\code
					// definition of Graph::addEdge(int parent_id, int child_id, float score)
				\endcode
			*/
			/// \snippet spatialstructures\src\graph.cpp GetOrAssignID_int
			int getOrAssignID(int input_int);

			/// <summary>
			/// Iterate through every edge in parent to find child. Returns when child is found
			/// </summary>

			/*!
				\code
					// definition of Graph::checkForEdge(int parent, int child) const
				\endcode
			*/
			/// \snippet spatialstructures\src\graph.cpp CheckForEdge
			bool checkForEdge(int parent, int child) const;

			/// <summary> Add an empty node for this new id </summary>

			/*!
				\code
					// No implementation for this private function
				\endcode
			*/
			void UpdateIDs(int new_id);

		public:
			// TODO: could this be cleaned up through a template?

			/// <summary>
			/// Construct a graph from a list of nodes and edges. This is faster than just using the
			/// addEdge method
			/// </summary>
			/// <param name="edges">
			/// The desired edges for Graph, in the form of an adjacency matrix (reference)
			/// </param>
			/// <param name="distances">
			/// The desired distances for Graph, following the form of param edges (reference)
			/// </param>
			/// <param name="Nodes"> A vector of Node for Graph (reference) </param>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
				\endcode
			*/
			Graph(
				const std::vector<std::vector<int>> & edges, 
				const std::vector<std::vector<float>> & distances, 
				const std::vector<Node> & Nodes
			);

			/*!
				\code
					// be sure to #include "graph.h"
			
					HF::SpatialStructures::Graph graph;		// This represents an order-zero graph (null graph)
															// It lacks vertices and edges.
				\endcode
			*/
			Graph() {};

			/// <summary> Determine if the graph has an edge with this parent and child, by
			/// constructing temporary Node (using the params parent and child) and calling the
			/// member function overload of HasEdge that accepts (const Node &). </summary>
			/// <returns>See return for member function overload that accepts (const Node &)</returns>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);									  
			
					// Prepare {x, y, z} coordinates (positions)
					auto parent_pos = node_1.getArray();		// (2.0, 3.0, 4.0)
					auto child_pos = node_2.getArray();			// (11.0, 22.0, 140.0)
			
					// last argument can be true/false for undirected/directed graph respectively
					bool has_edge = graph.HasEdge(parent_pos, child_pos, true);
				\endcode
			*/
			bool HasEdge(const std::array<float, 3>& parent, const std::array<float, 3>& child, bool undirected = false) const;

			/// <summary> Determine if the graph has an edge with this parent and child, using
			/// references to Node </summary> <param name="parent">Parent node of edge; u to v,
			/// parent is u</param> <param name="child">Child node of edge; u to v, child is
			/// v</param> <param name="undirected">true if undirected, false otherwise</param>
			/// <returns>True if edge exists with parent and child (Node &), false otherwise</returns>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// last argument can be true/false for undirected/directed graph respectively
					bool has_edge = graph.HasEdge(node_1, node_2, true);
				\endcode
			*/
			bool HasEdge(const Node& parent, const Node& child, const bool undirected = false) const;
			
			/// <summary> Determine if the graph has an edge with this parent and child, using the
			/// integer identifiers for nodes </summary> <param name="parent">Identifier for parent
			/// node (integer)</param> <param name="child">Identifier for child node
			/// (integer)</param> <param name="undirected">True if undirected graph, false
			/// otherwise. False by default.</param> <returns>True if checkForEdge(parent, child),
			/// or undirected && checkForEdge(child, parent), false otherwise</returns>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// last argument can be true/false for undirected/directed graph respectively
					bool has_edge = graph.HasEdge(0, 1, true);
				\endcode
			*/
			bool HasEdge(int parent, int child, bool undirected = false) const;

			/// <summary> Get a list of nodes from the graph sorted by ID </summary>
			/// <returns> A sorted vector of nodes </returns>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);	
			
					// Nodes() returns a copy of the ordered_nodes field
					std::vector<HF::SpatialStructures::Node> nodes_from_graph = graph.Nodes();
				\endcode
			*/
			std::vector<Node> Nodes() const;

			/// <summary> Get a list of edges to and from node N </summary>
			/// <param name="N"> The Node from which the list of edges will be derived </param>
			/// <returns> A list of edges to and from node N </returns>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);	
			
					// Retrieve the nodes from the graph, or use the original instance of
					// std::vector<HF::SpatialStructures::Node> passed to Graph upon instantiation
					std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
			
					// nodes[index] yields an instance of Node that we can pass to
					// GetUndirectedEdges. Any node that exists with graph can be passed to this
					// member function to retrieve a vector of undirected edges.
					int index = 2;
					std::vector<HF::SpatialStructures::Edge> undirected_edges = graph.GetUndirectedEdges(get_nodes[index]);
				\endcode
			*/
			std::vector<Edge> GetUndirectedEdges(const Node & N) const;
		
			/// <summary> Get every edge/node in the given graph as IDs </summary>
			/// <returns> A list of EdgeSet (Graph in the form of IDs) </returns>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);	
			
					// graph must be compressed, or a exception will be thrown
			
					// To brief, an EdgeSet has the following layout: struct EdgeSet { int parent;
					// std::vector<IntEdge> children; };
					//
					// An IntEdge has the following layout: struct IntEdge { int child; float
					// weight; };
			
					// A std::vector<EdgeSet> is a Graph, in the form of IDs.
					std::vector<HF::SpatialStructures::EdgeSet> edge_set = graph.GetEdges();
				\endcode
			*/
			std::vector<EdgeSet> GetEdges() const;

			/// <summary> Generate an ordered list of scores for the graph </summary>
			/// <param name="agg_type"> 0 for SUM, 1 for AVERAGE, 2 for COUNT </param>
			/// <param name="directed"> if graph is directed or not, default is true </param>
			/// <returns> a vector of type float of the edge costs </returns>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// graph must be compressed, or a exception will be thrown
					HF::SpatialStructures::COST_AGGREGATE aggregate = HF::SpatialStructures::COST_AGGREGATE::AVERAGE;		// aggregate == 1 in this case
			 
					// directed parameter may be true or false
					std::vector<float> aggregate_graph = graph.AggregateGraph(aggregate, true);	
				\endcode
			*/
			std::vector<float> AggregateGraph(COST_AGGREGATE agg_type, bool directed=true) const;

			/// <summary> Get a list of edges for the given node </summary>
			/// <param name="n"> Node from which a list of edges will be derived </param>
			/// <returns> A copy of the graph's list </returns>
			/// <exception cref="std::outofrange">
			/// Thrown if the requested object is not in the dictionary
			/// </exception>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// Retrieve the nodes from the graph, or use the original instance of
					// std::vector<Node> passed to Graph upon instantiation
					std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
			
					// nodes[index] yields an instance of Node that we can pass to
					// GetUndirectedEdges. Any node that exists with graph can be passed to this
					// member function to retrieve a vector of edges.
					int index = 2;
					HF::SpatialStructures::Node node = get_nodes[index];
			
					// Note that if node does not exist within graph, that an exception will be thrown.
					std::vector<HF::SpatialStructures::Edge> undirected_edges = graph[node];
			
					// See a (node)->(child_node_0, child_node_1, ... child_node_n)
					std::cout << node.getArray() << "->";
					for (auto e : undirected_edges) {
						std::cout << e.child.getArray() << ", ";
					}
					std::cout << std::endl;
				\endcode
			*/
			const std::vector<Edge> operator[](const Node& n) const;

			/// <summary>
			/// Construct a new edge for the graph from parent and child pair. If the parent is not
			/// in the graph as a key then it will be added
			/// </summary>
			/// <param name="parent"> The parent node to add to </param>
			/// <param name="child"> The child node to add as an edge </param>
			/// <param name="score"> The score for the given edge </param>

			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// Create a pair of nodes
					HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
					HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);
			
					graph.addEdge(n_parent, n_child);	// default score is 1.0f
				\endcode
			*/
			void addEdge(const Node& parent, const Node& child, float score = 1.0f);

			/// <summary>
			/// Construct a new edge for the graph using parent and child IDs. If the parent_id is
			/// not in the graph as a key, it will be added
			/// </summary>
			/// <param name="parent_id"> The ID of the parent node for the edge to construct </param>
			/// <param name="child_id"> The ID of the child node for the edge to construct </param>
			/// <param name="score"> The score for the given edge </param>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					int parent = 1;
					int child = 2;
			
					graph.addEdge(parent, child, 1.0f);
					graph.Compress();
				\endcode
			*/
			void addEdge(int parent_id, int child_id, float score);

			/// <summary> Tell whether or not the graph has the given node in it as a parent </summary>
			/// <param name="n"> Node to check for </param>
			/// <returns> True if the node exists, false otherwise </returns>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
					bool has_key = graph.hasKey(other_node);	// other_node does not exist in graph, so has_key == false;
			
					// Likewise, if we pass a Node instance that indeed exists...
			
					// Retrieve the nodes from the graph, or use the original instance of
					// std::vector<Node> passed to Graph upon instantiation
					std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
			
					// nodes[index] yields an instance of Node that we can pass to hasKey. Any node
					// that exists with graph can be passed to this member function to determine if
					// the graph has the node's key, or not.
					int index = 2;
					HF::SpatialStructures::Node good_node = get_nodes[index];
			
					has_key = graph.hasKey(good_node);		// now has_key is true
				\endcode
			*/
			bool hasKey(const Node& n) const;

			/// <summary> Get a list of nodes as arrays of floats </summary>
			/// <returns> A list of floats for each position of every node in the graph </returns>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// A container of std::array<float, 3> is constructed and populated within
					// NodesAsFloat3, and returned. Each array of 3 floats represents a Node's
					// position within the Cartesian coordinate system. { x, y, z }
					std::vector<std::array<float, 3>> nodes_as_floats = graph.NodesAsFloat3();
			
					// The two loops below will yield the same output
					for (auto n : graph.Nodes()) {
						std::cout << "(" << n.x << "," << n.y << "," << n.z << ")" << std::endl;
					}
			
					for (auto a : nodes_as_floats) {
						std::cout << a << std::endl;
					}
				\endcode
			*/
			std::vector<std::array<float, 3>> NodesAsFloat3() const;

			/// <summary> Determine how many nodes are in the graph </summary>
			/// <returns> An int displaying how many nodes are in the graph (id_to_nodes.size()) </returns>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					int id_count = graph.size();		  // We retrieve the size of the node id count within graph (3)
				\endcode
			*/
			int size() const;

			/// <summary> Retrieve the ID for the given node. </summary>
			/// <returns> The ID assigned to this node. -1 if it was not yet added to the graph </returns>
			
			/*!
				\code
					// be sure to #include "graph.h"
			
					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
			
					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
			
					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
			
					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
					bool has_key = graph.hasKey(other_node);							// other_node does not exist in graph, so has_key == false;
			
					int ID = graph.getID(other_node);									// ID will assigned -1, because other_node is not a part of graph.
			
					// Likewise, if we pass a Node instance that indeed exists...
			
					// Retrieve the nodes from the graph, or use the original instance of
					// std::vector<Node> passed to Graph upon instantiation
					std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
			
					// nodes[index] yields an instance of Node that we can pass to hasKey. Any node
					// that exists with graph can be passed to this member function to determine if
					// the graph has the node's key, or not.
					int index = 2;					// we assume for this example that index 2 is valid.
					HF::SpatialStructures::Node good_node = get_nodes[index];
			
					ID = graph.getID(good_node);	// ID > -1, i.e. it is a Node instance that exists within this Graph.
				\endcode
			*/
			int getID(const Node& node) const;

			/// <summary>
			/// Compress the graph, significantly reducing memory usage, but disabling insertion.
			/// </summary>
			
			/*!
				\code
					// be sure to #include "graph.h"

					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// Create a pair of nodes
					HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
					HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

					graph.addEdge(n_parent, n_child);	// default score is 1.0f
			
					// In order to use GetEdges, or AggregateGraph, we must compress our graph instance
					graph.Compress();						// GetEdges and AggregateGraph are now usable
				\endcode
			*/			
			void Compress();

			/// <summary>
			/// Obtain pointers to the 3 arrays of the CSR, as well as any extra info that may be
			/// necesary to use the CSR in another language. This will automatically compress the
			/// graph if it isn't compressed already
			/// </summary>
			/// <returns> An instance of a struct CSRPtrs, using the member field edge_matrix </returns>
			
			/*!
				\code
					// be sure to #include "graph.h"

					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);

					// Create a pair of nodes
					HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
					HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

					graph.addEdge(n_parent, n_child);	// default score is 1.0f

					// Graph will be compressed automatically be GetCSRPointers
					CSRPtrs returned_csr = graph.GetCSRPointers();
				\endcode
			*/
			CSRPtrs GetCSRPointers();

			/// <summary> Retrieve the node that corresponds to id </summary>
			/// <param name="id"> The id for the desired node </param>
			/// <returns> The node corresponding to id, by value </returns>
			
			/*!
				\code
					// be sure to #include "graph.h"

					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// Let's retrieve node_1.
					int desired_node_id = 2;
					HF::SpatialStructures::Node node_from_id = graph.NodeFromID(desired_node_id);
			
					// Note that NodeFromID ceases to work if the id argument provided does not
					// exist as an ID among the nodes within graph
				\endcode
			*/
			Node NodeFromID(int id) const;

			/// <summary> Clear all nodes and edges in the graph. </summary>
			
			/*!
				\code
					// be sure to #include "graph.h"

					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					// Note: graph is compressed upon instantiation
					HF::SpatialStructures::Graph graph(edges, distances, nodes);
			
					// If we want to remove all nodes and edges from graph, we may do so with Clear:
					graph.Clear();						// edge_matrix is zeroed out, buffer is squeezed,
														// triplets are also cleared, and
														// needs_compression == true
				\endcode
			*/
			void Clear();

			// TODO: Should these even be in the graph?

			/// <summary> Calculate cross slope for the given graph </summary>
			
			/*!
				\code
					// TODO example - code commented out in graph.cpp
				\endcode
			*/
			void GenerateCrossSlope();

			/// <summary> Calculate energy for the given graph </summary>
			
			/*!
				\code
					// TODO example - code commented out in graph.cpp
				\endcode
			*/
			void GenerateEnergy();
		};
	}
}
