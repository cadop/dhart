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

		enum class COST_AGGREGATE : int {
			SUM = 0,
			AVERAGE = 1,
			COUNT = 2
		};

		struct CSRPtrs {
			int nnz;
			int rows;
			int cols;

			float* data;
			int* outer_indices;
			int* inner_indices;

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
			std::vector<Node> ordered_nodes;			// A ordered list of nodes with ids
			std::vector<int> id_to_nodes;				// a list that indexes ordered_nodes with ids
			robin_hood::unordered_map<Node, int> idmap; // Maps a list of X,Y,Z positions to ids
			Eigen::SparseMatrix<float, 1> edge_matrix;  // Contains all edges between nodes
			int next_id = 0;						    // The ID of the next unique node
			std::vector<Eigen::Triplet<float>> triplets;
			bool needs_compression = true;				// The CSR is inaccurate and requires compression

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
			//TODO: could this be cleaned up through a template?


			///<summary>
			/// Construct a graph from a list of nodes and edges. This is faster than just using the addedge method
			/// <summary> 
			Graph(
				const std::vector<std::vector<int>> & edges, 
				const std::vector<std::vector<float>> & distances, 
				const std::vector<Node> & Nodes
			);

			Graph() {};
			///<summary>
			/// Determine if the graph has an edge with this parent and child
			///</summary>
			bool HasEdge(const std::array<float, 3>& parent, const std::array<float, 3>& child, bool  undirected = false) const;
			bool HasEdge(const Node& parent, const Node& child, const bool undirected = false) const;
			bool HasEdge(int parent, int child, bool undirected = false) const;


			/// <summary>
			/// Get a list of nodes from the graph sorted by ID
			/// </summary>
			/// <returns>A sorted vector of nodes</returns>
			std::vector<Node> Nodes() const;

			/// <summary>
			/// Get a list of edges to and from node N
			/// </summary>
			std::vector<Edge> GetUndirectedEdges(const Node & N) const;
		
			/// <summary>
			/// Get every edge/node in the given graph as IDs
			/// </summary>
			std::vector<EdgeSet> GetEdges() const;

			/// <summary>
			/// Generate an ordered list of scores for the graph
			/// </summary>
			std::vector<float> AggregateGraph(COST_AGGREGATE agg_type, bool directed=true) const;

			/// <summary>
			/// Get a list of edges for the given node
			/// </summary>
			/// <param name="n"></param>
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
			void addEdge(int parent_id, int child_id, float score);

			/// <summary>
			/// Tell whether or not the graph has the given node in it as a parent
			/// </summary>
			/// <param name="n"> Node to check for</param>
			/// <returns>True if the node exists, false otherwise</returns>
			bool hasKey(const Node& n) const;

			/// <summary>
			/// Get a list of nodes as arrays of floats
			/// </summary>
			/// <returns>A list of float3s for each position of every node in the graph </returns>
			std::vector<std::array<float, 3>> NodesAsFloat3() const;

			/// <summary>
			/// Calculate how many nodes are in the graph
			/// </summary>
			/// <returns>An int displaying how many nodes are in the graph</returns>
			int size() const;

			/// <summary>
			/// Get the ID for the given node.
			/// </summary>
			/// <returns>The ID assigned to this node. -1 if it was not yet added to the graph</returns>
			int getID(const Node& node) const;

			/// <summary>
			/// Compress the graph, significantly reducing memory usage, but disabling insertion.
			/// </summary>
			void Compress();

			/// <summary>
			/// Obtain poitners to the 3 arrays of the CSR, as well as any extra info that may be necesary to use
			/// the CSR in another language. This will automatically compress the graph if it isn't compressed already
			/// </summary>
			CSRPtrs GetCSRPointers();

			/// <summary>
			/// Get the node belonging to that id
			/// </summary>
			Node NodeFromID(int id) const;

			/// <summary>
			/// Clear all nodes and edges in the graph.
			/// </summary>
			void Clear();

			/// <summary>
			/// Calculate cross slope for the given graph
			/// </summary>
			//TODO: Should these even be in the graph?
			void GenerateCrossSlope();
			void GenerateEnergy();
		};
	}
}
