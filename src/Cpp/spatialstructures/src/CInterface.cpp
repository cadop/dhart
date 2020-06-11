///
/// \file		CInterface.cpp
///	\brief		Source file for interoperability between programming languages
///
/// \author		TBA
/// \date		06 Jun 2020
///
#include <spatialstructures_C.h>
#include <iostream>
#include <Constants.h>

#include <HFExceptions.h>
#include <graph.h>
#include <node.h>
#include <edge.h>

// TODO: Use a template for this
/// <summary>
/// Convert a raw array from an external caller to an organized vector of points
/// </summary>
/// <param name="raw_array">Pointer to the external array</param>
/// <param name="size">The number of points stored in the raw array, equal to the total number of floats / 3</param>
/// <returns></returns>
std::vector<std::array<float, 3>> ConvertRawFloatArrayToPoints(const float* raw_array, int size) {
	std::vector<std::array<float, 3>> out_array(size);
	for (int i = 0; i < size; i++) {
		const int os = i * 3;
		out_array[i][0] = raw_array[os];
		out_array[i][1] = raw_array[os + 1];
		out_array[i][2] = raw_array[os + 2];
	}
	return out_array;
}

std::vector<std::array<int, 3>> ConvertRawIntArrayToPoints(const int* raw_array, int size) {
	std::vector<std::array<int, 3>> out_array(size);
	for (int i = 0; i < size; i++) {
		const int os = i * 3;
		out_array[i][0] = raw_array[os];
		out_array[i][1] = raw_array[os + 1];
		out_array[i][2] = raw_array[os + 2];
	}
	return out_array;
}

C_INTERFACE GetEdgesForNode(const Graph* graph, const Node* Node, vector<Edge>** out_vector_ptr, Edge** out_edge_list_ptr, int* out_edge_list_size) {
	// This can't function if the node isn't a parent
	if (!(graph->hasKey(*Node))) {
		return HF_STATUS::OUT_OF_RANGE;
	}

	vector<Edge>* Edges = new vector<Edge>();
	*Edges = (*graph)[*Node];
	*out_edge_list_ptr = Edges->data();
	*out_edge_list_size = Edges->size();
	*out_vector_ptr = Edges;
	return OK;
}

C_INTERFACE GetSizeOfNodeVector(const std::vector<HF::SpatialStructures::Node>* node_list, int* out_size) {
	*out_size = node_list->size();
	return OK;
}

C_INTERFACE GetSizeOfEdgeVector(const std::vector<HF::SpatialStructures::Edge>* edge_list, int* out_size) {
	*out_size = edge_list->size();
	return OK;
}

C_INTERFACE DestroyNodes(std::vector<HF::SpatialStructures::Node>* nodelist_to_destroy) {
	if (nodelist_to_destroy) delete nodelist_to_destroy;
	return OK;
}

C_INTERFACE DestroyEdges(std::vector<HF::SpatialStructures::Edge>* edgelist_to_destroy) {
	if (edgelist_to_destroy) delete edgelist_to_destroy;
	return OK;
}

C_INTERFACE DestroyGraph(HF::SpatialStructures::Graph* graph_to_destroy) {
	if (graph_to_destroy) delete graph_to_destroy;
	return OK;
}
