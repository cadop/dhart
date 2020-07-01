#include "visibility_graph_C.h"

#include <vector>
#include <visibility_graph.h>
#include <embree_raytracer.h>
#include <graph.h>
#include <node.h>

#include <HFExceptions.h>

using namespace HF::RayTracer;
using namespace HF::SpatialStructures;
using namespace HF;
using std::vector;

C_INTERFACE CreateVisibilityGraphAllToAll(
	EmbreeRayTracer* ert,
	const float* nodes,
	int num_nodes,
	Graph** out_graph,
	float height
) {
	auto array_of_nodes = ConvertRawFloatArrayToPoints(nodes, num_nodes);
	
	vector<Node> vector_of_nodes(num_nodes);
	for (int i = 0; i < num_nodes; i++) {
		const auto& arr = array_of_nodes[i];
		auto& vec = vector_of_nodes[i];
		
		vec[0] = arr[0]; vec[1] = arr[1]; vec[2] = arr[2];
	}

	Graph * vg = new Graph();
	*vg = VisibilityGraph::AllToAll(*ert, vector_of_nodes, height );
	
	*out_graph = vg;
	return HF::Exceptions::HF_STATUS::OK;
}

C_INTERFACE CreateVisibilityGraphAllToAllUndirected(
	EmbreeRayTracer* ert, 
	const float* nodes,
	int num_nodes, 
	Graph** out_graph,
	float height,
	const int cores
) {
	auto array_of_nodes = ConvertRawFloatArrayToPoints(nodes, num_nodes);

	vector<Node> vector_of_nodes(num_nodes);
	for (int i = 0; i < num_nodes; i++) {
		const auto& arr = array_of_nodes[i];
		auto& vec = vector_of_nodes[i];

		vec[0] = arr[0]; vec[1] = arr[1]; vec[2] = arr[2];
	}
	Graph* vg = new Graph();
	*vg = VisibilityGraph::AllToAllUndirected(*ert, vector_of_nodes, height, cores);

	*out_graph = vg;
	return HF::Exceptions::HF_STATUS::OK;
}

C_INTERFACE CreateVisibilityGraphGroupToGroup(
	HF::RayTracer::EmbreeRayTracer* ert, 
	const float* group_a, 
	const int size_a,
	const float* group_b, 
	const int size_b,
	Graph** out_graph,
	float height
) {
	auto array_a = ConvertRawFloatArrayToPoints(group_a, size_a);
	auto array_b = ConvertRawFloatArrayToPoints(group_b, size_b);

	vector<Node> vector_a(size_a);
	vector<Node> vector_b(size_b);
	for (int i = 0; i < size_a; i++) {
		const auto& arra = array_a[i];
		auto& va = vector_a[i];
		va[0] = arra[0]; va[1] = arra[1]; va[2] = arra[2];
	}
	for (int i = 0; i < size_b; i++) {
		const auto& arrb = array_b[i];
		auto& vb = vector_b[i];
		vb[0] = arrb[0]; vb[1] = arrb[1]; vb[2] = arrb[2];
	}
	*out_graph = new Graph();

	**out_graph = VisibilityGraph::GroupToGroup(*ert, vector_a, vector_b, height);
	
	if ((*out_graph)->GetCSRPointers().AreValid()) {
		return HF::Exceptions::HF_STATUS::OK;
	}
	else {
		delete* out_graph;
		return HF::Exceptions::HF_STATUS::NO_GRAPH;
	}
}
