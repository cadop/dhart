///
/// \file		graph.cpp
///	\brief		Source file for a graph ADT
///
/// \author		TBA
/// \date		06 Jun 2020
///
#include "graph.h"

#include <Graph.h>
#include <algorithm>
#include <math.h>
#include <Constants.h>
#include <assert.h>

using namespace Eigen;
using std::vector;

namespace HF::SpatialStructures {
	int Graph::size() const { return id_to_nodes.size(); }

	int Graph::getID(const Node& node) const
	{
		if (hasKey(node))
			return idmap.at(node);
		else
			return -1;
	}

	CSRPtrs Graph::GetCSRPointers()
	{
		Compress(); // The graph must be compressed for this to work

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


	Node Graph::NodeFromID(int id) const { return ordered_nodes[id]; }

	std::vector<Node> Graph::Nodes() const {
		return ordered_nodes;
	}

	vector<Edge> Graph::GetUndirectedEdges(const Node & n) const {
		// Get the ID of n
		int node_id = getID(n);
		// If N is not in the graph, return an empty array.
		if (node_id < 0) return vector<Edge>();

		// Get the directed edges for this node from calling
		// operator[]
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
		// Throw if we're not compressed since this is a const function
		// and compressing the graph will make it non-const
		if (this->needs_compression)
			throw std::exception("The graph must be compressed!");

		// Preallocate an array of edge sets
		vector<EdgeSet> out_edges(this->size());
		
		// Iterate through every row in the csr 
		for (int k = 0; k < edge_matrix.outerSize(); ++k) {
			auto& edgeset = out_edges[k];
			edgeset.parent = k;

			// Iterate every column in the row. 
			for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, k); it; ++it)
			{
				// Add to array of edgesets
				float cost = it.value();
				int child = it.col();
				edgeset.children.push_back(IntEdge{ child, cost });
			}
		}
		return out_edges;
	}

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
		if (this->needs_compression) throw std::exception("The graph must be compressed!");

		vector<float> out_costs(this->size(), 0);
		if (directed)
			for (int k = 0; k < edge_matrix.outerSize(); ++k) {
				float sum = edge_matrix.row(k).sum();
				int count = edge_matrix.row(k).nonZeros();
				Aggregate(out_costs[k], sum, agg_type, count);
			}
		else {
			vector<int> count(this->size(), 0);
			for (int k = 0; k < edge_matrix.outerSize(); ++k) {
				for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, k); it; ++it)
				{
					float cost = it.value();
					int child = it.col();
					int parent = it.row();

					Aggregate(out_costs[parent], cost, agg_type, count[parent]);
					Aggregate(out_costs[child], cost, agg_type, count[child]);
				}
			}

		}
		return out_costs;
	}

	const std::vector<Edge> Graph::operator[](const Node& n) const
	{
		int parent_id = idmap.at(n);
		std::vector<Edge> out_edges;

		for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, parent_id); it; ++it) {
			auto value = it.value();
			auto col = it.col();

			out_edges.emplace_back(Edge(NodeFromID(col), value));
		}

		return out_edges;
	}

	void Graph::addEdge(const Node& parent, const Node& child, float score)
	{
		// ![GetOrAssignID_Node]
		needs_compression = true;

		int parent_id = getOrAssignID(parent);
		int child_id = getOrAssignID(child);
		triplets.emplace_back(
			Eigen::Triplet<float>(parent_id, child_id, score)
		);
		// ![GetOrAssignID_Node]
	}

	void Graph::addEdge(int parent_id, int child_id, float score)
	{
		// ![GetOrAssignID_int]
		needs_compression = true;
		next_id = std::max(child_id, std::max(parent_id, next_id));
		getOrAssignID(child_id);
		getOrAssignID(parent_id);
		triplets.emplace_back(Eigen::Triplet<float>(parent_id, child_id, score));
		// ![GetOrAssignID_int]
	}

	bool Graph::checkForEdge(int parent, int child) const {
		// ![CheckForEdge]
		for (SparseMatrix<float, 1>::InnerIterator it(edge_matrix, parent); it; ++it)
			if (it.col() == child) return true;
		
		return false;
		// ![CheckForEdge]
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
		int parent_id = idmap.at(parent);
		int child_id = idmap.at(child);
		
		// Call integer overload.
		return HasEdge(parent_id, child_id, undirected);
	}

	int Graph::getOrAssignID(const Node& input_node)
	{
		if (hasKey(input_node))
			return getID(input_node);
		else {
			idmap[input_node] = next_id;
			ordered_nodes.push_back(input_node);
			id_to_nodes.push_back(ordered_nodes.size() - 1);
			ordered_nodes[next_id].id = next_id;
			next_id++;
			return next_id - 1;
		}
	}

	int Graph::getOrAssignID(int input_int)
	{
		// If it's already in our id list, then there's nothing more to do
		if (std::find(this->id_to_nodes.begin(), this->id_to_nodes.end(), input_int)
			!= this->id_to_nodes.end()) 
			return input_int;
		else {
			ordered_nodes.push_back(Node());
			ordered_nodes.back().id = input_int;
			id_to_nodes.push_back(ordered_nodes.size() - 1);
		}
	}

	Graph::Graph(const vector<vector<int>>& edges, const vector<vector<float>> & distances, const vector<Node> & Nodes)
	{
		// Generate an array with the size of every column from
		// the size of the edges array
		assert(edges.size() == distances.size());
		vector<int> sizes(edges.size());
		for (int i = 0; i < edges.size(); i++) {
			sizes[i] = edges[i].size();
		}

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

	bool Graph::HasEdge(const std::array<float, 3>& parent, const std::array<float, 3>& child, bool undirected) const {
		// Just convert it and pass it over to the other HasEdge
		const Node p(parent);
		const Node c(child);
		return HasEdge(p, c, undirected);
	}

	bool Graph::hasKey(const Node& n) const { return (idmap.count(n) > 0); }

	std::vector<std::array<float, 3>> Graph::NodesAsFloat3() const
	{
		auto N = Nodes();
		int n = N.size();

		std::vector <std::array<float, 3> > out_nodes(n);

		/*
		for (int i = 0; i < n; i++)
		{
			out_nodes[i][0] = N[n].x;
			out_nodes[i][1] = N[n].y;
			out_nodes[i][2] = N[n].z;
		}
		*/

		/// This yields the intended output
		for (int i = 0; i < n; i++) {
			out_nodes[i][0] = N[i].x;
			out_nodes[i][1] = N[i].y;
			out_nodes[i][2] = N[i].z;
		}

		return out_nodes;
	}

	void Graph::Compress() {
		if (needs_compression) {
			int max_id = std::max_element(this->ordered_nodes.begin(), this->ordered_nodes.end())[0].id + 1;
			edge_matrix.resize(max_id, max_id);
		//	edge_matrix.resize(size(), size());
			edge_matrix.setFromTriplets(triplets.begin(), triplets.end());
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
		id_to_nodes.clear();
		idmap.clear();
	}
}

/*
void Graph::GenerateCrossSlope()
{
	// Iterate through every edge
	for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
		const auto & parent = it->getFirst();
		std::vector<Edge>& link_list = dictionary[parent];

		// Compare every edge to every other edge to get a
		for (Edge& link_A : link_list) {
			const auto& child_a = link_A.child;
			auto dir_a = parent.directionTo(child_a);

			std::vector<Edge> weights;
			// Check to see if child_B is perpendicular to child_A
			for (auto& link_B : link_list) {
				if (link_B.child == link_A.child) continue;
				const auto& child_b = link_B.child;
				auto dir_b = parent.directionTo(child_b);

				// Compute the dot product for the direction_a and direction_B
				float dot_prod = (dir_b[0] * dir_a[0]) + (dir_a[1] * dir_b[1]);

				// If they are perpendicular, then store the child
				if (abs(dot_prod) < ROUNDING_PRECISION) weights.push_back(link_B);
			}

			// Compute weight based on each connection if any perpendicular nodes were found
			float weight = 0.0f;
			if (weights.size() == 0)
				weight = link_A.score;
			else if (weights.size() == 1) {
				float a_z = child_a[2];
				float b_z = weights[0].child[2];

				weight = abs(a_z - b_z) + weights[0].score;
			}
			else if (weights.size() == 2) {
				float a_z = child_a.pos[2];
				float b_z = weights[0].child[2];
				float c_z = weights[1].child[2];

				weight = abs(b_z - c_z) + weights[0].score;
			}
			else {
				// TODO: Throw some kind of exception?;
				assert(false);
			}

			link_A.cross_slope = weight;
		}
	}
}
static inline double myradians(double angle) {
	return (double)angle * (M_PI / 180.00);
}
void Graph::GenerateEnergy()
{
	// Iterate through every edge
	for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
		auto& parent = it->getFirst();
		std::vector<HF::Edge>& link_list = dictionary[parent];

		// Compare every edge to every other edge to get a
		for (HF::Edge& link_A : link_list) {
			// Clamp it to 0.4, -0.4
			auto slope = std::clamp(tanf(myradians(link_A.angle)), -0.4f, 0.4f);

			auto e = 280.5 * (pow(slope, 5)) - 58.7 * (pow(slope, 4)) - 76.8 * (pow(slope, 3)) + 51.9 * (pow(slope, 2)) + 19.6 * (slope)+2.5;
			assert(e > 0);

			link_A.score = e * link_A.distance;
		}
	}
}
*/