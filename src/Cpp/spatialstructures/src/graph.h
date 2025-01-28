/// \file graph.h \brief Contains definitions for the
/// <see cref="HF::SpatialStructures::Graph">
/// Graph
/// </see>
/// class
///
/// \author TBA \date 06 Jun 2020
#pragma once

#include <robin_hood.h>
#include <vector>
#include <Edge.h>
#include <Node.h>
#include <path.h>
#include <Eigen>
#include <iostream>

namespace Eigen {
}

namespace HF::SpatialStructures {
	using EdgeMatrix = Eigen::SparseMatrix<float, 1>; ///< The type of matrix the graph uses internally
	using TempMatrix = Eigen::Map<const EdgeMatrix>;  ///< A mapped matrix of EdgeMatrix. Only owns pointers to memory. 

	/*! \brief Methods of aggregating the costs for edges for each node in the graph.

	\see Graph.AggregateGraph() for details on how to use this enum.
	*/
	enum class COST_AGGREGATE : int {
		/// Add the cost of all edges.
		SUM = 0,
		/// Average the cost of all edges
		AVERAGE = 1,
		/// Count how many edges this node has.
		COUNT = 2
	};

	/*! \brief Node to use for calculating the cost of an edge when converting node attributes to edge costs
	*/
	enum class Direction : int {
		INCOMING = 0, //< Use the child node's attribute for the cost.
		OUTGOING = 1, //< Use the parent node's attribute as the cost.
		BOTH = 2	//< Add the parent and child's attributes for the cost.
	};


	/*! \brief A struct to hold all necessary information for a CSR.

		\remarks
		This can be used by external clients to recreate or map to
		an existing CSR. For more information on what these arrays contain
		see
		https://eigen.tuxfamily.org/dox/group__TutorialSparse.html
	*/
	struct CSRPtrs {
		int nnz;	///< Number of non-zeros contained by the CSR
		int rows;	///< Number of rows in this CSR.
		int cols;	///< Number of columns in this CSR.

		float* data;			///< Stores the coefficient values of the non-zeros.
		int* outer_indices;		///< Stores for each column (resp. row) the index of the first non-zero in the previous two arrays.
		int* inner_indices;		///< Stores the row (resp. column) indices of the non-zeros.

		/// <summary>
		/// Verify the CSR referenced by this instance is valid.
		/// </summary>
		/// <returns>
		/// True if data, outer_indices, and inner_indices are non-null, false otherwise,
		/// </returns>
		/*!
			\details
			Simply checks that all the contained arrays are not null.

			\remarks
			For now, this just checks if any of the arrays are null since Eigen arrays will return
			null pointers if they are in an invalid state (uninitialized, uncompressed, etc).

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

		/// <summary>
		/// Returns the base address of the data buffer
		/// </summary>
		/// <returns>
		/// The base address of the data buffer
		/// </returns>

		/// If the data field has not given an address, data_begin will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/
		inline float* CSRPtrs::data_begin() const {
			return data ? data : nullptr;
		}

		/// <summary>
		/// Returns the address of one-past the last element within the data buffer
		/// </summary>
		/// <returns>
		/// The address of one-past the last element within the data buffer
		/// </returns>

		// If the data field has not been given an address, and/or the nnz field has not been given
		// a value, data_end will return nullptr.

		/*!
			\code
				// TODO example
			/endcode
		*/
		inline float* CSRPtrs::data_end() const {
			if (nnz > 0) {
				return data ? data + nnz : nullptr;
			}

			return nullptr;
		}

		/// <summary>
		/// Returns the base address of the inner_indices buffer
		/// </summary>
		/// <returns>
		/// The base address of the inner_indices buffer
		/// </returns>

		// If the inner_indices field has not been given an address, inner_begin will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/

		inline int* CSRPtrs::inner_begin() const {
			return inner_indices ? inner_indices : nullptr;
		}

		/// <summary>
		/// Returns the address of one-past the last element within the inner_indices buffer
		/// </summary>
		/// <returns>
		/// The address of one-past the last element within the inner_indices buffer
		/// </returns>

		/*!
			\detials
			If the inner_indices field has not been given an address, and/or the nnz field has not
			been given a value, inner_end will return nullptr.

			\code
				// TODO example
			\endcode
		*/

		inline int* CSRPtrs::inner_end() const {
			if (nnz > 0) {
				return inner_indices ? inner_indices + nnz : nullptr;
			}

			return nullptr;
		}

		/// <summary>
		/// Returns the base address of the outer_indices buffer
		/// </summary>
		/// <returns>
		/// The base address of the outer_indices buffer
		/// </returns>

		// If the outer_indices field has not been given an address, outer_begin will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/
		inline int* CSRPtrs::outer_begin() const {
			return outer_indices ? outer_indices : nullptr;
		}

		/// <summary>
		/// Returns the address of one-past the last element within the outer_indices buffer
		/// </summary>
		/// <returns>
		/// The address of one-past the last element within the outer_indices buffer
		/// </returns>

		// If the outer_end field has not been given an address, and/or the rows field has not been
		// given a value, outer_end will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/
		inline int* CSRPtrs::outer_end() const {
			if (rows > 0) {
				return outer_indices ? outer_indices + rows : nullptr;
			}

			return nullptr;
		}

		/// <summary>
		/// Returns the address of the first non-zero element of row_number within the CSR data buffer
		/// </summary>
		/// <param name="row_number">
		/// The desired row number to access within the CSR
		/// </param>
		/// <returns>
		/// The address of the first non-zero element within the CSR data buffer at row_number
		/// </returns>

		// If the data field has not been given an address, or the rows field has not been given a
		// value, or row_number >= rows, row_begin will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/
		inline float* CSRPtrs::row_begin(int row_number) const {
			float* begin = nullptr;

			if (data && rows > 0) {
				if (row_number >= 0 && row_number < rows) {
					begin = data + outer_indices[row_number];
				}
			}

			return begin;
		}

		/// <summary>
		/// Returns the address of the first non-zero element of row_number + 1, i.e. the base
		/// address of the next row within the CSR data buffer
		/// </summary>
		/// <param name="row_number">
		/// The desired row number for the CSR such that the address returned is the address
		/// pointing to the beginning element for the subsequent row
		/// </param>
		/// <returns>
		/// The address of the first non-zero element for row_number + 1 -- unless row_number + 1 ==
		/// csr.rows, then data_end() is returned
		/// </returns>

		// If the data field has not been given an address, or the rows field has not been given a
		// value, or row_number >= rows, row_end will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/
		inline float* CSRPtrs::row_end(int row_number) const {
			float* end = nullptr;
			const int next_row = row_number + 1;

			if (data && rows > 0) {
				if (next_row > 0 && next_row < rows) {
					end = data + outer_indices[next_row];
				}
				else if (next_row > 0 && next_row == rows) {
					end = data_end();
				}
			}

			return end;
		}

		/// <summary>
		/// Returns the address of the element that determines the column where the first non-zero
		/// value begins within row_number
		/// </summary>
		/// <param name="row_number">
		/// The desired row number for the CSR such that the address returned is of the value that
		/// dictates where the first non-zero value begins within row_number
		/// </param>
		/// <returns>
		/// The address of the value that represents the column index of the first non-zero value
		/// within row_number
		/// </returns>

		// If the inner_indices and/or outer_indices field have not been given addresses, or
		// row_number >= rows, col_begin will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/

		inline int* CSRPtrs::col_begin(int row_number) const {
			int* begin = nullptr;

			if (inner_indices && outer_indices) {
				if (row_number >= 0 && row_number < rows) {
					begin = inner_indices + outer_indices[row_number];
				}
			}

			return begin;
		}

		/// <summary>
		/// Returns the address of the element that denotes the end of a 'subarray' within inner_indices
		/// </summary>
		/// <param name="row_number">
		/// The desired row number for the CSR such that the address returned is one-past the last
		/// value for a 'subarray' within inner_indices
		/// </param>
		/// <returns>
		/// The address of the value that represents the column index of the first non-zero value
		/// for row_number + 1
		/// </returns>

		// If the inner_indices and/or outer_indices field have not been given addresses, or
		// row_number >= rows, col_end will return nullptr.

		/*!
			\code
				// TODO example
			\endcode
		*/
		inline int* CSRPtrs::col_end(int row_number) const {
			int* end = nullptr;
			const int next_row = row_number + 1;

			if (inner_indices && outer_indices) {
				if (next_row > 0 && next_row < rows) {
					end = inner_indices + outer_indices[next_row];
				}
				else if (next_row > 0 && next_row == rows) {
					end = inner_end();
				}
			}
			return end;
		}
	};

	/*!
		\brief A Subgraph consists of a parent Node m_parent and a container of Edge m_edges such that all Edge in m_edges
			   extend from m_parent.
	*/
	struct Subgraph {
		Node m_parent;						///< The parent node from which all Edge in m_edges extend
		std::vector<Edge> m_edges;			///< The edges that extend from m_parent
	};

	/*!
		\brief A set of edge costs for a graph.

		\details
		Arrays that can be swapped in and out of the array's indices.
	*/
	class EdgeCostSet {
	private:
		std::vector<float> costs; ///< Array of costs to be used like eigen's internal indices array.

	public:
		/*! \brief Construct an empty edge cost set. */
		EdgeCostSet() { this->costs.resize(0); };

		/*! 
			\brief Create an edge cost set and allocate a specific size.
			
			\param size Size of this cost set to be preallocated upon construction.		
		*/
		inline EdgeCostSet(int size) { this->ResizeIfNeeded(size); }

		/*! \brief Get the size of this edge matrix
			\returns how many values this matrix can hold, or
			the size of its internal costs array.
		*/
		inline int size() const { return this->costs.size(); };

		/*! \brief Resize this edge matrix if needed.
		
			\param new_size new size of the array to allocate to
			\details
			If size is less than the current size, this function doesn't do 
			anything. All new values are defaulted to NAN.
		*/
		inline void ResizeIfNeeded(int new_size) {
			const int old_size = this->size();
			if (this->size() < new_size)
			{
				// resize the cost array
				this->costs.resize(new_size);

				// fill new values with NAN
				const auto start_offset = costs.begin() + old_size;
				std::fill(start_offset, costs.end(), NAN);
			}
		};

		/*! \brief Clear all values from this edge cost set.*/
		inline void Clear() { this->costs.clear(); }

		/*!
			\brief Index internal values array
			\param i index to get the cost of
			\returns the value at index i in  costs.
		*/
		inline float& operator[](int i) { 
			assert(this->bounds_check(i));
			return this->costs[i]; 
		}

		inline bool bounds_check(int i) const {
			return !(i >= this->size()) || (i < 0);
		}

		/*!
			\brief Index internal values array
			\param i index to get the cost of
			\returns the value at index i in  costs.
		*/
		inline float operator[](int i) const { 
			assert(this->bounds_check(i));
			return this->costs[i]; 
		}

		/*! 
			\brief Get the pointer to the start of this array.
		
			\returns a pointer to the value of this EdgeCostSet's internal costs array
		*/
		inline float* GetPtr() {
			assert(this->costs.size() > 0);
			return this->costs.data();
		}
		
		/*! 
			\brief Get the pointer to the start of this array.
		
			\returns a pointer to the value of this EdgeCostSet's internal costs array
		*/
		inline const float* GetPtr() const { 
			assert(this->costs.size() > 0);
			return this->costs.data(); 
		}

		inline std::vector<float> GetEdgeCostSetCosts() {
			return this->costs;
		}
	};

	/*! \brief A Graph of nodes connected by edges that supports both integers and HF::SpatialStructures::Node.

		\details
		Internally, this object uses Eigen (https://eigen.tuxfamily.org/dox/group__TutorialSparse.html)
		to store and maintain a CSR matrix. The CSR is always stored as a n by n sparse matrix where
		n is the number of nodes in ordered_nodes.

		\par Cost Types
		This Graph is capable of holding multiple cost types for any of it's edges.
		Each cost type has a distinct key as it's name, such as "CrossSlope" or
		"EnergyExpenditure". Upon creation, the graph is assigned a default cost
		type, `Distance` which can be accessed explicitly by the key "Distance" or
		leaving the cost_type field blank. Alternate costs have corresponding edges
		in the default cost set, but different costs to traverse from the parent
		to the child node.

		\invariant 1) Every node in the graph will have a Unique ID with no repeats
		\invariant 2) Any edge cost set will be a valid replacement for CSR's values
		array. 

	*/
	class Graph {
		using NodeAttributeValueMap = robin_hood::unordered_map<int, std::string>;
		using NodeFloatAttributeValueMap = robin_hood::unordered_map<int, float>;
	private:
		int next_id = 0;								///< The id for the next unique node.
		std::vector<Node> ordered_nodes;				///< A list of nodes contained by the graph.

		//robin_hood::unordered_map<int, int> id_to_ordered_node; ///< Maps ids to indexes in ordered_nodes.
		robin_hood::unordered_map<Node, int> idmap;		///< Maps a list of X,Y,Z positions to positions in ordered_nodes

		std::vector<Eigen::Triplet<float>> triplets;	///< Edges to be converted to a CSR when Graph::Compress() is called.
		bool needs_compression = true;					///< If true, the CSR is inaccurate and requires compression.

		robin_hood::unordered_map<std::string, NodeAttributeValueMap> node_attr_map; ///< Node attribute type : Map of node id to node attribute
		robin_hood::unordered_map<std::string, NodeFloatAttributeValueMap> node_float_attr_map; ///< Node attribute type : Map of node id to node attribute for float
		std::string active_cost_type;								///< The active edge matrix to use for the graph
		EdgeMatrix edge_matrix;				///< The underlying CSR containing edge information.

		std::string default_cost = "Distance";/// < The default cost type of the graph. 
		std::unordered_map<std::string, EdgeCostSet> edge_cost_maps; ///< Hashmap containing evey alternate cost type

		/*!\brief Indicates that the graph has cost arrays.

		   \details
		   If this is true, and the graph compresses again, then all cost arrays will be wrecked.
		   An exception should be thrown in the case this happens, because this is misuse.
		*/
		bool has_cost_arrays = false;

		/*!
			\brief Determines whether or not the graph is using integer nodes.

			\details
			This causes the graph to spend more time finding the maximum node ID, since it's not
			gauranteed edges will be added in order.
		*/
		bool nodes_out_of_order = false;

		/*!
			\brief
			Get the unique ID for this x, y, z position and assign it an new one if it doesn't already exist.

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
		int getOrAssignID(const Node & input_node);

		/*!
			\brief Add an ID to the graph if it doesn't exist already.

			\warning
			Adding integer edges to the graph isn't entirely supported. This WILL create
			gaps in the CSR, and break the order of ordered_nodes.

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

		/*!
			\brief Add a new edge cost to the CSR or update if if a cost already exists.

			\param parent_id ID of the parent node.
			\param child_id ID of the child node
			\param cost Cost of traversing from parent to child

			\remarks
			This is called when the graph is compressed and the user tries to add
			a new edge.

			\pre parent_id and child_id point to valid nodes in the graph.

			\warning
			This will invalidate any EdgeCostSets. Don't call this if you have
			edge cost sets.
		*/
		void CSRAddOrUpdateEdge(int parent_id, int child_id, float cost);

		/*! \brief Add a new edge to the triplets list.

			\param parent_id Id of the parent node.
			\param child_id Id of the child node.
			\param cost Cost of traversing from parent to child.

			\pre parent_id and child_id point to valid nodes in the graph.
		*/
		void TripletsAddOrUpdateEdge(int parent_id, int child_id, float cost);

		/*!
			\brief Resize the CSR to fit all the nodes in ordered_nodes if needed.

			\details
			If the CSR can already fit all of the ndoes in ordered_nodes then this won't do
			anything.

			\post The CSR will be large enough to fit all of the nodes in ordered_nodes.

		*/
		void ResizeIfNeeded();

		/*!
			\brief Check if this ID has already been assigned.
			\param id Id of the node to check

			\returns True if the ID has already been assigned to a node in the graph. False otherwise.

			\par TimeComplexity
			Performs a search over every node in the graph: O(n).

		*/
		bool hasKey(int id) const;

		/*!
			\brief Check if we have this edge matrix already defined.

			\param Unique key of the cost type to search for in the cost map.
			\returns True there is a cost with this name, false otherwise.

			\pre Key does not belong to the default graph.

		*/
		bool HasCostArray(const std::string & key) const;

		/*!
			\brief Get a reference to the edge matrix at the given key.

			\param key Name of the cost to retrieve.

			\returns The EdgeCostArray with the name of `key`.

			\exception HF::Exceptions::NoCost if the given key doesn't exist

			\pre `key` is not the default graph name or an empty string.

			\see HasCostArray for a way of checking that the cost type exists before calling
				in situations where throwing is not possible, or unwanted.
		*/
		EdgeCostSet& GetCostArray(const std::string& key);

		/*!
			\brief Get a reference to the edge matrix, or create a new one if it doesn't exist

			\param name Name of the cost matrix to create or retrieve.

			\returns A reference to the existing cost array, or the newly created cost array.

			\pre `key` is not the default name in the graph or an empty string.
		*/
		EdgeCostSet& GetOrCreateCostType(const std::string& name);

		/*! \brief Create a new edge matrix.

			\param name Unique name of the new cost array

			\returns A reference to the new edge cost set

			\pre 1) `name` does not already belong to another cost array in the graph.
			\pre 2) `name` is not the default name in the graph or an empty string.
		*/
		EdgeCostSet& CreateCostArray(const std::string& name);

		/*!
			\brief Get a reference to the edge matrix at the given key.

			\param key Name of the cost to retrieve.

			\returns The EdgeCostArray with the name of `key`.

			\exception HF::Exceptions::NoCost if the given cost doesn't exist.

			\see HasCostArray for a way of checking that the cost type exists before calling
				in situations where throwing is not possible, or unwanted.
		*/
		const EdgeCostSet& GetCostArray(const std::string& key) const;

		/*!
			\brief Check if this name belongs to the default graph.

			\param name The name to test against the default.

			\returns
			True if this name belongs to the default CSR. False otherwise.

			\note
			The default name is either the name the graph was constructed with ("Distance" if none)
			or an empty string that returns true when calling string.empty().

		*/
		bool IsDefaultName(const std::string& name) const;

		/*!
			\brief Get the index of the cost at parent/child.

			\param parent_id ID of the edge's parent.
			\param child_id ID of the edge's child.

			\returns
			-1 if there is no edge between parent and child. Otherwise returns
			the index in the values array that belongs to the cost of traversing
			from parent to child.

			\details
			Determines the start and end bounds of the row belonging to `parent_id` using
			the CSR's outer_index_ptr, then searches these bounds in inner_indices for `child_id`.
			If a match is found, the distance from child_id to the beginning of the csr's inner_index
			array is calculated and returned.

			\remarks
			This is used in several places to index EdgeCostSets.

			\pre `parent_id` is a valid node in the graph.
		*/
		int FindValueArrayIndex(int parent_id, int child_id) const;

		/*!
			\brief Add an edge to a cost set between parent_id and child_id.

			\param parent_id The id of the parent node in the graph
			\param child_id the ID of the child node in the graph

			\exception std::out_of_range no edge from parent_id to child_id exists in the default cost type.

			\pre 1) An edge from parent to child already exists in the graph for the default cost type.
			\pre 2) parent_id and child_id are both IDs of nodes that already exist in the graph.
		*/
		void InsertEdgeIntoCostSet(int parent_id, int child_id, float score, EdgeCostSet& cost_set);

		/*!
			\brief Insert edges for a specific cost type into a cost set.

			\param cost_set Set of costs to insert edges into.
			\param es An array of edge_sets containing parent and child nodes to add to the graph.

			\pre All edges in es already exist in the default graph.
		*/
		void InsertEdgesIntoCostSet(EdgeCostSet& cost_set, const std::vector<EdgeSet>& es);

		/*!
			\brief Insert an edge into the default cost array or a new cost array.

			\param parent_id ID of the edge's parent node
			\param child_id ID of the edge's child node
			\param score Cost of traversing from parent to child
			\param cost_type The type of cost to add this edge to

			\details
			If the graph isn't compressed, calls TripletsAddOrUpdateEdge(). If the graph
			is compressed calls CSRAddOrUpdateEdge(). If the cost at cost_type doesn't
			exist, then it will be created.

			\pre 1) parent_id and child_id already exist in the graph.
			\pre 2) If not using the default cost_type the must already be compressed.
			\pre 3) Any edges being added to alternate cost types already exist in the
			default graph.

			\throws std::logic_error If trying to add an edge to an alternate cost type
			when the graph hasn't been compressed.
			\throws std::logic_error If trying to add an edge between nodes that doesn't exist
			already in the graph.

		*/
		void InsertOrUpdateEdge(int parent_id, int child_id, float score, const std::string& cost_type);

		/*!
			\brief Get the cost of traversing the edge between parent and child using set

			\param set The set of edges to get this cost in
			\param parent_id ID of the edge's parent node.
			\param child_id ID of the edge's child node

			\returns
			NAN if no edge exists in `set` between `parent_id` and `child_id`. If an edge
			does exist betwen `parent_id` and `child_id` in `set` then returns the cost

			\pre parent_id and child_id both belong to nodes that already exist in the graph.

		*/
		float GetCostForSet(const EdgeCostSet& set, int parent_id, int child_id) const;

		/*! 
			\brief Get the edges for the given node.
			
			\param parent_id Node to get the outgoing edges of
			\param undirected If this is true, then get both outgoing and incoming edges of parent_id
			\param cost_type Cost type to use for retrieved edges

			\returns
			All edges from (or to if undirected is true) parent_id for the given cost type
		
			\throws NoCost if cost_type isn't eh default graph and doesn't exist in the graph's cost types

		*/
		std::vector<Edge> GetEdgesForNode(
			int parent_id,
			bool undirected = false,
			const std::string& cost_type = ""
		) const;

		/*! 
			\brief Construct a temp matrix for the specific cost type.
			
			\param cost_type The type of cost to generate the temp matrix for

			\returns 
			A newly constructed TempMatrix with the outer and inner indices of 
			the default edge_matrix but the values of `cost_type`.

			\pre 1) cost_type must be a valid cost type that already exists in the graph
			\pre 2) cost_type must not be the default cost type. 

		*/
		TempMatrix MapCostMatrix(const std::string& cost_type) const;

		/*! \brief Check if this graph has a specific node attribute*/
		bool HasNodeAttribute(const std::string & key) const;

	public:
		/*!
		 \brief Construct a graph from a list of nodes, edges, and distances.

		 \param edges Ordered array of arrays of edges for each node in nodes.
		 \param distances Ordered array of distance from parent to child for each edge in edges.
		 \param Nodes Ordered array of nodes to act as a parent to all children in it's array in edges.
		 \param default_cost Default cost of the graph. This is the name of the first used cost.

		\details Preallocates the matrix it in element by element and compresses it.

		 \pre 1) The size of all input arrays must match:
		 `(edges.size() == nodes.size() && nodes.size() == distances.size())`

		\pre 2) For the node at `nodes[i]`, `edges[i]` should contain an array for the id of all nodes
		that `nodes[i]` has an edge from and, and `distances[i]` should contain an array of the the distance
		from `nodes[i]` to one of the nodes it has an edge to in `edges[i]`.

		\note After constructing a graph with this constructor, it will not be able to be modified.
		Use the empty constructor and use addEdge if you want to modify the graph after construction.
		This may change in the future.

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
			const std::vector<std::vector<int>>& edges,
			const std::vector<std::vector<float>>& distances,
			const std::vector<Node>& Nodes,
			const std::string& default_cost = "Distance"
		);

		/*! \brief Construct an empty graph.

			\remarks This can be used to create a new graph to later be filled with edges/nodes
			by calling Graph::addEdge() then calling Graph::Compress(). Implementation
			is based on the Eigen documentation for Filling a CSR:
			https://eigen.tuxfamily.org/dox/group__TutorialSparse.html.

			\see Graph::addEdge() for details on adding edges.
			\see Graph::Compress() for details on compressing the graph.

			\code
				// be sure to #include "graph.h"
				HF::SpatialStructures::Graph graph;		// This represents an order-zero graph (null graph)
														// It lacks vertices and edges.
			\endcode
		*/
		Graph(const std::string& default_cost_name = "Distance");

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
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

		/*! \brief Determine if the graph has an edge from parent to child.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// last argument can be true/false for undirected/directed graph respectively
				bool has_edge = graph.HasEdge(node_1, node_2, true);
			\endcode
		*/
		bool HasEdge(
			const Node& parent,
			const Node& child,
			const bool undirected = false,
			const std::string cost_type = ""
		) const;

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// last argument can be true/false for undirected/directed graph respectively
				bool has_edge = graph.HasEdge(0, 1, true);
			\endcode
		*/
		bool HasEdge(int parent, int child, bool undirected = false, const std::string& cost_type = "") const;

		/// <summary>
		/// Get a list of nodes from the graph sorted by ID.
		/// </summary>
		/// <returns>
		/// A sorted vector of nodes.
		/// </returns>

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Nodes() returns a copy of the ordered_nodes field
				std::vector<HF::SpatialStructures::Node> nodes_from_graph = graph.Nodes();
			\endcode
		*/
		std::vector<Node> Nodes() const;

		/// <summary>
		/// Get a list of all edges to and from node N.
		/// </summary>
		/// <param name="N">
		/// The Node to get edges from and to.
		/// </param>
		/// <returns>
		/// A list of edges to and from node N or an empty array if is not in the graph.
		/// </returns>
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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Retrieve the nodes from the graph, or use the original instance of
				// std::vector<HF::SpatialStructures::Node> passed to Graph upon instantiation
				std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

				// nodes[index] yields an instance of Node that we can pass to GetUndirectedEdges.
				// Any node that exists with graph can be passed to this member function to retrieve
				// a vector of undirected edges.
				int index = 2;
				std::vector<HF::SpatialStructures::Edge> undirected_edges = graph.GetUndirectedEdges(get_nodes[index]);
			\endcode
		*/
		std::vector<Edge> GetUndirectedEdges(const Node& N, const std::string& cost_type = "") const;

		/// <summary>
		/// Get every in the given graph as IDs.
		/// </summary>
		/// <returns>
		/// An array of edgesets for every node in the graph (Graph in the form of IDs).
		/// </returns>
		/*!

			\exception std::exception if the graph hasn't been compressed. compressed.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// graph must be compressed, or a exception will be thrown

				// To brief, an EdgeSet has the following layout: struct EdgeSet { int parent;
				// std::vector<IntEdge> children; };
				//
				// An IntEdge has the following layout: struct IntEdge { int child; float weight; };

				// A std::vector<EdgeSet> is a Graph, in the form of IDs.
				std::vector<HF::SpatialStructures::EdgeSet> edge_set = graph.GetEdges();
			\endcode
		*/
		std::vector<EdgeSet> GetEdges() const;
	
		/*! \brief Get children of a specific node as integers
		
			
		
		*/
		std::vector<IntEdge> GetIntEdges(int parent) const;

		/// <summary>
		/// Summarize the costs of every outgoing edge for every node in the graph.
		/// </summary>
		/// <param name="agg_type">
		/// Type of aggregation to use.
		/// </param>
		/// <param name="directed">
		/// If true, include both incoming and outgoing edges for calculating a node's score.
		/// </param>
		/// <returns>
		/// An ordered list of scores for agg_type on each node in the graph.
		/// </returns>
		/*!

			\remarks Useful for getting scores from the VisibilityGraph.

			\exception std::out_of_range if agg_type doesn't match any value of COST_AGGREGATE.
			\exception Std::exception if the graph isn't compressed.

			\par Time Complexity
			If undirected: `O(k)` where k is the total number of edges in the graph.\n
			If directed: `O(n)` where n is the total number of nodes in the graph.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// graph must be compressed, or a exception will be thrown
				HF::SpatialStructures::COST_AGGREGATE aggregate = HF::SpatialStructures::COST_AGGREGATE::AVERAGE;		// aggregate == 1 in this case

				// directed parameter may be true or false
				std::vector<float> aggregate_graph = graph.AggregateGraph(aggregate, true);
			\endcode
		*/
		std::vector<float> AggregateGraph(COST_AGGREGATE agg_type, bool directed = true, const std::string& cost_type = "") const;
		/*!
			\todo Should this just return an empty list instead of throwing?
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
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Retrieve the nodes from the graph, or use the original instance of
				// std::vector<Node> passed to Graph upon instantiation
				std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

				// nodes[index] yields an instance of Node that we can pass to GetUndirectedEdges.
				// Any node that exists with graph can be passed to this member function to retrieve
				// a vector of edges.
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
		/// Add a new edge to the graph from parent to child.
		/// </summary>
		/// <param name="parent">
		/// Parent node of the edge.
		/// </param>
		/// <param name="child">
		/// Child node of the edge.
		/// </param>
		/// <param name="score">
		/// Cost of traversing from aprent to child.
		/// </param>
		/*!
			\details
			If the parent or child node do not have an ID. An ID will be assigned
			automatically.

			\warning
			This will not work if the graph wasn't created from the empty constructor since
			it has no internal edge list to add to.

			\remarks
			This adds a new element to the triplet list so next time Compress
			is called, the value is added to the graph.

			\todo How should this signal that the graph can't have edges added to it? Or how do
			we add edges to an existing graph quickly without adding to its edge list?

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
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Create a pair of nodes
				HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
				HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

				graph.addEdge(n_parent, n_child);	// default score is 1.0f
			\endcode
		*/
		void addEdge(const Node& parent, const Node& child, float score = 1.0f, const std::string& cost_type = "");

		/// <summary>
		/// Add a new edge to the graph from parent to child.
		/// </summary>
		/// <param name="parent">
		/// Parent node of the edge.
		/// </param>
		/// <param name="child">
		/// Child node of the edge.
		/// </param>
		/// <param name="score">
		/// Cost of traversing from aprent to child.
		/// </param>
		/*!

			\param cost_type Type of cost to add this edge to 
			
			\details
			If the parent or child ids don't exist in the dictionary, they will be added.

			\warning
			This will not work if the graph wasn't created from the empty constructor since
			it has no internal edge list to add to.

			\remarks
			This adds a new element to the triplet list so next time Compress is called,
			the value is added to the graph.
			(Note: if an edge exists between parent_id and child_id,
			the score value will be added to the existing score value for the edge
			formed by parent_id and child_id).

			\throws std::logic_error Tried to add an edge to an alternate cost type when the graph isnt compressed
			\throws std::out_of_range Tried to add an edge to an alternate cost type when it
			hasn't been added to the default graph

			\todo How should this signal that the graph can't have edges added to it? Or how do
			we add edges to an existing graph quickly without adding to its edge list?

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				int parent = 1;
				int child = 2;

				graph.addEdge(parent, child, 1.0f);
				graph.Compress();
			\endcode
		*/
		void addEdge(int parent_id, int child_id, float score, const std::string& cost_type = "");

		/// <summary>
		/// Determine if n exists in the graph.
		/// </summary>
		/// <param name="n">
		/// Node to look for.
		/// </param>
		/// <returns>
		/// True if the node exists, false otherwise.
		/// </returns>

		/*!

			\details
			Performs a single hash to check if n exists in the hashmap.

			\par Time Complexity
			O(1) since it's a single hash function.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
				bool has_key = graph.hasKey(other_node);	// other_node does not exist in graph, so has_key == false;

				// Likewise, if we pass a Node instance that indeed exists...

				// Retrieve the nodes from the graph, or use the original instance of
				// std::vector<Node> passed to Graph upon instantiation
				std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

				// nodes[index] yields an instance of Node that we can pass to hasKey. Any node that
				// exists with graph can be passed to this member function to determine if the graph
				// has the node's key, or not.
				int index = 2;
				HF::SpatialStructures::Node good_node = get_nodes[index];

				has_key = graph.hasKey(good_node);		// now has_key is true
			\endcode
		*/
		bool hasKey(const Node& n) const;

		/// <summary>
		/// Get a list of nodes as float arrays.
		/// </summary>
		/// <returns>
		/// An array of float arrays containing the position of every node in the graph in order.
		/// </returns>
		/*!
			\remarks
			May be useful for to functions that take arrays instead of nodes.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// A container of std::array<float, 3> is constructed and populated within
				// NodesAsFloat3, and returned. Each array of 3 floats represents a Node's position
				// within the Cartesian coordinate system. { x, y, z }
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

		/// <summary>
		/// Determine how many nodes are in the graph.
		/// </summary>
		/// <returns>
		/// The number of nodes in the graph.
		/// </returns>
		/*!
			\details Size is directly returned from `id_to_nodes.size()`.
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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				int id_count = graph.size();		  // We retrieve the size of the node id count within graph (3)
			\endcode
		*/
		int size() const;

		/*! 
			\brief Calculate the maximum ID of any node in the graph.
			
			\returns The maximum ID of any node ing the graph.

			\note This shouldn't be needed often, as unless this graph has integer edges,
			nodes will always be stored in order. 
		
		*/
		int MaxID() const;

		/// <summary>
		/// Retrieve the ID for node in this graph.
		/// </summary>
		/// <returns>
		/// The ID assigned to this node. -1 if it was not yet added to the graph
		/// </returns>
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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
				bool has_key = graph.hasKey(other_node);							// other_node does not exist in graph, so has_key == false;

				int ID = graph.getID(other_node);									// ID will assigned -1, because other_node is not a part of graph.

				// Likewise, if we pass a Node instance that indeed exists...

				// Retrieve the nodes from the graph, or use the original instance of
				// std::vector<Node> passed to Graph upon instantiation
				std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

				// nodes[index] yields an instance of Node that we can pass to hasKey. Any node that
				// exists with graph can be passed to this member function to determine if the graph
				// has the node's key, or not.
				int index = 2;					// we assume for this example that index 2 is valid.
				HF::SpatialStructures::Node good_node = get_nodes[index];

				ID = graph.getID(good_node);	// ID > -1, i.e. it is a Node instance that exists within this Graph.
			\endcode
		*/
		int getID(const Node& node) const;

		/*!
			\brief Compress the graph to a CSR and enable the usage of several functions.

			\details
			This won't do anything if called on an already compressed graph. The graph is "compressed"
			by resizing the edge matrix to the maximum ID of any node in triplets, then calling
			setFromTriplets().

			\note
			This function actually doesn't actually reduce memory usage since it keeps the edge list
			in order to allow for modifications to the graph. In the future, it may be beneficial allow for
			the user to pass in a boolean that would delete the triplet array if true.

			\remarks
			This method of constructing the CSR is based on Eigen's documentation for
			Filling a sparse matrix https://eigen.tuxfamily.org/dox/group__TutorialSparse.html.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
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
		/// Obtain the size of and pointers to the 3 arrays that comprise this graph's CSR. graph if
		/// it isn't compressed already
		/// </summary>
		/// <returns>
		/// Pointers and sizes of the arrays that comprise the CSR. If the CSR cannot be constructed
		/// due to factors such as an empty input array, then the CSRPtrs contain null values for
		/// it's pointers.
		/// </returns>

		/*!
			\details This will automatically call Compress if it hasn't been called already.

			\remarks
			This can be useful for reconstructing or mapping to the CSR without interacting
			with eigen at all. Numpy can directly make map the arrays returned by this function
			to it's own CSR implementation.

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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Create a pair of nodes
				HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
				HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

				graph.addEdge(n_parent, n_child);	// default score is 1.0f

				// Graph will be compressed automatically be GetCSRPointers
				CSRPtrs returned_csr = graph.GetCSRPointers();
			\endcode

			\see CSRPtrs.AreValid() for checking if the return value represents a valid CSR.
		*/
		CSRPtrs GetCSRPointers(const std::string& cost_type = "");

		/// <summary>
		/// Retrieve the node that corresponds to id.
		/// </summary>
		/// <param name="id">
		/// The ID of the node to get.
		/// </param>
		/// <returns>
		/// The node corresponding to id, by value.
		/// </returns>
		/*!
			\exception std::out_of_range id didn't belong to any node in the graph.
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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Let's retrieve node_1.
				int desired_node_id = 2;
				HF::SpatialStructures::Node node_from_id = graph.NodeFromID(desired_node_id);

				// Note that NodeFromID ceases to work if the id argument provided does not exist as
				// an ID among the nodes within graph
			\endcode
		*/
		Node NodeFromID(int id) const;

		/// <summary>
		/// Clear all nodes and edges from the graph.
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

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				// Note: graph is compressed upon instantiation
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// If we want to remove all nodes and edges from graph, we may do so with Clear:
				graph.Clear();						// active_edge_matrix is zeroed out, buffer is squeezed,
													// triplets are also cleared, and
													// needs_compression == true
			\endcode
		*/
		void Clear();

		/// <summary>
		/// Retrieve n's child nodes - n is a parent node
		/// </summary>
		/// <param name="n">
		/// The parent node from which child nodes will be derived
		/// </param>
		/// <returns>
		/// A container of child nodes that form edges that extend from parent node n
		/// </returns>

		/*!
			\code
				// TODO example
			\endcode
		*/
		std::vector<Node> GetChildren(const Node& n) const;

		/// <summary>
		/// Retrieve node parent_id's child nodes
		/// </summary>
		/// <param name="parent_id">
		/// The parent node ID from which child nodes will be derived
		/// </param>
		/// <returns>
		/// A container of child nodes that form edges that extend from node parent_id
		/// </returns>

		/*!
			\code
				// TODO example
			\endcode
		*/
		std::vector<Node> GetChildren(const int parent_id) const;

		/*!
			\brief Retrieves a Subgraph using a Node
			\param parent_node	The parent node from which the Subgraph will be derived
			\return A structure that consists of parent_node and the container of Edge that consists of the Edge that extend from parent

			\code
				// TODO example
			\endcode
		*/
		Subgraph GetSubgraph(const Node& parent_node, const std::string& cost_type = "") const;

		/*!
			\brief	Retrieves a Subgraph using a parent node ID
			\param parent_id	The parent node id from which the Subgraph will be derived
			\return	A structure that consists of the node at parent_id and the container of Edge that consists of the Edge that extend from parent_id

			\code
				// TODO example
			\endcode
		*/
		Subgraph GetSubgraph(int parent_id, const std::string & cost_type = "") const;

		/// <summary>
		/// Add an attribute to the node at id
		/// </summary>
		/// <param name="id">
		/// The ID of the node that will receive attribute
		/// </param>
		/// <param name="attribute">
		/// The attribute that the node at ID will receive
		/// </param>
		/// <param name="score">
		/// The weight, or distance that extends from the node at id
		/// </param>

		/*!
			\code
				// TODO example
			\endcode
		*/
		void AddNodeAttribute(int id, const std::string & attribute, const std::string & score);

		/// <summary>
		/// Add a float attribute to the node at id. If the node at id already has a score for the
		/// attribute at name, then existing score should be overwritten.
		/// 
		/// If the attribute is a string attribute, score will be added
		/// as a string value. The attribute will not be converted into a float
		///	attribute.
		/// </summary>
		/// <param name="id">
		/// The ID of the node that will receive attribute
		/// </param>
		/// <param name="name">
		/// The attribute that the node at ID will receive
		/// </param>
		/// <param name="score">
		/// The weight, or distance that extends from the node at id
		/// </param>

		/*!
			\code
				// TODO example
			\endcode
		*/

		void AddNodeAttributeFloat(int id, const std::string& name, const float score);

		/// <summary>
		/// Add an attribute to the node at id. If the node at id already has a score for the
		/// attribute at name, then existing score should be overwritten
		/// </summary>
		/// <param name="id">
		/// The container of IDs from which nodes will be retrieved and given attributes
		/// </param>
		/// <param name="name">
		/// The attribute that each node will receive
		/// </param>
		/// <param name="scores">
		/// The container of score, ordered by the container of node IDs
		/// </param>

		/*!
			\pre The length of ids, and the length of scores must be equal 
			\throws std::logic_error The length of scores and the length of ID do not match.

			\code
				// TODO example
			\endcode
		*/
		void AddNodeAttributes(const std::vector<int> & id, const std::string &  name, const std::vector<std::string> & scores);
		/// <summary>
		/// Add a float attribute to the node at id. If the node at id already has a score for the
		/// attribute at name, then existing score should be overwritten.
		/// 
		/// If the attribute is a string attribute, scores will be added
		/// as string values. The attribute will not be converted into a float
		///	attribute.
		/// </summary>
		/// <param name="id">
		/// The container of IDs from which nodes will be retrieved and given attributes
		/// </param>
		/// <param name="name">
		/// The attribute that each node will receive
		/// </param>
		/// <param name="scores">
		/// The container of score, ordered by the container of node IDs
		/// </param>

		/*!
			\pre The length of ids, and the length of scores must be equal
			\throws std::logic_error The length of scores and the length of ID do not match

			\code
				// TODO example
			\endcode
		*/
		void AddNodeAttributesFloat(const std::vector<int>& id, const std::string& name, const std::vector<float>& scores);

		/// <summary>
		/// Get the score for the given attribute of every node in the graph. Nodes that do not have
		/// a score for this attribute should return an empty string for this array.
		/// </summary>
		/// <param name="attribute">
		/// The attribute from which a container of scores will be obtained
		/// </param>
		/// <returns>
		/// A container of score, each in the form of a std::string, obtained from attribute
		/// </returns>

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
 
				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Get node IDs
				int ID_0 = graph.getID(node_0);
				int ID_1 = graph.getID(node_1);
				int ID_2 = graph.getID(node_2);

				std::vector<int> ids = {ID_0, ID_1, ID_2};

				// Assign attributes to nodes
				std::string attribute = "demo attribute";
				std::vector<std::string> scores = {"2.3", "6.1", "4.0"};
				graph.AddNodeAttributes(ids, attribute, scores);

				// Get attribute for all nodes
				std::vector<std::string> cross_slopes = graph.GetNodeAttributes(attribute); // {"2.3", "6.1", "4.0"}
			\endcode
		*/
		std::vector<std::string> GetNodeAttributes(std::string attribute) const;
		/// <summary>
		/// Get the score for the given attribute of every node in the graph. Nodes that do not have
		/// a score for this attribute should return the default value 0.0 for this array.
		/// </summary>
		/// <param name="name">
		/// The attribute from which a container of scores will be obtained. 
		/// </param>
		/// <returns>
		/// A container of score, each in the form of a float, obtained from attribute
		/// </returns>
		/*!
			\pre	`name` is a float attribute. That is, only float values have been added to this attribute.

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

				// Get node IDs
				int ID_0 = graph.getID(node_0);
				int ID_1 = graph.getID(node_1);
				int ID_2 = graph.getID(node_2);

				std::vector<int> ids = {ID_0, ID_1, ID_2};

				// Assign attributes to nodes
				std::string attribute = "demo attribute";
				std::vector<float> scores = {2.3, 6.1, 4.0};
				graph.AddNodeAttributesFloat(ids, attribute, scores);

				// Get attribute for all nodes
				std::vector<float> cross_slopes = graph.GetNodeAttributesFloat(attribute); // {2.3, 6.1, 4.0}
			\endcode
		*/
		std::vector<float> GetNodeAttributesFloat(std::string name) const;
		/// <summary>
		/// Get the score for the given attribute of the specified nodes. Nodes that do not have
		/// a score for this attribute should return an empty string for this array.
		/// </summary>
		/// <param name="ids">
		/// A list of node IDs to obtain scores for.
		/// </param>
		/// <param name="attribute">
		/// The attribute from which a container of scores will be obtained
		/// </param>
		/// <returns>
		/// A container of score, each in the form of a std::string, obtained from attribute
		/// </returns>
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
 
				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Get node IDs
				int ID_0 = graph.getID(node_0);
				int ID_1 = graph.getID(node_1);
				int ID_2 = graph.getID(node_2);
				std::vector<int> ids = {0, 1, 2};

				// Assign attributes to nodes
				std::string attribute = "demo attribute";
				std::vector<std::string> scores = {"1.8", "9.6", "5.7"};
				graph.AddNodeAttributes(ids, attribute, scores);

				// Get attribute for specific nodes
				std::vector<std::string> cross_slope_1 = graph.GetNodeAttributesByID({ID_1}, attribute); // {"9.6"}
				std::vector<std::string> cross_slope_02 = graph.GetNodeAttributesByID({ID_0, ID_2}, attribute); // {"1.8", "5.7"}
			\endcode
		*/
		std::vector<std::string> GetNodeAttributesByID(std::vector<int>& ids, std::string attribute) const;

		/// <summary>
		/// Get the score for the given attribute of the specified nodes. Nodes that do not have
		/// a score for this attribute should return an empty string for this array.
		/// </summary>
		/// <param name="ids">
		/// A list of node IDs to obtain scores for.
		/// </param>
		/// <param name="name">
		/// The attribute from which a container of scores will be obtained.
		/// </param>
		/// <returns>
		/// A container of score, each in the form of a std::string, obtained from name
		/// </returns>
		/*!
			\pre	`name` is a float attribute. That is, only float values have been added to this attribute.

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

				// Get node IDs
				int ID_0 = graph.getID(node_0);
				int ID_1 = graph.getID(node_1);
				int ID_2 = graph.getID(node_2);
				std::vector<int> ids = {0, 1, 2};

				// Assign attributes to nodes
				std::string attribute = "demo attribute";
				std::vector<float> scores = {1.8, 9.6, 5.7};
				graph.AddNodeAttributesFloat(ids, attribute, scores);

				// Get attribute for specific nodes
				std::vector<float> cross_slope_1 = graph.GetNodeAttributesByIDFloat({ID_1}, attribute); // {9.6}
				std::vector<float> cross_slope_02 = graph.GetNodeAttributesByIDFloat({ID_0, ID_2}, attribute); // {1.8, 5.7}
			\endcode
		*/
		std::vector<float> GetNodeAttributesByIDFloat(std::vector<int>& ids, std::string name) const;

		/*! \brief Check if this attribute exists in the graph and contains float values*/
		bool IsFloatAttribute(const std::string& name) const;

		/// <summary>
		/// Count the number of edges of associated cost type
		/// </summary>
		/// <param name="cost_type">
		/// The desired cost type to count the number of edges of
		/// </param>
		/// <returns>
		/// The number of edges of associated cost type in the graph
		/// </returns>

		int CountEdges(const std::string& cost_type) const;

		/// <summary>
		/// Count the number of edges in a given edgeset.
		/// </summary>
		/// <param name="AllEdges">
		/// The edgeset to count the number of edges in
		/// </param>
		/// <returns>
		/// The number of edges in the given edgeset
		/// </returns>
		
		int CountEdgesFromEdgeSets(std::vector<EdgeSet> AllEdges) const;

		/// <summary>
		/// Get edge costs of all given edges
		/// </summary>
		/// <param name="cost_type">
		///  The desired cost type to get costs for
		/// </param>
		/// <returns>
		/// A vector of floats containing all costs of given cost type
		/// </returns>

		std::vector<float> GetEdgeCosts(const std::string& cost_type) const;

		/// <summary>
		/// Get costs of all given edges
		/// </summary>
		/// <param name="ids">
		///  The ids of nodes to map to edges for cost calculation
		/// </param>
		/// <param name="cost_type">
		///  The desired cost type to get costs for
		/// </param>
		/// <returns>
		/// A vector of floats containing costs of given cost type associated with the given edges
		/// </returns>
		
		std::vector<float> GetEdgeCostsFromNodeIDs(std::vector<int>& ids, const std::string& cost_type) const;
		
		/// <summary>
		/// Maps a path structure to a vector of node ids (n1,n2,n2,n3,n3...,nk-1,nk)
		/// </summary>
		/// <param name="path">
		///  The path of nodes to map
		/// </param>
		/// <returns>
		/// A vector of node ids that represents the path in the form (n1,n2,n2,n3,n3...,nk-1,nk)
		/// </returns>

		std::vector<int> Graph::MapPathToVectorOfNodes(HF::SpatialStructures::Path path) const;

		/// <summary>
		/// Maps a path of node ids (n1,n2,...,nk) to (n1,n2,n2,n3,n3...,nk-1,nk)
		/// </summary>
		/// <param name="path">
		///  The path of nodes to map
		/// </param>
		/// <returns>
		/// A vector of node ids that represents the path in the form (n1,n2,n2,n3,n3...,nk-1,nk)
		/// </returns>

		std::vector<int> MapPathToVectorOfNodes(std::vector<int>& path) const;

		/// <summary>
		///  Computes an alternate cost type between nodes along a path.
		/// </summary>
		/// <param name="path">
		///	The path of nodes to get costs between.
		/// </param>
		/// <param name="cost_type">
		///	The type of cost to query
		/// </param>
		/// <returns>
		/// A vector of floats, representing each cost between nodes along the path.
		/// </returns>

		std::vector<float> Graph::AlternateCostsAlongPath(Path path, const std::string& cost_type) const;

		/// <summary>
		///  Computes an alternate cost type between nodes along a path.
		/// </summary>
		/// <param name="path">
		///	The path of nodes to get costs between.
		/// </param>
		/// <param name="cost_type">
		///	The type of cost to query
		/// </param>
		/// <returns>
		/// A vector of floats, representing each cost between nodes along the path.
		/// </returns>
		
		std::vector<float> AlternateCostsAlongPath(std::vector<int>& path, const std::string& cost_type) const;

		/// <summary>
		/// Clears the attribute at name and all of its contents from the internal hashmap
		/// </summary>
		/// <param name="name">
		/// The attribute that will be cleared from this graph's internal hashmap
		/// </param>

		/*!
			\brief
			\param name

			\code
				// TODO example
			\endcode
		*/
		void ClearNodeAttributes(std::string name);

		bool DumpToJson(const std::string & path);

		/*!
			\brief Add multiple edges to the graph.

			\param edges The set of edges to add to the graph
			\param cost_name The cost_type to add the edges to. If this cost type
			doesn't exist in the graph yet, then it will be created. If left blank
			or set to the default name, then the edges will be added to the default
			cost type.

			\pre 1) If adding edges to an alternate cost type, the edges must already have
			been added to the default graph.
			\pre 2) If adding an alternate edge to the graph, the graph must already be compressed

			\throws std::logic_error Trying to add an edge to an alternate cost type when it's not compressed
			\throws std::out_of_range Trying to add an edge to an alternate cost type when it hasn't already
			been added to the default graph2) If adding an alternate edge to the graph, the graph must already be compressed

		*/
		void AddEdges(const EdgeSet& edges, const std::string& cost_name = "");

		/*! \brief Add an array of edges to the graph.*/
		void AddEdges(const std::vector<EdgeSet>& edges, const std::string& cost_name = "");

		/*!
			\brief Get the edges of a specfic cost type
			\param cost_name The name of the cost to get edges for
			\returns An edge set for the edges of `cost_name`

			\exception HF::Exceptions::NoCost The cost at `cost_name` didn't exist in the graph
		*/
		std::vector<EdgeSet> GetEdges(const std::string& cost_name) const;

		/*! \brief Get an array of all cost names within this graph.

			\returns A list of all cost_types that exist within this graph (excluding
			the default cost array).
		*/
		std::vector<std::string> GetCostTypes() const;
		
		/*! 
		
			\brief get the cost from parent_id to child_id in the given cost_type.
			
			\param parent_id Node that's being traversed from.
			\param child_id Node that's being traversed to.
			\param cost_type Type of cost to get for this edge. If blank, the graph's
			default cost type will be used. 

			\returns The cost of traversing from `parent_id` to `child_id` for `cost_type`. 

			\pre cost_type must be the name of a cost that already exists in the graph,
			or blank. 
		*/
		float GetCost(int parent_id, int child_id, const std::string& cost_type = "") const;

		/*! \brief Add a set of intedges to the graph.
		
			\param edges An ordered vector of vectors in which each outer vector holds
			a vector of edges for the node at the ID of that index. For example the 
			vector at index 0 would hold a vector of intedges for the node at ID 0.a

			\param cost_type The type of cost to add these edges to. If this cost type
			does not yet exist, then it will be created.

			\pre 1) If adding edges to an alternate cost type, the edges must already have
			been added to the default graph.
			\pre 2) If adding an alternate edge to the graph, the graph must already be compressed

			\throws std::logic_error Trying to add an edge to an alternate cost type when it's not compressed
			\throws std::out_of_range Trying to add an edge to an alternate cost type when it hasn't already
			been added to the default graph2) If adding an alternate edge to the graph, the graph must already be compressed
		*/
		void Graph::AddEdges(const std::vector<std::vector<IntEdge>>& edges, const std::string& cost_type);


		/*! \brief Get the cost map for the graph
 

			\param cost_type Name of te cost to get the cost map for
		
		*/
		std::unordered_map<std::string, EdgeCostSet> Graph::GetCostMap(const std::string& cost_type = " ") const;


		/*!
			\brief Clear one or more cost arrays from the graph.
			
			\param cost_name Name of the cost array to clear. If equal to the default
			cost of this graph or empty string, will clear all existing cost arrays
			(except for the default)

			\throws NoCost if the costname specified not match either the default cost or any other
			cost type held by the graph.
		*/
		void ClearCostArrays(const std::string & cost_name = "");

		/*! \brief Generate edge costs from a set of node attributes. 
		
			\param attr_key Attribute to create a new cost set from.
			\param cost_string Name of the new cost set.
			\param dir Direction that the cost of the edge should be calculated in. For example
					   INCOMING will use the cost of the node being traveled to by the edge.

			\throws std::out_of_range if `node_attribute` could not be found. 

			\par Example
			\snippet tests\src\SpatialStructures.cpp EX_AttrsToStrings
			\snippet tests\src\SpatialStructures.cpp EX_AttrsToStrings2
			`0->1: 111.000000`
		*/
		void AttrToCost(
			const std::string & node_attribute,
			const std::string & cost_to_store_as,
			Direction consider = Direction::INCOMING);
		
	};
}
