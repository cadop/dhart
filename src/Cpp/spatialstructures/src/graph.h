#pragma once
#include <robin_hood.h>
#include <vector>
#include <Edge.h>
#include <Node.h>
#include <Eigen>

//TODO: Forward Declares? Later.
namespace Eigen {
}

/// <summary>
/// Output CSR for interop with other languages
/// </summary>
namespace HF {
	namespace SpatialStructures {

		/// <summary>
		/// TODO summary
		/// </summary>
		enum class COST_AGGREGATE : int {
			SUM = 0,
			AVERAGE = 1,
			COUNT = 2
		};

		/// <summary>
		/// TODO summary
		/// </summary>
		struct CSRPtrs {
			int nnz;	///< TODO description ///<
			int rows;	///< row count ///<
			int cols;	///< column count ///<

			float* data;			///< TODO description ///<
			int* outer_indices;		///< TODO description ///<
			int* inner_indices;		///< TODO description ///<

			/// <summary>
			/// Determines if member pointers data, outer_indices, and inner_indices are non-null, or not
			/// </summary>
			/// <returns>True if data, outer_indices, and inner_indices are non-null, false otherwise</returns>
			inline bool AreValid() {
				return (data && outer_indices && inner_indices);
			}
		};

		/// <summary>
		/// A HF Graph similar to the one stored in python
		/// Invariants:
		/// 1) Always stores a valid graph
		/// 2) Every node added will be assigned a unique ID
		/// </summary>
		class Graph {
		private:
			//TODO: Should there be a reverse map?
			std::vector<Node> ordered_nodes;				///< A ordered list of nodes with ids ///<
			std::vector<int> id_to_nodes;					///< a list that indexes ordered_nodes with ids ///<
			robin_hood::unordered_map<Node, int> idmap;		///< Maps a list of X,Y,Z positions to ids ///<
			Eigen::SparseMatrix<float, 1> edge_matrix;		///< Contains all edges between nodes ///<
			int next_id = 0;								///< The ID of the next unique node ///<
			std::vector<Eigen::Triplet<float>> triplets;	///< vector of Triplet, type float ///<
			bool needs_compression = true;					///< The CSR is inaccurate and requires compression ///<

			/// <summary>
			/// Get the unique ID for this x,y,z position. If one does not exist yet for this node
			/// then assign it one.
			/// </summary>
			int getOrAssignID(const Node& input_node);

			/// <summary>
			/// Get the unique ID for this x,y,z position. If one does not exist yet for this node
			/// then assign it one.
			/// </summary>
			int getOrAssignID(int input_int);


			/// <summary>
			/// Iterate through every edge in parent to find child. Returns when child is found	
			/// </summary>
			bool checkForEdge (int parent, int child) const;

			/// <summary>
			/// Add an empty node for this new id
			/// </summary>
			void UpdateIDs(int new_id);

		public:
			// TODO: could this be cleaned up through a template?

			/// <summary>
			/// Construct a graph from a list of nodes and edges. This is faster than just using the addEdge method
			/// </summary> 
			/// <param name="edges">The desired edges for Graph, in the form of an adjacency matrix (reference)</param>
			/// <param name="distances">The desired distances for Graph, following the form of param edges (reference)</param>
			/// <param name="Nodes">A vector of Node for Graph (reference)</param>
			Graph(
				const std::vector<std::vector<int>> & edges, 
				const std::vector<std::vector<float>> & distances, 
				const std::vector<Node> & Nodes
			);

			Graph() {};

			/// <summary>
			/// Determine if the graph has an edge with this parent and child, 
			/// by constructing temporary Node (using the params parent and child) 
			/// and calling the member function overload of HasEdge that accepts (const Node &).
			/// </summary>
			/// <returns>See return for member function overload that accepts (const Node &)</returns>
			bool HasEdge(const std::array<float, 3>& parent, const std::array<float, 3>& child, bool undirected = false) const;

			/// <summary>
			/// Determine if the graph has an edge with this parent and child, using references to Node
			/// </summary>
			/// <param name="parent"></param>
			/// <param name="child"></param>
			/// <param name="undirected></param>
			/// <returns>True if edge exists with parent and child (Node &), false otherwise</returns>
			bool HasEdge(const Node& parent, const Node& child, const bool undirected = false) const;
			
			/// <summary>
			/// Determine if the graph has an edge with this parent and child, using the integer identifiers for nodes
			/// </summary>
			/// <param name="parent">Identifier for parent node (integer)</param>
			/// <param name="child">Identifier for child node (integer)</param>
			/// <param name="undirected">True if undirected graph, false otherwise. False by default.</param>
			/// <returns>True if checkForEdge(parent, child), or undirected && checkForEdge(child, parent), false otherwise</returns>
			bool HasEdge(int parent, int child, bool undirected = false) const;

			/// <summary>
			/// Get a list of nodes from the graph sorted by ID
			/// </summary>
			/// <returns>A sorted vector of nodes</returns>
			std::vector<Node> Nodes() const;

			/// <summary>
			/// Get a list of edges to and from node N
			/// </summary>
			/// <param name="N">The Node from which the list of edges will be derived</param>
			/// <returns>A list of edges to and from node N</returns>
			std::vector<Edge> GetUndirectedEdges(const Node & N) const;
		
			/// <summary>
			/// Get every edge/node in the given graph as IDs
			/// </summary>
			/// <returns>A list of EdgeSet (Graph in the form of IDs)</returns>
			std::vector<EdgeSet> GetEdges() const;

			/// <summary>
			/// Generate an ordered list of scores for the graph
			/// </summary>
			/// <param name="agg_type">0 for SUM, 1 for AVERAGE, 2 for COUNT</param>
			/// <param name="directed">if graph is directed or not, default is true</param>
			/// <returns>a vector of type float of the edge costs</returns>
			std::vector<float> AggregateGraph(COST_AGGREGATE agg_type, bool directed=true) const;

			/// <summary>
			/// Get a list of edges for the given node
			/// </summary>
			/// <param name="n">Node from which a list of edges will be derived</param>
			/// <returns>A copy of the graph's list</returns>
			/// <exception cref="std::outofrange"> Thrown if the requested object is not in the dictionary</exception>
			const std::vector<Edge> operator[](const Node& n) const;

			/// <summary>
			/// Construct a new edge for the graph from parent and child pair. If the parent is not in the graph
			/// as a key then it will be added
			/// </summary>
			/// <param name="parent">The parent node to add to</param>
			/// <param name="child">The child node to add as an edge</param>
			/// <param name="score">The score for the given edge</param>
			void addEdge(const Node& parent, const Node& child, float score = 1.0f);

			/// <summary>
			/// Construct a new edge for the graph using parent and child IDs.
			/// If the parent_id is not in the graph as a key, it will be added
			/// </summary>
			/// <param name="parent_id">The ID of the parent node for the edge to construct</param>
			/// <param name="child_id">The ID of the child node for the edge to construct</param>
			/// <param name="score">The score for the given edge</param>
			void addEdge(int parent_id, int child_id, float score);

			/// <summary>
			/// Tell whether or not the graph has the given node in it as a parent
			/// </summary>
			/// <param name="n">Node to check for</param>
			/// <returns>True if the node exists, false otherwise</returns>
			bool hasKey(const Node& n) const;

			/// <summary>
			/// Get a list of nodes as arrays of floats
			/// </summary>
			/// <returns>A list of floats for each position of every node in the graph </returns>
			std::vector<std::array<float, 3>> NodesAsFloat3() const;

			/// <summary>
			/// Determine how many nodes are in the graph
			/// </summary>
			/// <returns>An int displaying how many nodes are in the graph (id_to_nodes.size())</returns>
			int size() const;

			/// <summary>
			/// Retrieve the ID for the given node.
			/// </summary>
			/// <returns>The ID assigned to this node. -1 if it was not yet added to the graph</returns>
			int getID(const Node& node) const;

			/// <summary>
			/// Compress the graph, significantly reducing memory usage, but disabling insertion.
			/// </summary>
			void Compress();

			/// <summary>
			/// Obtain pointers to the 3 arrays of the CSR, as well as any extra info that may be necesary to use
			/// the CSR in another language. This will automatically compress the graph if it isn't compressed already
			/// </summary>
			/// <returns>An instance of a struct CSRPtrs, using the member field edge_matrix</returns>
			CSRPtrs GetCSRPointers();

			/// <summary>
			/// Retrieve the node that corresponds to id
			/// </summary>
			/// <param name="id">The id for the desired node</param>
			/// <returns>The node corresponding to id, by value</returns>
			Node NodeFromID(int id) const;

			/// <summary>
			/// Clear all nodes and edges in the graph.
			/// </summary>
			void Clear();

			// TODO: Should these even be in the graph?

			/// <summary>
			/// Calculate cross slope for the given graph
			/// </summary>
			void GenerateCrossSlope();

			/// <summary>
			/// Calculate energy for the given graph
			/// </summary>
			void GenerateEnergy();
		};
	}
}
