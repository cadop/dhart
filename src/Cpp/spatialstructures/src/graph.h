/// \file graph.h \brief Header file for a graph ADT
///
/// \author TBA \date 06 Jun 2020
/// \todo Forward declares for eigen.  

#pragma once

#include <robin_hood.h>
#include <vector>
#include <Edge.h>
#include <Node.h>
#include <Eigen>

namespace Eigen {
}

/// <summary> Output CSR for interop with other languages </summary>
namespace HF {
	namespace SpatialStructures {


		/*! Ways to aggregate the costs for edges of each node. 
		
		\see Aggregate Graph for details on how to use this struct.
		*/
		enum class COST_AGGREGATE : int {
			// Add the cost of all edges.
			SUM = 0,
			// Average the cost of all edges
			AVERAGE = 1,
			// Count how many edges this node has. 
			COUNT = 2
		};

		/*! A struct to hold all necessary information for a CSR.

			\details 
			This can be used by external clients to recreate or map to 
			the csr. For more information see 
			https://eigen.tuxfamily.org/dox/group__TutorialSparse.html
		*/
		struct CSRPtrs {
			int nnz;	///< Number of non-zeros
			int rows;	///< Number of rows in this CSR.
			int cols;	///< Number of columns in this CSR.

			float* data;			///< Pointer to the CSR's data array.
			int* outer_indices;		///< Pointer to the CSR's outer indices array. 
			int* inner_indices;		///< Pointer to the CSR's inner indices array.

			/// <summary>
			/// Verify this object points to a valid CSR.
			/// </summary>
			/// <returns>
			/// True if data, outer_indices, and inner_indices are non-null, false otherwise,
			/// </returns>
			/*!
				\remarks
				This is implemented this way because uninitialized or invalid Eigen arrays will return
				null if they are in an invalid state (uninitialized, uncompressed, etc). 

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

		/*! \brief A Graph of nodes connected by edges that supports both integers and HF::SpatialStructures::Node.
			
			\details
			Internally, this object uses Eigen (https://eigen.tuxfamily.org/dox/group__TutorialSparse.html)
			to store and maintain a CSR matrix.

			\invariant
				Always stores a valid graph and always assignes a unique ID to every node.
				
		*/
		class Graph {
		private:
			std::vector<Node> ordered_nodes;				///< A list of nodes contained by the graph
			std::vector<int> id_to_nodes;					///< a list that indexes ordered_nodes with ids
			robin_hood::unordered_map<Node, int> idmap;		///< Maps a list of X,Y,Z positions to ids
			Eigen::SparseMatrix<float, 1> edge_matrix;		///< The CSR matrix
			int next_id = 0;								///< The id for the next unique node
			std::vector<Eigen::Triplet<float>> triplets;	///< A list of edges for construction by a list of triplets.
			bool needs_compression = true;					///< The CSR is inaccurate and requires compression

			/// <summary>
			/// Get the unique ID for this x, y, z position or assign it a unique ID if one doesn't already exist
			/// </summary>
			/*!
				\details 
				If the node has not yet been seen by the graph, next_id will be assigned to it and incremented,
				then the node node will and its new id will be added to idmap. If the node has already been 
				assigned an ID, then the ID will be returned directly from idmap. 

				\param input_node Node to retrieve and potentially assign a new ID for. 
				\returns The ID of input_node. 
				\code
					// definition of Graph::addEdge(const Node& parent, const Node& child, float score)
				\endcode
			*/
			/// \snippet spatialstructures\src\graph.cpp GetOrAssignID_Node
			int getOrAssignID(const Node& input_node);

			/*!
				\brief Add an ID to the graph if it doesn't exist already.

				\details 
				This onyl exists 

				\code
					// definition of Graph::addEdge(int parent_id, int child_id, float score)
				\endcode
			*/
			/// \snippet spatialstructures\src\graph.cpp GetOrAssignID_int
			int getOrAssignID(int input_int);


			/*!
				\brief Determine if an edge between parent and child exists in the graph.
				
				\details 
				Iterates through every row in the parent's column to find child. If child
				is not found in this column, false is returned. If child can be found in this
				column, then true is returned.

				\par Time Complexity
				O(k) where k is the number of edges from parent.
				
			*/
			/// \snippet spatialstructures\src\graph.cpp CheckForEdge
			bool checkForEdge(int parent, int child) const;

			/// <summary> Add an empty node for this new id </summary>
			/*! \deprecated Never Implemented.*/
			void UpdateIDs(int new_id);

		public:
			/*!
			 \brief Construct a graph from a list of nodes, edges, and distances. 

			 \param edges Ordered array of arrays of edges for each node in nodes.
			 \param distances Ordered array of distance from parent to child for each edge in edges.
			 \param Nodes Ordered array of nodes to act as a parent to all children in it's array in edges.

			 \pre 1) The size of all input arrays must match: 
			 `(edges.size() == nodes.size() && nodes.size() == distances.size())`
			
			\pre 2) For the node at `nodes[i]`, `edges[i]` should contain an array for the id of all nodes
			that `nodes[i]` has an edge from and, and `distances[i]` should contain an array of the the distance
			from `nodes[i]` to one of the nodes it has an edge to in `edges[i]`.

			 \remarks
			 This constructor can offer "slightly higher performance and memory consumption"
			 than constructing a graph using Graph::addEdge in a loop according to official eigen documentation,
			 however it may not be feasible for certain situations where the entire graph isn't
			 known before the constructor is called. 
			 The implementation is based on the algorithm from Eigen's documentation under
			 the section Filling a Sparse Matrix 
			 https://eigen.tuxfamily.org/dox/group__TutorialSparse.html.

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
		
				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);
			\endcode
			*/
			Graph(
				const std::vector<std::vector<int>> & edges, 
				const std::vector<std::vector<float>> & distances, 
				const std::vector<Node> & Nodes
			);

			/*! \brief Construct an empty graph.

				\remarks This can be used to create a new graph to later be filled with edges/nodes
				by calling Graph::addEdge() then calling Graph::MakeCompressed. Implementatin
				is based on the Eigen documentation for Filling a CSR:
				https://eigen.tuxfamily.org/dox/group__TutorialSparse.html.


				\code
					// be sure to #include "graph.h"
					HF::SpatialStructures::Graph graph;		// This represents an order-zero graph (null graph)
															// It lacks vertices and edges.
				\endcode
			*/
			Graph() {};

			/*! \brief Determine if the graph has an edge from parent to child.
			
				\param parent Parent of the edge to check for.
				\param child Child of the edge to check for.
				\param undirected If true, look for an edge from child to parent as well.
				\returns True if an edge between parent and child exists 
				(also child and parent if undirected is true).

				\remarks
				Converts parent and child to Node then calls the node overload.
				
				\exception std::exception if the matrix is uncompressed.
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
			bool HasEdge(
				const std::array<float, 3>& parent,
				const std::array<float, 3>& child,
				bool undirected = false
			) const;

			/* \brief Determine if the graph has an edge from parent to child.
			
				\param parent Parent of the edge to check for.
				\param child Child of the edge to check for.
				\param undirected If true, look for an edge from child to parent as well.
				\returns True if an edge between parentand child exists
				(also child and parent if undirected is true).

				\remarks
				Gets the IDs of both nodes, then calls the integer overload.
				
				\exception std::exception if the graph is uncompressed.
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
			/* \brief Determine if the graph has an edge from parent to child.

				\param parent Parent of the edge to check for.
				\param child Child of the edge to check for.
				\param undirected If true, look for an edge from child to parent as well.
				\returns True if an edge between parentand child exists
				(also child and parent if undirected is true).

				\exception std::exception if the graph is uncompressed.
				
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

			/// <summary> Get a list of nodes from the graph sorted by ID. </summary>
			/// <returns> A sorted vector of nodes. </returns>

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

			/// <summary> Get a list of all edges to and from node N. </summary>
			/// <param name="N"> The Node to get edges from and to. </param>
			/// <returns> A list of edges to and from node N or an empty array if is not 
			/// in the graph. </returns>
			/*!

				\par Time Complexity
				`O(k)` where k is the number of edges in the graph since it needs to iterate through 
				every edge in the graph to find the edges to this node.

				\see operator[] to get a list of directed edges only containing edges from N.
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
		
			/// <summary> Get every in the given graph as IDs. </summary>
			/// <returns> An array of edgesets for every node in the graph
			/// (Graph in the form of IDs). </returns>
			/*!
				
				\exception std::exception if the graph isn't compressed.
				
				\par Time Complexity
				O(k) where k is the number of edges in the graph.

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

			/// <summary> Summarize the costs of every outgoing edge for every
			/// node in the graph. </summary>
			/// <param name="agg_type"> Type of aggregation to use. </param>
			/// <param name="directed"> If true, include both incoming and
			/// outgoing edges for every node in nodes </param>
			/// <returns> An ordered list of scores for agg_type on 
			/// each node in the graph. </returns>
			/*!
				
				\remarks Can be useful to get scores from the VisibilityGraph.

				\exception std::out_of_range if agg_type doesn't match any value of COST_AGGREGATE.
				\see COST_AGGREGATE to see a list of supported aggregation types.
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
