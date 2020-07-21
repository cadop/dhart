///
/// \file	graph.cpp
/// \brief	Contains implementation for the <see cref="HF::SpatialStructures::Graph">Graph</see> class
///
/// \author	TBA
/// \date	06 Jun 2020

/// \todo Forward declares for eigen.

#include <Graph.h>
#include <algorithm>
#include <math.h>
#include <Constants.h>
#include <assert.h>
#include <HFExceptions.h>

using namespace Eigen;
using std::vector;
using std::string;
using namespace HF::Exceptions;

namespace HF::SpatialStructures {

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

	int Graph::size() const { return ordered_nodes.size(); }

	int Graph::getID(const Node& node) const
	{
		if (hasKey(node))
			return idmap.at(node);
		else
			return -1;
	}

	int Graph::GetIDFromIndex(int index) const {
		throw HF::Exceptions::NotImplemented();
	}

	EdgeCostSet & Graph::GetCostArray(const string & key) 
	{
		return const_cast<EdgeCostSet &>(GetCostArray(key));
	}

	bool Graph::HasCostArray(string key) {
		return (edge_cost_maps.count(key) > 0);
	}

	EdgeCostSet & Graph::GetOrCreateCostType(const std::string& name)
	{
		assert(!this->IsDefaultName(name));

		if(this->HasCostArray(name))
			return this->GetCostArray(name);
		else
			return this->CreateCostArray(name);
	}

	EdgeCostSet & Graph::CreateCostArray(const std::string & name)
	{
		assert(!this->HasCostArray(name));

		// Create a new edge cost set and insert it into the hashmap. Return the edge matrix.
		edge_cost_maps.insert({ name, EdgeCostSet(this->size()) });

		// Get and return it
		return GetCostArray(name);
	}

	const EdgeCostSet& Graph::GetCostArray(const std::string& key) const
	{
		return (edge_cost_maps.at(key));
	}

	const EdgeCostSet& Graph::GetDefaultCostArray() const
	{
		return this->GetCostArray(this->default_cost);
	}

	bool Graph::IsDefaultName(const std::string& name)
	{
		return (name.empty());
	}

	int Graph::ValueArrayIndex(int parent_id, int child_id) const
	{
		const auto outer_index_ptr = edge_matrix.outerIndexPtr();
		const auto inner_index_ptr = edge_matrix.innerIndexPtr();

		return inner_index_ptr[outer_index_ptr[parent_id]];
	}

	void Graph::InsertEdgeIntoCostSet(int parent_id, int child_id, float cost, EdgeCostSet& cost_set) {
		const int value_index = ValueArrayIndex(parent_id, child_id);
		cost_set[value_index] = cost;
	}

	void Graph::InsertEdgesIntoCostSet(EdgeCostSet& cost_set, const std::vector<EdgeSet>& es)
	{
		for (const auto & edge_set : es)
		{
			const int parent_id = edge_set.parent;
			for (const auto& edge : edge_set.children) {
				
				const int child_id = edge.child;
				const int cost = edge.weight;
				
				InsertEdgeIntoCostSet(parent_id, child_id, cost, cost_set);
			}
		}
	}

	CSRPtrs Graph::GetCSRPointers(const std::string & cost_type)
	{
		// The graph must be compressed for this to work
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

		return out_csr;
	}

	Node Graph::NodeFromID(int id) const { return ordered_nodes.at(id);}

	std::vector<Node> Graph::Nodes() const {
		return ordered_nodes;
	}

	vector<Edge> Graph::GetUndirectedEdges(const Node & n) const {

		// Get the ID of n
		int node_id = getID(n);

		// If N is not in the graph, return an empty array.
		if (node_id < 0) return vector<Edge>();

		// Get the directed edges for this node from calling operator[]
		vector<Edge> out_edges = (*this)[n];

		// Iterate through every other node
		for (int i = 0; i < size(); i++) {

			// Don't look in this node's edge array
			if (i == node_id) continue;

			// See if this edge
			if (HasEdge(i, node_id)) {
				float cost = edge_matrix.coeff(i, node_id);
				Node child_node = NodeFromID(i);
				Edge edge(child_node, cost);

				out_edges.push_back(edge);
			}
		}
		return out_edges;
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
			if (new_value > 0) count++;
			out_total = count;
			break;
		case COST_AGGREGATE::SUM:
				out_total += new_value;
			break;
		case COST_AGGREGATE::AVERAGE: {
			// Ensure the number of elements isn't zero.
			int n = std::max(count - 1, 1);

			out_total = (n * (out_total)+new_value) / std::max(count, 1);
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

	std::vector<float> Graph::AggregateGraph(COST_AGGREGATE agg_type, bool directed) const
	{
		// This won't work if the graph isn't compressed.
		if (this->needs_compression) throw std::exception("The graph must be compressed!");

		// Create an array of floats filled with zeros.
		vector<float> out_costs(this->size(), 0);

		// If directed is true, then we only need the values in a node's row to calculate it's score.
		if (directed)
			for (int k = 0; k < this->size(); ++k) {
				
				// Sum all values in the row for node k
				float sum = edge_matrix.row(k).sum();

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
			vector<int> count(this->size(), 0);

			// Iterate through every node in the graph
			for (int k = 0; k < this->size(); ++k) {

				// Iterate through every edge for the node at column k
				for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, k); it; ++it)
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

	const std::vector<Edge> Graph::operator[](const Node& n) const
	{

		// Get the row of this node
		const int row = GetIndex(n);

		// Iterate through the row of n and add add all values to the output array
		std::vector<Edge> out_edges;
		for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, row); it; ++it) {
			auto value = it.value();
			auto col = it.col();

			out_edges.emplace_back(Edge(NodeFromID(col), value));
		}

		return out_edges;
	}

	void Graph::InsertOrUpdateEdge(int parent_id, int child_id, float score, const string& cost_type) {
		if (IsDefaultName(cost_type)) {
			if (this->needs_compression)
				TripletsAddOrUpdateEdge(parent_id, child_id, score);
			else
				CSRAddOrUpdateEdge(parent_id, child_id, score);
		}
		else
			InsertEdgeIntoCostSet(parent_id, child_id, score, GetOrCreateCostType(cost_type));
	}

	void Graph::addEdge(const Node& parent, const Node& child, float score, const string & cost_type)
	{
		// ![GetOrAssignID_Node]
		
		// Get parent/child ids
		int parent_id = getOrAssignID(parent);
		int child_id = getOrAssignID(child);
	
		// If this is already compressed, update the CSR, otherwise add it to the list of triplets.
		if (!needs_compression)
			CSRAddOrUpdateEdge(parent_id, child_id, score);
		else
			TripletsAddOrUpdateEdge(parent_id, child_id, score);

		// ![GetOrAssignID_Node]
	}

	void Graph::addEdge(int parent_id, int child_id, float score, const string & cost_type)
	{
		// ![GetOrAssignID_int]

		// Store these Ids in the hashmap if they don't exist already.
		getOrAssignID(child_id);
		getOrAssignID(parent_id);

		// If this is already compressed, update the CSR, otherwise add it to the list of triplets.
		if (!needs_compression)
			CSRAddOrUpdateEdge(parent_id, child_id, score);
		else
			TripletsAddOrUpdateEdge(parent_id, child_id, score);

		// ![GetOrAssignID_int]
	}

	bool Graph::checkForEdge(int parent, int child) const {
		// ![CheckForEdge]
		
		// Get the index for parent and child
		const int parent_index = parent;
		const int child_index = child;

		// Iterate through parent's row to see if it has child.
		for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, parent_index); it; ++it)
			if (it.col() == child_index) return true;
		
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
		const int parent_index = parent_id;
		const int child_index = child_id;

		triplets.emplace_back(Eigen::Triplet<float>(parent_index, child_index, cost));
	}

	void Graph::ResizeIfNeeded()
	{
		const int num_nodes = size()+1; // You need 1 more column/row than max capacity. 

		if (num_nodes > edge_matrix.rows())
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

	int Graph::GetIndex(const Node& n) const
	{
		return (getID(n));
	}

	bool Graph::HasEdge(int parent, int child, bool undirected) const {
		return (checkForEdge(parent, child) || (undirected && checkForEdge(child, parent)));
	}

	bool Graph::HasEdge(const Node& parent, const Node& child, const bool undirected) const {

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
	}
	
	void Graph::AddEdges(const vector<vector<EdgeSet>>& edges, const string& cost_name)
	{
		if (this->IsDefaultName(cost_name))
			throw NotImplemented();

		auto cost_set = GetOrCreateCostType(cost_name);
		
		
	}

	vector<EdgeSet> Graph::GetEdges(const string& cost_name) const
	{
		throw HF::Exceptions::NotImplemented();
		return std::vector<EdgeSet>();
	}

	void Graph::AddEdges(const vector<vector<IntEdge>>& edges, const string& cost_name)
	{
		throw HF::Exceptions::NotImplemented();
	}

	void Graph::AddEdges(std::vector<std::vector<IntEdge>>& edges) {

	}

	void Graph::AddEdges(std::vector<std::vector<EdgeSet>>& edges) {
	}

	std::vector<Node> Graph::GetChildren(const Node& n) const {
		std::vector<Node> children;

		auto edges = (*this)[n];

		for (auto e : edges) {
			children.push_back(e.child);
		}

		return children;
	}

	std::vector<Node> Graph::GetChildren(const int parent_id) {
		return GetChildren(NodeFromID(parent_id));
	}

	Subgraph Graph::GetSubgraph(Node& parent_node) {
		return Subgraph{ parent_node, (*this)[parent_node] };
	}

	Subgraph Graph::GetSubgraph(int parent_id) {
		Node parent_node = ordered_nodes[parent_id];
		return Subgraph{ parent_node, (*this)[parent_node] };
	}

	void Graph::AddNodeAttribute(int id, std::string attribute, std::string score) {
		const auto node = NodeFromID(id);
		bool node_not_found = hasKey(node);

		if (node_not_found) {
			// Check to see if a node with id exists in the graph.
			// If not, return.
			return;
		}

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

	void Graph::AddNodeAttributes(std::vector<int> id, std::string name, std::vector<std::string> scores) {
		// If size of id container and size of scores container are not in alignment,
		// we return.
		if (id.size() != scores.size()) {
			return;
		}

		auto scores_iterator = scores.begin();

		for (int node_id : id) {
			// We can call AddNodeAttribute for each node_id in id.
			// If the attribute type name does not exist,
			// it will be created with the first invocation of AddNodeAttribute.
			AddNodeAttribute(node_id, name, *(scores_iterator++));
		}
	}

	std::vector<std::string> Graph::GetNodeAttributes(std::string attribute) const {
		std::vector<std::string> attributes;

		/* // requires #include <algorithm>, but not working?
		std::string lower_cased =
			std::transform(attribute.begin(), attribute.end(),
				[](unsigned char c) { return std::tolower(c); }
		);
		*/
		std::string lower_cased = attribute;

		auto node_attr_map_it = node_attr_map.find(lower_cased);

		if (node_attr_map_it == node_attr_map.end()) {
			// If the attribute does not exist...
			// return an empty container.
			return attributes;
		}

		// We now have the NodeAttributeValueMap for the desired attribute.
		// A NodeAttributeValueMap stores buckets of [node id : node attribute value as string]
		NodeAttributeValueMap node_attr_value_map = node_attr_map_it->second;

		for (auto& bucket : node_attr_value_map) {
			// For all buckets in the node_attr_value_map,
			// extract the attribute (attr) and append it to attributes
			std::string attr = bucket.second;
			attributes.push_back(attr);
		}

		// Return all attr found
		return attributes;
	}

	void Graph::ClearNodeAttributes(std::string name) {
		/* // requires #include <algorithm>, but not working?
		std::string lower_cased =
			std::transform(attribute.begin(), attribute.end(),
				[](unsigned char c) { return std::tolower(c); }
		);
		*/
		std::string lower_cased = name;

		auto node_attr_map_it = node_attr_map.find(lower_cased);

		if (node_attr_map_it == node_attr_map.end()) {
			// If the attribute name does not exist,
			// return.
			return;
		}

		// Note that a node_attr_map is a
		// unordered_map<std::string, NodeAttributeValueMap>
		// where the key is attribute type name, like "cross slope",
		// and the value is a hashmap, as described below:
		///
		// A NodeAttributeValueMap is a
		// unordered_map<int, std::string>
		// where the key is a node id,
		// and the value is an attribute value, in the form of a string.
		//
		// What is being cleared is the
		// NodeAttributeValueMap that is mapped to name.
		// The attribute name is still a key in node_attr_map,
		// but has no value -- which is the NodeAttributeValueMap instance.
		node_attr_map[name].clear();
	}
}
