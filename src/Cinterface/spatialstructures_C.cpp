#include "cost_algorithms.h"
#include <spatialstructures_C.h>
#include <HFExceptions.h>
#include <graph.h>
#include <edge.h>
#include <node.h>
#include <robin_hood.h>
#include <iostream>

using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Subgraph;
using HF::SpatialStructures::Node;
using HF::SpatialStructures::Edge;
using namespace HF::Exceptions;
using std::vector;
using std::string;


inline bool parse_string(const char * c) {
	if (!c) return false;

	try {
		std::string test_string(c);
	}
	catch (...) {
		return false;
	}

	return true;
}

C_INTERFACE  GetAllNodesFromGraph(const Graph* graph, vector<Node>** out_vector_ptr, Node** out_data_ptr)
{
	if (!graph)
		return HF_STATUS::INVALID_PTR;

	try {

		std::vector<Node>* nodes = new vector<Node>();
		auto i = (*graph);
		*nodes = graph->Nodes();

		*out_data_ptr = nodes->data();
		*out_vector_ptr = nodes;
		return OK;
	}
	catch (...) { return HF_STATUS::GENERIC_ERROR; }
	return GENERIC_ERROR;
}
C_INTERFACE GetSizeOfNodeVector(
	const vector<Node>* node_list,
	int* out_size
) {
	*out_size = node_list->size();
	return OK;
}

C_INTERFACE GetSizeOfEdgeVector(
	const vector<Edge>* edge_list,
	int* out_size
){
	*out_size = edge_list->size();
	return OK;
}

C_INTERFACE GetEdgeCost(
	const Graph* g, 
	int parent,
	int child,
	const char* cost_type,
	float* out_float
) {

	try {
		*out_float = g->GetCost(parent, child, std::string(cost_type));
		if (!std::isfinite(*out_float)) *out_float = -1.0f;
	}
	catch (HF::Exceptions::NoCost)
	{
		return NO_COST;
	}
	catch (std::logic_error)
	{
		return NOT_COMPRESSED;
	}
	catch (...)
	{
		return GENERIC_ERROR;
	}
	return OK;

}

C_INTERFACE AggregateCosts(
	const Graph* graph,
	int agg,
	bool directed,
	const char* cost_type,
	std::vector<float>** out_vector_ptr,
	float** out_data_ptr
) {

	if (!parse_string(cost_type))
		return NO_COST;

	try {
		std::string cost_string(cost_type);
		*out_vector_ptr = new std::vector<float>();
		**out_vector_ptr = graph->AggregateGraph(static_cast<HF::SpatialStructures::COST_AGGREGATE>(agg), directed, cost_string);
		*out_data_ptr = (**out_vector_ptr).data();
	}
	catch (HF::Exceptions::NoCost) {
		return NO_COST; // Cost doesn't exist
	}
	catch (std::logic_error) {
		return NOT_COMPRESSED; // Graph isn't compressed
	}
	catch (std::exception & e){
		return GENERIC_ERROR; // Graph likely doesn't exist, but something else is funky. 
	}
	return OK;
}

C_INTERFACE CreateGraph(
	const float* nodes,
	int num_nodes,
	Graph** out_graph
){
	//TODO: Add node constructor
	Graph * g = new Graph();
	*out_graph = g;
	return OK;
}

C_INTERFACE AddEdgeFromNodes(
	Graph* graph,
	const float* parent,
	const float* child,
	float score,
	const char* cost_type
){
	Node parent_node, child_node;

	if (!parse_string(cost_type))
		return NO_COST;

	std::string cost_name(cost_type);
	try {
		parent_node = Node(parent[0], parent[1], parent[2]);
		child_node = Node(child[0], child[1], child[2]);
	}
	catch(std::exception & E){
		std::cerr << "Invalid parent or child node " << std::endl;
		return HF::Exceptions::INVALID_PTR;
	}

	try {
		graph->addEdge(parent_node, child_node, score, cost_name);
	}
	catch (std::logic_error){
		return NOT_COMPRESSED;
	}
	catch (std::out_of_range) {
		return OUT_OF_RANGE;
	}
	return OK;
}

C_INTERFACE AddEdgeFromNodeIDs(Graph * graph, int parent_id, int child_id, float score, const char * cost_type) {
	if (!parse_string(cost_type))
		return NO_COST;

	try {
		graph->addEdge(parent_id, child_id, score, std::string(cost_type));
	}
	catch (std::logic_error) {
		return NOT_COMPRESSED;
	}
	catch (std::out_of_range) {
		return OUT_OF_RANGE;
	}

	return OK;
}

C_INTERFACE GetCSRPointers(
	Graph* graph,
	int* out_nnz,
	int* out_num_rows,
	int* out_num_cols,
	float** out_data_ptr,
	int ** out_inner_indices_ptr,
	int ** out_outer_indices_ptr,
	const char * cost_type
) {
	try {
		auto CSR = graph->GetCSRPointers(std::string(cost_type));
		*out_nnz = CSR.nnz;
		*out_num_rows = CSR.rows;
		*out_num_cols = CSR.cols;

		*out_data_ptr = CSR.data;
		*out_inner_indices_ptr = CSR.inner_indices;
		*out_outer_indices_ptr = CSR.outer_indices;

		return OK;
	}
	catch (NoCost) {
		return NO_COST;
	}
	catch (...) {
		return GENERIC_ERROR;
	}
}

C_INTERFACE GetNodeID(
	HF::SpatialStructures::Graph* graph,
	const float * point,
	int* out_id
) {

	Node node(point[0], point[1], point[2]);
	*out_id = graph->getID(node);
	return OK;
}

C_INTERFACE Compress(Graph* graph)
{
	graph->Compress();
	return OK;
}

C_INTERFACE ClearGraph(HF::SpatialStructures::Graph* graph, const char* cost_type)
{
	std::string cost_name(cost_type);

	if (!cost_name.empty())
		graph->Clear();
	else {
		try { graph->ClearCostArrays(cost_name); }
		catch (NoCost) {
			return NO_COST;
		}
	}
	return OK;

}

C_INTERFACE DestroyNodes(vector<Node>* nodelist_to_destroy)
{
	if (nodelist_to_destroy) delete nodelist_to_destroy;
	return OK;
}

C_INTERFACE DestroyEdges(vector<Edge>* edgelist_to_destroy)
{
	if (edgelist_to_destroy) delete edgelist_to_destroy;
	return OK;
}

C_INTERFACE DestroyGraph(Graph* graph_to_destroy)
{
	if (graph_to_destroy) delete graph_to_destroy;
	return OK;
}

C_INTERFACE CalculateAndStoreEnergyExpenditure(HF::SpatialStructures::Graph* g) {
	// Collecting energy expenditure data for all parent nodes.

	// In std::vector<std::vector<EdgeSet>> Graph::CalculateEnergyExpenditure(Graph& g),
	// std::vector<EdgeSet> Graph::CalculateEnergyExpenditure(Subgraph& sg)
	// is called for all Subgraph in g.

	// result is a std::vector<std::vector<EdgeSet>>
	auto result = HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure(*g);

	// Now passing result, a std::vector<std::vector<EdgeSet>> to Graph::AddEdges.
	// The result container will be ordered by parent id.

	g->AddEdges(result, AlgorithmCostTitle(COST_ALG_KEY::ENERGY_EXPENDITURE));

	return OK;
}
C_INTERFACE AddNodeAttributes(
	Graph* g,
	const int* ids,
	const char* attribute,
	const char** scores,
	int num_nodes
) {
	
	// It is easy to convert raw pointers that are known to point to buffers/arrays
	// to a std::vector.

	// ids is the base address, and ids + num_nodes is one-past the last address allocated for ids.
	std::vector<int> v_ids(ids, ids + num_nodes);
	
	// scores is the base address, and scores + num_nodes is one-past the last address allocated for scores.
	std::vector<std::string> v_scores(scores, scores + num_nodes);

	// If it turns out that v_ids and v_scores have different sizes,
	// AddNodeAttributes will discover this.
	try {
		g->AddNodeAttributes(v_ids, std::string(attribute), v_scores);
	}
	catch (std::logic_error) {
		//return HF_STATUS::OUT_OF_RANGE;
		assert(false); // This is purely due to programmer error. The top of this function should
					   // ONLY read num_nodes elements from either array, and this exception will
					   // only throw if the length of scores and ids is different
	}

	return OK;
}

C_INTERFACE GetNodeAttributes(const HF::SpatialStructures::Graph* g, const char* attribute, 
							  char** out_scores, int* out_score_size) {

	// Get the node attributes from tthe graph at attribute
	vector<string> v_attrs = g->GetNodeAttributes(std::string(attribute));
	
	// Iterate through each returned string and copy it into
	// the output array
	for (int i = 0; i < v_attrs.size(); i++){

		// Get the string and its corresponding c_string
		const string& v_str = v_attrs[i];
		const char* cstr = v_str.c_str();
		
		// Allocate space in the output array for this string 
		const int string_length = v_str.size() + 1; //NOTE: This must be +1 since the null terminator doesn't count
													// towards the string's overall length
		out_scores[i] = new char[string_length];

		// Copy the contents of c_str into the output array
		std::strncpy(out_scores[i], cstr, string_length);
	}

	// Update the *out_score_size value, which corresponds to v_attrs.size().
	// (it also corresponds to i, but this notation using v_attrs.size() is easier to understand)
	* out_score_size = v_attrs.size();

	// If v_attrs.size() == 0, do we want to throw an exception,
	// which would mean that attribute does not exist as a attribute type?
	return OK;
}

C_INTERFACE DeleteScoreArray(char** scores_to_delete, int num_char_arrays) {
	if (scores_to_delete) {
	
		for (int i = 0; i < num_char_arrays; i++) {
			char* score_string = scores_to_delete[i];
			delete[](score_string); // Explictly delete the char array at victim
		}
	}
	return OK;
}

C_INTERFACE ClearAttributeType(HF::SpatialStructures::Graph* g, const char* s) {
	if (s) {
		// Does not hurt to check if s is non-null
		g->ClearNodeAttributes(std::string(s));
		return OK;
	}
	// Inform the caller that they gave us a null pointer
	else
		return INVALID_PTR;

}


C_INTERFACE CalculateAndStoreCrossSlope(HF::SpatialStructures::Graph* g) {
	// Collecting cross slope data for all parent nodes.

	// In std::vector<std::vector<IntEdge>> Graph::CalculateCrossSlope(Graph& g),
	// std::vector<IntEdge> Graph::CalculateCrossSlope(Subgraph& sg)
	// is called for all Subgraph in g.

	// result is a std::vector<std::vector<IntEdge>>
	auto result = HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope(*g);

	// Now passing result, a std::vector<std::vector<IntEdge>> to Graph::AddEdges.
	// The result container will be ordered by parent id.

	// TODO: implement void Graph::AddEdges(std::vector<std::vector<IntEdge>>& edges);
	g->AddEdges(result, AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE));

	return OK;
}
