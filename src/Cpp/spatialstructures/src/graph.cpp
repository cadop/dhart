///
/// \file	graph.cpp
/// \brief	Contains implementation for the <see cref="HF::SpatialStructures::Graph">Graph</see> class
///
/// \author	TBA
/// \date	06 Jun 2020

/// \todo Forward declares for eigen.

#include <Graph.h>
#include <algorithm>
#include <cmath>
#include <Constants.h>
#include <cassert>
#include <HFExceptions.h>
#include <numeric>
#include <iostream>
#include <charconv>

using namespace Eigen;
using std::vector;
using std::string;
using namespace HF::Exceptions;

namespace HF::SpatialStructures {

	/*! \brief Constructs a mapped CSR that's identical to `g`, with the values arrays of `ca`.
		
		\param g Edge matrix to map to
		\param ca EdgeCostSet to use the value arrays of

		\returns a TempMatrix mapped to the inner and outer indices of `g`, and the values array of `ca`
	
	*/
	inline TempMatrix CreateMappedCSR(const EdgeMatrix& g, const EdgeCostSet& ca) {

		const Map<const SparseMatrix<float, 1>> m(
			static_cast<int>(g.rows()),
			static_cast<int>(g.cols()),
			static_cast<int>(g.nonZeros()),
			static_cast<const int*>(g.outerIndexPtr()),
			static_cast<const int*>(g.innerIndexPtr()),
			//static_cast<const float*>(g.valuePtr()),
			static_cast<const float*>(ca.GetPtr()),
			static_cast<const int*>(g.innerNonZeroPtr())
		);

		return m;
	}

	/*!
	\summary Determines if a std::string is a floating-point value, i.e. '3.1415', '2.718', or is not -- i.e. '192.168.1.1', 'a_string'
	\param value The std::string to assess, as to whether it is a floating-point value or not
	\returns True, if value is determined to be a floating-point number, false otherwise

	\remarks A 'floating-point type', as defined by this function,
			 begins with zero or more integers in succession (but no alphas/symbols), then a decimal point ('.'),
			 followed by one or more integers in succession (absolutely no alphas/symbols).
			 Any input value that does not adhere to this specification will be denoted as a string type.

	\code

		std::string str_0 = "3.1415";
		std::string str_1 = ".1415";
		std::string str_2 = "192.168.1.1";
		std::string str_3 = "pthread.h";

		bool result_0 = is_floating_type(str_0);	// true
		bool result_1 = is_floating_type(str_1);	// true
		bool result_2 = is_floating_type(str_2);	// false
		bool result_3 = is_floating_type(str_3);	// false
	\endcode
*/
	bool is_floating_type(std::string value);

	inline bool is_floating_type(std::string value) {
		bool result = false;
		char* ptr = nullptr;

		std::strtof(value.c_str(), &ptr);

		if (*ptr == '\0') {
			char* next = nullptr;
			std::strtof(value.c_str(), &next);
			result = *next == '\0';
		}

		return result;
	}

	template<typename csr>
	inline int IMPL_GetCost(const csr & c, int parent, int child) {

		// Get the inner and outer index array pointers of the CSR
		const auto outer_index_ptr = c.outerIndexPtr();
		const auto inner_index_ptr = c.innerIndexPtr();

		return IMPL_ValueArrayIndex(parent, child, outer_index_ptr, inner_index_ptr);

	}

	float Graph::GetCost(int parent_id, int child_id, const std::string & cost_type) const{
		if (this->needs_compression)
			throw std::logic_error("Graph must be compressed to read costs using getcost");

		if (IsDefaultName(cost_type)) {
			const int index = IMPL_GetCost(this->edge_matrix, parent_id, child_id);
			if (index < 0 ) return NAN;
			else return this->edge_matrix.valuePtr()[index];
		}
		else
			return GetCostForSet(this->GetCostArray(cost_type), parent_id, child_id);
	}

	void Graph::ClearCostArrays(const std::string& cost_name)
	{
		// Delete them all if this is the default name
		if (this->IsDefaultName(cost_name))
			edge_cost_maps.clear();
		else if (!this->HasCostArray(cost_name))
			throw NoCost("Tried to delete a cost that doesn't already exist!");

		else
			edge_cost_maps.erase(cost_name);

	}



	inline float StringToFloat(const std::string & str_to_convert) {
		return std::stod(str_to_convert);
	}

	inline std::vector<float> ConvertStringsToFloat(const std::vector<string>& strings) {
		std::vector<float> out_floats(strings.size());

		for (int i = 0; i < out_floats.size(); i++)
			out_floats[i] = StringToFloat(strings[i]);
		return out_floats;
	}

	void Graph::AttrToCost(
		const std::string& node_attribute,
		const std::string & out_attribute, 
		Direction gen_using)
	{
		// Throw if we don't have this attribute
		if (!this->HasNodeAttribute(node_attribute))
			throw std::out_of_range("Node Attribute" + node_attribute + " doesn't exist in the graph!");
		
		// Get the costs for this attribute
		const auto& scores = ConvertStringsToFloat(this->GetNodeAttributes(node_attribute));

		// Iterate through all nodes in the graph
		for (const auto& parent : ordered_nodes) {

			// Get the subgraph for this node
			const auto subgraph = this->GetIntEdges(parent.id);
			
			// Iterate through every edge of this node
			for (const IntEdge& edge : subgraph)
			{
				// Calculate the cost for this node
				float cost = -1;
				switch (gen_using) {
				case Direction::INCOMING:
					cost= scores[edge.child];
					break;
				case Direction::BOTH:
					cost= scores[edge.child] + scores[parent.id];
					break;

				case Direction::OUTGOING:
					cost = scores[parent.id];
					break;
				}

				// Add it to the graph as an edge
				this->addEdge(parent.id, edge.child, cost, out_attribute);
			}
		}
	}

	int Graph::size() const { return ordered_nodes.size(); }

	int Graph::MaxID() const {
		int max_id = -1;

		for (const auto& node : ordered_nodes)
			max_id = std::max(node.id, max_id);

		return max_id;
	}

	int Graph::getID(const Node& node) const
	{
		// First check if we have this node
		if (hasKey(node))

			// If so, return its id
			return idmap.at(node);

		// If not, return -1
		else
			return -1;
	}

	EdgeCostSet& Graph::GetCostArray(const string& key)
	{
		// If this is the default key, then you're missing a check
		// somewhere. 
		assert(!IsDefaultName(key));

		if (!this->HasCostArray(key)) 
			throw NoCost("Tried to access a key that doesn't exist in the cost_arrays");

		// Return the cost map at key.
		return (edge_cost_maps.at(key));
	}

	bool Graph::HasCostArray(string key) const {
		return (edge_cost_maps.count(key) > 0);
	}

	EdgeCostSet& Graph::GetOrCreateCostType(const std::string& name)
	{
		// If this is the default name, then we must throw
		if (this->IsDefaultName(name))
			throw std::logic_error("Tried to create cost array with the graph's default name");

		// If we already have this cost array, return a reference to it
		if (this->HasCostArray(name))
			return this->GetCostArray(name);

		// Otherwise create a new one then return a reference to it. 
		else
			return this->CreateCostArray(name);
	}

	EdgeCostSet& Graph::CreateCostArray(const std::string& name)
	{
		// If you manage to throw this, something is wrong with
		// CheckCostArray
		assert(!this->HasCostArray(name));

		if (this->size() < 1)
			throw std::logic_error("Tried to add a cost to a graph with 0 nodes");

		// the size of the cost array must be large enough to hold all nonzeros
		// for this specific CSR
		const int nnz = edge_matrix.nonZeros();

		// Create a new cost set large enough to hold all non-zeros in the graph
		// then insert it into the hashmap.
		edge_cost_maps.insert({ name, EdgeCostSet(nnz) });

		// Get and return it
		return GetCostArray(name);
	}

	const EdgeCostSet& Graph::GetCostArray(const std::string& key) const
	{
		// Ensure that we throw our custom exception if this key doesn't exist
		if (!this->HasCostArray(key)) 
			throw NoCost(key);
		

		// Get the cost from the cost map
		return (edge_cost_maps.at(key));
	}

	bool Graph::IsDefaultName(const string& name) const
	{
		// Check if the name is empty "" or it matches our default
		// cost member.
		return (name.empty() || (name == this->default_cost));
	}

	int IMPL_ValueArrayIndex(
		int parent_id,
		int child_id,
		const int* outer_index_ptr,
		const int* inner_index_ptr
	) {
		// Get the bounds for our search
		const int search_start_index = outer_index_ptr[parent_id];
		const int search_end_index = outer_index_ptr[parent_id + 1];

		// Get a pointer to the start and end of our search bounds.
		const int* search_start_ptr = inner_index_ptr + search_start_index;
		const int* search_end_ptr = inner_index_ptr + search_end_index;

		// Use an iterator to find a pointer to the value in memory
		auto itr = std::find(search_start_ptr, search_end_ptr, child_id);

		// Return if we hit the end of our search bounds, indicating that the
		// edge doesn't exist.

		if (itr == search_end_ptr)
			return -1;
		// Find the distance between the pointer we found earlier and the
		// start of the values array
		else {
			int index = std::distance(inner_index_ptr, itr);
			return index;
		}

	}

	int Graph::FindValueArrayIndex(int parent_id, int child_id) const
	{
		// Get the inner and outer index array pointers of the CSR
		const auto outer_index_ptr = edge_matrix.outerIndexPtr();
		const auto inner_index_ptr = edge_matrix.innerIndexPtr();
			
		return IMPL_ValueArrayIndex(parent_id, child_id, outer_index_ptr, inner_index_ptr);
	}

	void Graph::InsertEdgeIntoCostSet(int parent_id, int child_id, float cost, EdgeCostSet& cost_set) {
		// Get the index of the edge between parent_id and child_id in the values array
		const int value_index = FindValueArrayIndex(parent_id, child_id);

		// If the index is < 0 that means this edge doesn't exist and we must throw
		if (value_index < 0)
			throw std::out_of_range("Tried to insert into edge that doesn't exist in default graph. ");

		// otherwise write this to the cost set.
		cost_set[value_index] = cost;
	}

	void Graph::InsertEdgesIntoCostSet(EdgeCostSet& cost_set, const std::vector<EdgeSet>& es)
	{
		// Iterate through every edgeset in es
		for (const auto& edge_set : es)
		{
			// Get the parent of this specific edge set
			const int parent_id = edge_set.parent;
			
			// Iterate through every edge in this edge set. 
			for (const auto& edge : edge_set.children) {

				// Get the child id and cost of this edge
				const int child_id = edge.child;
				const float cost = edge.weight;

				// Insert this edge into cost_set
				InsertEdgeIntoCostSet(parent_id, child_id, cost, cost_set);
			}
		}
	}

	CSRPtrs Graph::GetCSRPointers(const string& cost_type)
	{
		const bool default_cost = this->IsDefaultName(cost_type);

		// The graph must be compressed for this to work
		if (default_cost)
			Compress();

		// Construct CSRPtr with the required info from edge_matrix.
		CSRPtrs out_csr{
			static_cast<int>(edge_matrix.nonZeros()),
			static_cast<int>(edge_matrix.rows()),
			static_cast<int>(edge_matrix.cols()),

			edge_matrix.valuePtr(),
			edge_matrix.outerIndexPtr(),
			edge_matrix.innerIndexPtr()
		};

		// If this isn't the default cost, replace the pointer with
		// the pointer of the given cost array. 
		if (!default_cost)
		{
			EdgeCostSet& cost_set = this->GetCostArray(cost_type);
			out_csr.data = cost_set.GetPtr();
		}

		return out_csr;
	}

	Node Graph::NodeFromID(int id) const { return ordered_nodes.at(id); }

	std::vector<Node> Graph::Nodes() const {
		return ordered_nodes;
	}

	/*!
		\brief Retrieve the outgoing edges of a node. 


		\tparam csr A CSR that can be accessed by Eigen. 
		\param edge_matrix EdgeMatrix to use for the costs of each edge
		\param parent_id ID of the node to get incoming edges of
		\param g Graph containing edge_matrix. Used to get the Nodes of children ids.

		\returns An array of all edges going to parent_id with the cost used in cost_type. 

		\par Time Complexity O(n)
		Checks every other node for an edge to n.

		\remarks
		This can be slow since it needs to check if every other node in the graph has an edge to parent_id.
	*/
	template<typename csr>
	inline vector<Edge> IMPL_UndirectedEdges(const csr& edge_matrix, const int parent_id, const Graph * g) {

		// If N is not in the graph, return an empty array.
		const int node_id = parent_id;
		if (node_id < 0) return vector<Edge>();

		// Get the directed edges for this node from calling operator[]
		vector<Edge> out_edges;

		// Iterate through every other node
		for (int i = 0; i < edge_matrix.rows(); i++) {

				// Don't look in this node's edge array
				if (i == node_id) continue;

			// See if there's an edge between I and node_id
			if (edge_matrix.coeff(i, node_id) != 0) {

				// if so, get its cost
				float cost = edge_matrix.coeff(i, node_id);
				
				// Get the node belonging to the child's id
				Node child_node = g->NodeFromID(i);

				// Construct a new edge and push it back into ot_edges
				Edge edge{ child_node, cost };
				out_edges.push_back(edge);
			}
		}
		return out_edges;
	}

	vector<Edge> Graph::GetUndirectedEdges(const Node& n, const std::string& cost_type) const {
		// call GetEdgesForNode, since it already handles this. 
		return this->GetEdgesForNode(getID(n),  true, cost_type);
	}

	std::vector<EdgeSet> Graph::GetEdges() const
	{

		// Throw if we're not compressed since this is a const function and compressing the graph
		// will make it non-const
		if (this->needs_compression)
			throw std::exception("The graph must be compressed!");

		// Preallocate an array of edge sets
		vector<EdgeSet> out_edges(this->size());

		// Iterate through every row in the csr
		for (int node_index = 0; node_index < this->size(); ++node_index) {
			const int node_id = node_index;

			auto& edgeset = out_edges[node_id];
			edgeset.parent = node_id;

			// Iterate every column in the row.
			for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, node_index); it; ++it)
			{
				// Add to array of edgesets
				float cost = it.value();
				int child =it.col();
				edgeset.children.push_back(IntEdge{ child, cost });
			}
		}
		return out_edges;
	}

	std::vector<IntEdge> Graph::GetIntEdges(int parent) const
	{
		if (parent > this->MaxID()) return std::vector<IntEdge>();

		std::vector<IntEdge> intedges;
		for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, parent); it; ++it)
		{
			// Add to array of edgesets
			float cost = it.value();
			int child = it.col();
			intedges.push_back(IntEdge{ child, cost });
		}
		return intedges;
	}

	/// <summary> Aggregate new_value into out_total using the method specified in agg_type. </summary>
	/// <param name="out_total"> Total to aggregate new_value into. </param>
	/// <param name="new_value"> Value to aggregate into out_total. </param>
	/// <param name="agg_type"> Aggregation method to use. </param>
	/// <param name="count"> Number of elements. Incremented with each call. </param>
	/*!

		\exception std::out_of_range agg_type doesn't exist in COST_AGGREgATE

		\remarks
		This is similar to the function from ViewAnalysis but will increment
		count with each call.

		\see COST_AGGREGATE for more information on each aggregate type.

	*/
	inline void Aggregate(float& out_total, float new_value, const COST_AGGREGATE agg_type, int & count)
	{
		switch (agg_type) {
		case COST_AGGREGATE::COUNT:

			// Only increment count if newvalue is equal to zero.
			// In the case that out_total is zero and count is greater than zero,
			// multiple new elements have been added, and we must not increment
			if (new_value > 0 && !(out_total == 0 && count > 0)) 
				count++; 
			out_total = count;
			break;
		case COST_AGGREGATE::SUM:
			out_total += new_value;
			break;
		case COST_AGGREGATE::AVERAGE: {
			// If this is zero this function won't work. 
			if (count == 0) count = 1;

			out_total = out_total + (new_value - out_total) / static_cast<float>(count);
			count++;
			break;
		}
		default:
			throw std::out_of_range("Unimplemented aggregation type");
			break;
		}
		assert((out_total == 0 || isnormal(out_total)));
		return;
	}

	/*!
		\brief  Summarize the costs of every outgoing edge for every node in the graph.
	
		\tparam csr A Valid Eigen CSR matrix. 
		\param agg_type Type of aggregation to use.
		\param num_nodes the number of nodes in the graph.
		\param directed If true, include both incoming and outgoing edges for calculating a node's score.
		\param csr_matrix the CSR matrix to use for the costs of every edge in the graph

		\returns An ordered list of scores for agg_type on each node in the graph.

		\remarks Useful for getting scores from the VisibilityGraph.

		\par Time Complexity
		If undirected: `O(k)` where k is the total number of edges in the graph.\n
		If directed: `O(n)` where n is the total number of nodes in the graph.

		\see COST_AGGREGATE to see a list of supported aggregation types.
		\code
			// be sure to #include "graph.h"
	*/

	template<typename csr>
	std::vector<float> Impl_AggregateGraph(COST_AGGREGATE agg_type, int num_nodes, bool directed, const csr & edge_matrix) {
		
		// Create an array of floats filled with zeros.
		vector<float> out_costs(num_nodes, 0);

		// If directed is true, then we only need the values in a node's row to calculate it's score.
		if (directed)
			for (int k = 0; k < num_nodes; ++k) {

				// Sum all values in the row for node k
				const float sum = edge_matrix.row(k).sum();

				// Get the count of non_zeros for node k
				int count = edge_matrix.row(k).nonZeros();

				// Run aggregate once with the sum and count of the non-zeros in row k
				Aggregate(out_costs[k], sum, agg_type, count);

			}

		// Based on implementation from Eigen's sparse matrix tutorial: Iterating over the nonzero
		// coefficents: https://eigen.tuxfamily.org/dox/group__TutorialSparse.html If directed is
		// false, then every edge needs to be iterated through since edges that go to a node also
		// count towards its score.
		else {
			// We need to keep track of count per node as well for this algorithm
			vector<int> count(num_nodes, 0);

			// Iterate through every node in the graph
			for (int k = 0; k < num_nodes; ++k) {

				// Iterate through every edge for the node at column k
				for (csr::InnerIterator it(edge_matrix, k); it; ++it)
				{
					// Get values from the iterator for this row/col.
					float cost = it.value();
					int child = it.col();
					int parent = it.row();

					// Aggregate costs for both the parent and the child.
					Aggregate(out_costs[parent], cost, agg_type, count[parent]);
					Aggregate(out_costs[child], cost, agg_type, count[child]);
				}
			}
		}
		return out_costs;
	}

	std::vector<float> Graph::AggregateGraph(COST_AGGREGATE agg_type, bool directed, const string& cost_type) const
	{
		// This won't work if the graph isn't compressed.
		if (this->needs_compression) throw std::exception("The graph must be compressed!");
	
		// Determine if this is the default cost. 
		const bool default_cost = this->IsDefaultName(cost_type);

		// If this isn't the default cost, map to the cost array. 
		if (!default_cost) {

			// Get the cost array and create a mapped CSR from it
			const EdgeCostSet & cost_array = this->GetCostArray(cost_type);
			const TempMatrix cost_matrix= CreateMappedCSR(this->edge_matrix, cost_array);

			// Call the implementation of aggregate graph with this template
			return Impl_AggregateGraph(agg_type, this->size(), directed, cost_matrix);
		}
		else
			// If this is the default cost, just call AggregateGraph with the default CSR
			return (Impl_AggregateGraph(agg_type, this->size(), directed, this->edge_matrix));
	}

	const std::vector<Edge> Graph::operator[](const Node& n) const
	{
		return GetEdgesForNode(this->getID(n));
	}

	void Graph::InsertOrUpdateEdge(int parent_id, int child_id, float score, const string& cost_type) {

		// If this is the default graph, we don't need to worry aobut 
		if (IsDefaultName(cost_type)) {
			if (this->needs_compression)
				TripletsAddOrUpdateEdge(parent_id, child_id, score);
			else
				CSRAddOrUpdateEdge(parent_id, child_id, score);
		}
		else
			// Can't add to an alternate cost if the graph isn't compressed
			if (this->needs_compression)
				throw std::logic_error("Tried to add an edge to an alternate cost type while uncompressed!");
			
			// Add the cost to the cost type pointed to by cost_type
			else
			{
				// Determine whether or not this cost type exists
				const bool new_cost_array = !HasCostArray(cost_type);
				
				// Create a new cost array or get a reference to an existing one
				auto& cost_set = GetOrCreateCostType(cost_type);

				// Try to add an edge. We need to ensure we maintain the graph's invariants
				// in the case that this throws.
				try {
					InsertEdgeIntoCostSet(parent_id, child_id, score, cost_set);
				}
				catch(const std::out_of_range & e){

					// If this is a new cost array, and we threw that would leave it in an invalid
					// state which will cause problems later. Remove this half baked cost array 
					// before returning from this exception.
					if (new_cost_array)
						this->ClearCostArrays(cost_type);
					
					// Rethrow here so callers know we failed to complete
					throw e;
				}
			}
	}

	float Graph::GetCostForSet(const EdgeCostSet & set, int parent_id, int child_id) const
	{
		// Get the index for the cost of the edge between parent_id and child_id
		const int index = FindValueArrayIndex(parent_id, child_id);

		// If the index is <0, the edge doesn't exist.
		// We represent non-existant edges as NAN
		if (index < 0) 
			return NAN;

		// Othrwise, return the value at this index in set
		else
			return set[index];
	}

	vector<Edge> Graph::GetEdgesForNode(int parent_id, bool undirected, const string & cost_type) const
	{
		// Get the row of this node
		const int row = parent_id;
		const bool default_name = this->IsDefaultName(cost_type);

		std::vector<Edge> outgoing_edges;

		// Get all of the outgoing edges by iterating through the array if it's the default
		if (default_name) {
			for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, row); it; ++it) {
				const auto col = it.col();
				float value = it.value();

				outgoing_edges.emplace_back(Edge(NodeFromID(col), value));
			}
		}
		// If using a custom cost, use our own indexing function instead of getting the value from the iterator
		else {
			const auto & cost_set = this->GetCostArray(cost_type);
			std::vector<Edge> out_edges;
			for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, row); it; ++it) {
				const auto col = it.col();
				auto value = this->GetCostForSet(cost_set, row, col);

				outgoing_edges.emplace_back(Edge(NodeFromID(col), value));
			}
		}
	
		// If this is undirected, we'll need to get a list of incoming edges as well.
		if (undirected) {
			vector<Edge> incoming_edges;
			
			// Default name uses the default edge_matrix member
			if (default_name)
				incoming_edges = IMPL_UndirectedEdges(this->edge_matrix, parent_id, this);
		
			// If we're using a custom type, map a CSR to it and use that instead
			else 
				incoming_edges = IMPL_UndirectedEdges(this->MapCostMatrix(cost_type), parent_id, this);

			// Combine the incoming and outgoing edges
			vector<Edge> incoming_and_outgoing(incoming_edges.size() + outgoing_edges.size());

			// Move the contents of outgoing and incoming into this output array, then return it
			std::move(outgoing_edges.begin(), outgoing_edges.end(), incoming_and_outgoing.begin());
			std::move(incoming_edges.begin(), incoming_edges.end(), incoming_and_outgoing.begin() + outgoing_edges.size());
			
			return incoming_and_outgoing;
		}
		else 
			return outgoing_edges;

	}

	TempMatrix Graph::MapCostMatrix(const std::string& cost_type) const
	{
		const EdgeCostSet& cost_array = this->GetCostArray(cost_type);
		const TempMatrix cost_matrix = CreateMappedCSR(this->edge_matrix, cost_array);
		return cost_matrix;
	}

	bool Graph::HasNodeAttribute(const std::string& key) const
	{
		return this->node_attr_map.count(key) > 0;
	}

	void Graph::addEdge(const Node& parent, const Node& child, float score, const string & cost_type)
	{
		// ![GetOrAssignID_Node]
		
		// Get parent/child ids
		int parent_id = getOrAssignID(parent);
		int child_id = getOrAssignID(child);
	
		// If this is already compressed, update the CSR, otherwise add it to the list of triplets.
		InsertOrUpdateEdge(parent_id, child_id, score, cost_type);
		// ![GetOrAssignID_Node]
	}

	void Graph::addEdge(int parent_id, int child_id, float score, const string & cost_type)
	{
		// ![GetOrAssignID_int]

		// Store these Ids in the hashmap if they don't exist already.
		getOrAssignID(child_id);
		getOrAssignID(parent_id);

		InsertOrUpdateEdge(parent_id, child_id, score, cost_type);
		
		// ![GetOrAssignID_int]
	}

	bool Graph::checkForEdge(int parent, int child) const {
		// ![CheckForEdge]
		
		// Get the index for parent and child
		const int parent_index = parent;
		const int child_index = child;

		// If the parent is not even in the graph, or the graph doesn't have any zeros, return early. 
		// Calling the iterator in both of these cases is undefined behavior and should be avoided
		if (edge_matrix.nonZeros() <= 0 || edge_matrix.rows() <= parent) return false;
	
		// Iterate through parent's row to see if it has child.
		for (EdgeMatrix::InnerIterator it(edge_matrix, parent_index); it; ++it) {
			if (it.col() == child_index) return true;
		}
		
		// If we've gotten to this point, then the child doesn't exist in parent's row
		return false;
		// ![CheckForEdge]
	}

	void Graph::CSRAddOrUpdateEdge(int parent_id, int child_id, float cost)
	{
		const int parent_index = parent_id;
		const int child_index = child_id;

		// Use coeffref if the cost already exists to avoid duplicate allocations
		if (HasEdge(parent_index, child_index))
			edge_matrix.coeffRef(parent_index, child_index) = cost;
		else {
			// Reallocate if we must, then insert. 
			ResizeIfNeeded();
			edge_matrix.insert(parent_index, child_index) = cost;
		}
	}

	void Graph::TripletsAddOrUpdateEdge(int parent_id, int child_id, float cost) {

		// Add this edge to the list of triplets.
		triplets.emplace_back(Eigen::Triplet<float>(parent_id, child_id, cost));
	}

	void Graph::ResizeIfNeeded()
	{
		int num_nodes = -1;

		// Find the maximum ID in the graph if using Int Nodes
		if (using_int_nodes)
			num_nodes = MaxID();
		else
			num_nodes = size();

		// You need one more row/col than capacity
		num_nodes += 1;

		// If the edge matrix can't fit this many nodes, expand it.
		if (num_nodes > edge_matrix.rows())

			// Conservative resize preserves all of the values in the graph
			edge_matrix.conservativeResize(num_nodes, num_nodes);

		assert(num_nodes <= edge_matrix.rows() && num_nodes <= edge_matrix.cols());
	}

	inline bool Graph::hasKey(int id) const
	{
		// The only way to search for an ID now is brute force
		for (int i = 0; i < ordered_nodes.size(); i++)
			if (ordered_nodes[i].id == id) return true;
		
		return false;
	}

	bool Graph::HasEdge(int parent, int child, bool undirected, const string & cost_type) const {
		// Check if these IDS even exist in the graph.
		if (!this->hasKey(parent) || !this->hasKey(child)) return false;

		// If this is the default name, check for the cost in the base CSR
		else if (IsDefaultName(cost_type))
			return (checkForEdge(parent, child) || ( undirected && checkForEdge(child, parent) ) );
		
		// If this isn't the default name, then get it from the cost set it's asking for
		else {
			// If this doesn't have the cost array defined before, return
			if (!this->HasCostArray(cost_type)) {
				return false;
			}
			// Otherwise get the cost array and try to find it. 
			const auto& cost_array = GetCostArray(cost_type);
			const auto cost = GetCostForSet(cost_array, parent, child);

			// If undirected is specified, call this fucntion again from parent to child
			// with undirected set to false.
			bool check_undirected = undirected ? HasEdge(child, parent, false, cost_type) : false;

			return (!isnan(cost) || check_undirected);
		}
	}

	bool Graph::HasEdge(const Node& parent, const Node& child, const bool undirected, const string cost_type) const {

		// Throw if the graph isn't compresesed.
		if (!edge_matrix.isCompressed())
			throw std::exception("Can't get this for uncompressed matrix!");

		// Return early if parent or child don't exist in the graph
		if (!hasKey(parent) || !hasKey(child)) return false;

		// Get the id of both parent and child.
		int parent_id = getID(parent);
		int child_id = getID(child);
		
		// Call integer overload.
		return HasEdge(parent_id, child_id, undirected);
	}

	inline int Graph::getOrAssignID(const Node& input_node)
	{
		// If it's already in the hashmap, then just return the existing ID
		if (hasKey(input_node))
			return getID(input_node);

		else {
			// Set the id in the hashmap, and add the node to nodes
			idmap[input_node] = next_id;
			ordered_nodes.push_back(input_node);
			
			ordered_nodes.back().id = next_id;
			// Increment next_id
			next_id++;

			// Return the node's new ID
			return next_id - 1;
		}
	}

	int Graph::getOrAssignID(int input_int)
	{
		// If this ID isn't in our list, add it, and create an empty node in ordered
		// nodes to take up space.
		if (!hasKey(input_int))
		{
			// If we're adding a new key that's an integer
			// ordered_nodes is no longer gauranteed to be
			// in order and we must tell the graph this.
			this->using_int_nodes = true;

			// Add an empty node to ordered_nodes
			ordered_nodes.push_back(Node());
		
			// Set the id of the empty node
			ordered_nodes.back().id = input_int;

			this->next_id = std::max(input_int, this->next_id);
		}

		return input_int;
	}

	Graph::Graph(
		const vector<vector<int>>& edges,
		const vector<vector<float>> & distances,
		const vector<Node> & Nodes,
		const std::string & default_cost
	) {

		this->default_cost = default_cost;
		
		// Generate an array with the size of every column from the size of the edges array
		assert(edges.size() == distances.size());
		vector<int> sizes(edges.size());
		for (int i = 0; i < edges.size(); i++)
			sizes[i] = edges[i].size();

		// Create the graph then reserve these sizes
		edge_matrix.resize(edges.size(),  edges.size());
		edge_matrix.reserve(sizes);

		// Iterate through every node in nodes
		for (int row_num = 0; row_num < edges.size(); row_num++)
		{
			//Add this node to our dictionary/ordered_node list
			getOrAssignID(Nodes[row_num]);

			// Get the row out of the edges array
			const auto & row = edges[row_num];
			for (int i = 0; i < row.size(); i++) {

				// Get the column and distance from the row and distance array
				float dist = distances[row_num][i];
				int col_num = row[i];

				// Insert it into the edge matrix.
				edge_matrix.insert(row_num, col_num) = dist;
			}
		}

		// Compress the edge matrix to finalize it.
		edge_matrix.makeCompressed();
		//assert(edge_matrix.nonZeros() > 0);
		needs_compression = false;
	}

	Graph::Graph(const std::string & default_cost_name)
	{
		// Assign default cost type, and create an edge matrix.
		this->default_cost = default_cost_name;
	}

	bool Graph::HasEdge(const std::array<float, 3>& parent, const std::array<float, 3>& child, bool undirected) const {
		// Just convert it and pass it over to the other HasEdge
		const Node p(parent);
		const Node c(child);
		return HasEdge(p, c, undirected);
	}

	bool Graph::hasKey(const Node& n) const { return (idmap.count(n) > 0); }

	std::vector<std::array<float, 3>> Graph::NodesAsFloat3() const
	{
		// Get a constant reference to ordered_nodes to maintain const-ness of this function.
		const auto & N= ordered_nodes;
		int n = N.size();

		// Preallocate output array
		std::vector <std::array<float, 3> > out_nodes(n);

		// Assign x,y,z for every node in nodes.
		for (int i = 0; i < n; i++) {
			out_nodes[i][0] = N[i].x;
			out_nodes[i][1] = N[i].y;
			out_nodes[i][2] = N[i].z;
		}

		return out_nodes;
	}

	void Graph::Compress() {

		// Only do this if the graph needs compression.
		if (needs_compression) {

			// If this has cost arrays then we never should have come here
			assert(!this->has_cost_arrays); 

			// Note that the matrix must have atleast one extra row/column
			int array_size = this->size() + 1;
			
			// Resize the edge matrix
			ResizeIfNeeded();

			// Set the edge matrix from triplets.
			edge_matrix.setFromTriplets(triplets.begin(), triplets.end());

			// Mark this graph as not requiring compression
			needs_compression = false;
		}
	}

	void Graph::Clear() {
		edge_matrix.setZero();
		edge_matrix.data().squeeze();
		triplets.clear();
		needs_compression = true;

		// Other graph representations should be cleared too
		ordered_nodes.clear();
		idmap.clear();

		// Clear all cost arrays
		// Clear all cost arrays.
		for (auto& cost_map : edge_cost_maps)
			cost_map.second.Clear();
	}
	
	void Graph::AddEdges(const vector<EdgeSet>& edges, const string& cost_name)
	{
		for (const auto& set : edges)
			AddEdges(set, cost_name);
	}


	void Graph::AddEdges(const vector<vector<IntEdge>> & edges, const std::string & cost_type) {
		// Each outer vector represents a parent;
		for (int parent = 0; parent < edges.size(); parent++)
		{
			const auto& outgoing_edges = edges[parent];
			for (const auto& edge : outgoing_edges)
				this->addEdge(parent, edge.child, edge.weight, cost_type);
		}
	}

	void Graph::AddEdges(const EdgeSet & edges, const string& cost_name) {
		const auto parent = edges.parent;
		
		if (this->IsDefaultName(cost_name))
			for (const auto& edge : edges.children)
				this->addEdge(parent, edge.child, edge.weight);
		else
			for (const auto& edge : edges.children)
				this->addEdge(parent, edge.child, edge.weight, cost_name);
	}


	vector<EdgeSet> Graph::GetEdges(const string & cost_name) const
	{
		// Call the other function if they're asking for the default.
		if (this->IsDefaultName(cost_name))
			return GetEdges();

		// Preallocate an array of edge sets
		vector<EdgeSet> out_edges(this->size());

		// Get the asked for cost set
		const auto& cost_set = this->GetCostArray(cost_name);
		
		// Iterate through every row in the csr
		for (int parent_index = 0; parent_index < this->size(); ++parent_index) {

			auto& edgeset = out_edges[parent_index];
			edgeset.parent = parent_index;

			// Iterate every column in the row.
			for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, parent_index); it; ++it)
			{
				// Add to array of edgesets
				int child_index = it.col();
				
				int cost_index = this->FindValueArrayIndex(parent_index, child_index);
				float cost = cost_set[cost_index];
				
				edgeset.children.push_back(IntEdge{ child_index, cost });
			}
		}
		return out_edges;
	}

	vector<std::string> Graph::GetCostTypes() const
	{
		vector<string> cost_types;
		
		// Iterate through the dictionary, adding the key of each
		// cost type into the output array
		for (const auto & it : this->edge_cost_maps)
			cost_types.push_back(it.first);

		return cost_types;
	}

	std::vector<Node> Graph::GetChildren(const Node& n) const {
		std::vector<Node> children;

		auto edges = (*this)[n];

		for (auto e : edges) {
			children.push_back(e.child);
		}

		return children;
	}

	std::vector<Node> Graph::GetChildren(const int parent_id) const {
		return GetChildren(NodeFromID(parent_id));
	}

	Subgraph Graph::GetSubgraph(const Node & parent_node, const string & cost_type) const {
		return this->GetSubgraph(this->getID(parent_node), cost_type);
	}

	Subgraph Graph::GetSubgraph(int parent_id, const string & cost_type) const {
		Node parent_node = ordered_nodes[parent_id];
		return Subgraph{ parent_node, this->GetEdgesForNode(parent_id, false, cost_type) };
	}

	void Graph::AddNodeAttribute(int id, std::string attribute, std::string score) {
		// Check if this id belongs to any node in the graph
		if (id > this->MaxID()) return;

		/* // requires #include <algorithm>, but not working?
		std::string lower_cased =
			std::transform(attribute.begin(), attribute.end(),
				[](unsigned char c) { return std::tolower(c); }
		);
		*/
		std::string lower_cased = attribute;

		// Retrieve an iterator to the [node attribute : NodeAttributeValueMap]
		// that corresponds with attribute
		auto node_attr_map_it = node_attr_map.find(lower_cased);

		if (node_attr_map_it == node_attr_map.end()) {
			// If the attribute type does not exist...create it.
			node_attr_map[lower_cased] = NodeAttributeValueMap();

			// Update this iterator so it can be used in the next code block
			node_attr_map_it = node_attr_map.find(lower_cased);
		}

		// We now have the NodeAttributeValueMap for the desired attribute.
		// A NodeAttributeValueMap stores buckets of [node id : node attribute value as string]
		NodeAttributeValueMap& node_attr_value_map = node_attr_map_it->second;

		// Need to see if id exists as a key within node_attr_value_map
		// This will give us the position of a bucket containing:
		// [node id : node attribute value as string]
		auto node_attr_value_map_it = node_attr_value_map.find(id);

		if (node_attr_value_map_it == node_attr_value_map.end()) {
			// If the node id provided does not exist in the value map...add it.
			node_attr_value_map[id] = score;

			// Update this iterator so it can be used in the next code block
			node_attr_value_map_it = node_attr_value_map.find(id);
		}

		// Should be the same as the id parameter passed in.
		const int found_id = node_attr_value_map_it->first;

		// Will be used to assess whether it is floating point, or not
		std::string found_attr_value = node_attr_value_map_it->second;

		// Let's determine the data type of score:
		bool score_is_floating_pt = is_floating_type(score);

		// Let's determine the data type of found_attr_value:
		bool attr_is_floating_pt = is_floating_type(found_attr_value);

		/*
			Need to determine if found_attr_value is
				- a string
				- a floating point value

			and if the data type for score matches that of found_attr_value
		*/
		if (attr_is_floating_pt) {
			// if the current attribute value is floating point...
			if (score_is_floating_pt) {
				// Ok - data type matched.
				node_attr_value_map_it->second = score;
			}
			else {
				// error?
			}
		}
		else {
			// if the current attribute value is not floating point...
			if (score_is_floating_pt) {
				// error?
			}
			else {
				// Ok - data type matched
				node_attr_value_map_it->second = score;
			}
		}
	}

	void Graph::AddNodeAttributes(
		const vector<int> & id, 
		const string & name,
		const vector<string> & scores
	) {
		// If size of id container and size of scores container are not in alignment,
		// throw, since our precondition was violated
		if (id.size() != scores.size())
			throw std::logic_error("Tried to pass id and string arrays that are different lengths");

		auto scores_iterator = scores.begin();

		for (int node_id : id) {

			// We can call AddNodeAttribute for each node_id in id.
			// If the attribute type name does not exist,
			// it will be created with the first invocation of AddNodeAttribute.
			AddNodeAttribute(node_id, name, *(scores_iterator++));
		}
	}

	vector<string> Graph::GetNodeAttributes(string attribute) const {

		// Return an empty array if this attribute doesn't exist
		if (node_attr_map.count(attribute) < 1) return vector<string>();
	
		// Get the attribute map for attribute now that we know it exists
		const auto& attr_map = node_attr_map.at(attribute);

		// Preallocate output array with empty strings. We'll only be modifying
		// the indexes that have scores assigned to them, and leaving the rest
		// as empty strings
		const int num_nodes = ordered_nodes.size();
		vector<string> out_attributes(ordered_nodes.size(), "");

		// Iterate through attribute map to assign scores for the nodes
		// that have them
		for (const auto& it : attr_map)
		{
			// Get the id and score of this element in the hashmap
			const int id = it.first;
			const string& score = it.second;

			// Copy it to the index of the node's ID in our output array
			out_attributes[id] = score;
		}

		// Return all found attributes
		return out_attributes;
	}

	void Graph::ClearNodeAttributes(std::string name) {
		/* // requires #include <algorithm>, but not working?
		std::string lower_cased =
			std::transform(attribute.begin(), attribute.end(),
				[](unsigned char c) { return std::tolower(c); }
		);
		*/
		//	std::string lower_cased = name;

		// Check if we have this name in our dictionary. Only try to delete
		// it if it already exists.
		if (node_attr_map.count(name) > 0) {

			// Erase the key from the dictionary, implicitly freeing the object
			// it contains.
			node_attr_map.erase(name);
		}

	}
}
