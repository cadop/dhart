#include "cost_algorithms.h"
#include <spatialstructures_C.h>
#include <HFExceptions.h>
#include <graph.h>
#include <edge.h>
#include <node.h>
#include <robin_hood.h>
#include <iostream>

using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::SpatialStructures::Edge;
using namespace HF::Exceptions;
using std::vector;

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

C_INTERFACE AggregateCosts(
	const Graph* graph,
	int agg,
	bool directed,
	std::vector<float>** out_vector_ptr,
	float** out_data_ptr
) {
	try {
		*out_vector_ptr = new std::vector<float>();
		**out_vector_ptr = graph->AggregateGraph(static_cast<HF::SpatialStructures::COST_AGGREGATE>(agg), directed);
		*out_data_ptr = (**out_vector_ptr).data();
	}
	catch (std::exception & e){
		return NO_GRAPH;
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
	float score
){
	Node parent_node, child_node;
	try {
		parent_node = Node(parent[0], parent[1], parent[2]);
		child_node = Node(child[0], child[1], child[2]);
	}
	catch(std::exception & E){
		std::cerr << "Invalid parent or child node " << std::endl;
		return HF::Exceptions::INVALID_PTR;
	}

	graph->addEdge(parent_node, child_node, score);
	return OK;
}

C_INTERFACE AddEdgeFromNodeIDs(Graph * graph, int parent_id, int child_id, float score) {
	graph->addEdge(parent_id, child_id, score);
	return OK;
}

C_INTERFACE GetCSRPointers(
	Graph* graph,
	int* out_nnz,
	int* out_num_rows,
	int* out_num_cols,
	float** out_data_ptr,
	int ** out_inner_indices_ptr,
	int ** out_outer_indices_ptr
) {
	auto CSR = graph->GetCSRPointers();

	*out_nnz = CSR.nnz;
	*out_num_rows = CSR.rows;
	*out_num_cols = CSR.cols;
	
	*out_data_ptr = CSR.data;
	*out_inner_indices_ptr = CSR.inner_indices;
	*out_outer_indices_ptr = CSR.outer_indices;
	
	return OK;
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

C_INTERFACE ClearGraph(HF::SpatialStructures::Graph* graph)
{
	graph->Clear();
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

C_INTERFACE AddNodeAttributes(HF::SpatialStructures::Graph* g, const int* ids, 
							 const char* attribute, const char** scores, int num_nodes) {
	
	// It is easy to convert raw pointers that are known to point to buffers/arrays
	// to a std::vector.

	// ids is the base address, and ids + num_nodes is one-past the last address allocated for ids.
	std::vector<int> v_ids(ids, ids + num_nodes);
	
	// scores is the base address, and scores + num_nodes is one-past the last address allocated for scores.
	std::vector<std::string> v_scores(scores, scores + num_nodes);

	// If it turns out that v_ids and v_scores have different sizes,
	// AddNodeAttributes will discover this.
	g->AddNodeAttributes(v_ids, std::string(attribute), v_scores);

	return OK;
}

C_INTERFACE GetNodeAttributes(const HF::SpatialStructures::Graph* g, const char* attribute, 
							  char*** out_scores, int* out_score_size) {

	std::vector<std::string> v_attrs = g->GetNodeAttributes(std::string(attribute));

	// Caller creates a char **scores variable.
	// char **scores = new char*[max_size];
	// int score_size;
	//
	// Graph g; // assume this graph is ready
	// 
	// They will invoke this code: 
	// GetNodeAttributes(&g, "attr name", &scores, &score_size);
	//
	// In this function:
	// dereference out_scores, like this: (*out_scores)
	//
	// each (*out_scores)[i] will point to newly allocated memory.
	// (*out_scores)[i] = new char[std::strlen(attr_str_size)];
	//
	// std::strncpy((*out_scores)[i], src_str, attr_str_size);
	// (*out_scores)[i][attr_str_size - 1] = '\0';

	int i = 0;
	for (auto v_str : v_attrs) {
		// Get the c-string from the current v_str in the loop.
		const char* cstr = v_str.c_str();

		// Derive the length of the c-string using strlen
		size_t attr_str_size = std::strlen(cstr);

		// out_scores is the address of a (char **); (the address to a buffer of string)
		// a (char **) is a pointer to a buffer of (char *). (a buffer of string)
		// a (char *) is a pointer to a buffer of char. (a string)
		// (*out_scores) is a char **, and its memory was already allocated by the caller.
		//
		// For each (*out_scores)[i], we will use operator new to dynamically allocate memory.
		(*out_scores)[i] = new char[attr_str_size];

		// (*out_scores)[i] is an empty buffer, so we copy the c-string
		// to (*out_scores)[i]. We make sure to null-terminate this buffer.
		std::strncpy((*out_scores)[i], cstr, attr_str_size);
		(*out_scores)[attr_str_size - 1] = '\0';

		// We increment i, so that we allocate memory for the next (char *), or string
		// in the iteration. v_str, the std::string, will be updated automatically
		// by virtue of the for-each loop as we iterate over v_attrs.
		++i;
	}

	// Update the *out_score_size value, which corresponds to v_attrs.size().
	// (it also corresponds to i, but this notation using v_attrs.size() is easier to understand)
	* out_score_size = v_attrs.size();

	// If v_attrs.size() == 0, do we want to throw an exception,
	// which would mean that attribute does not exist as a attribute type?
	return OK;
}

C_INTERFACE DeleteScoreArray(char*** scores_to_delete, int num_char_arrays) {
	if (scores_to_delete) {
		// If scores_to_delete is non-null...

		char** curr = (*scores_to_delete);
		char** end = (*scores_to_delete) + num_char_arrays;

		while (curr < end) {
			char** victim = curr;
			++curr;

			delete* victim;
			*victim = nullptr;	// eliminate dangling pointer
		}

		*scores_to_delete = nullptr; // eliminate dangling pointer
	}

	return OK;
}

C_INTERFACE ClearAttributeType(HF::SpatialStructures::Graph* g, const char* s) {
	if (s) {
		// Does not hurt to check if s is non-null
		g->ClearNodeAttributes(std::string(s));
	}

	return OK;
}
