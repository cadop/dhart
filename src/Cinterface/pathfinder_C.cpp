#include <pathfinder_C.h>

#include <memory>
#include <vector>

#include <HFExceptions.h>
#include <graph.h>
#include <path_finder.h>
#include <path.h>
#include <numeric>
#include <boost_graph.h>

using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::string;

using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Path;
using HF::SpatialStructures::PathMember;
using HF::Exceptions::HF_STATUS;
using namespace HF::Pathfinding;


C_INTERFACE CreatePath(
	const HF::SpatialStructures::Graph* g,
	int start,
	int end,
	const char * cost_type,
	int* out_size,
	HF::SpatialStructures::Path** out_path,
	HF::SpatialStructures::PathMember** out_data
) {
	// Get a boost graph from *g
	std::unique_ptr<BoostGraph, BoostGraphDeleter> bg;
	
	try {
		// If cost_name is not a valid cost type in *g,
		// HF::Exceptions::NoCost is thrown by Graph::GetEdges
		bg = CreateBoostGraph(*g, std::string(cost_type));
	}
	catch (HF::Exceptions::NoCost) {
		return HF::Exceptions::HF_STATUS::NO_COST;
	}
	catch (...) { // Ensure that we catch other potential issues
		return HF::Exceptions::GENERIC_ERROR;
	}

	
	// Allocate a new empty path
	Path* P = new Path();

	// Generate a path using the boost graph we just created.
	*P = FindPath(bg.get(), start, end);

	// If P isn't empty, set our output pointer to it
	if (!P->empty()) {
		*out_path = P;
		*out_data = P->GetPMPointer();
		*out_size = P->size();
		return HF::Exceptions::HF_STATUS::OK;
	}

	// Otherwise, free the memory for it and signal that no path
	// could be found
	else {
		delete P;
		return HF::Exceptions::HF_STATUS::NO_PATH;
	}
}

C_INTERFACE CreatePaths(
	const HF::SpatialStructures::Graph* g,
	const int* start,
	const int* end,
	const char* cost_type,
	HF::SpatialStructures::Path** out_path_ptr_holder,
	HF::SpatialStructures::PathMember** out_path_member_ptr_holder,
	int* out_sizes,
	int num_paths
) {

	vector<int> starts(start, start + num_paths);
	vector<int> ends(end, end + num_paths);

	// Get a boost graph from *g
	std::unique_ptr<BoostGraph, BoostGraphDeleter> bg;
	
	try {
		// If cost_name is not a valid cost type in *g,
		// std::out_of_range is thrown by Graph::GetEdges
		bg = CreateBoostGraph(*g, std::string(cost_type));
	}
	catch (HF::Exceptions::NoCost) {
		return HF::Exceptions::HF_STATUS::NO_COST;
	}
	catch (...) {
		return HF::Exceptions::HF_STATUS::GENERIC_ERROR;
	}

	// Find all the asked for paths
	InsertPathsIntoArray(
		bg.get(),
		starts,
		ends,
		out_path_ptr_holder,
		out_path_member_ptr_holder,
		out_sizes
	);

	return HF::Exceptions::HF_STATUS::OK;
}

/// <summary>
/// Get the size of a path
/// </summary>
C_INTERFACE GetPathInfo(
	HF::SpatialStructures::Path* p,
	PathMember** out_member_ptr,
	int* out_size
) {
	if (p) {
		*out_size = p->size();
		*out_member_ptr = p->GetPMPointer();
		return HF::Exceptions::HF_STATUS::OK;
	}
	else {
		*out_size = -1;
		*out_member_ptr = NULL;
		return HF::Exceptions::HF_STATUS::NO_PATH;
	}
}


C_INTERFACE DestroyPath(Path* path_to_destroy) {
	DeleteRawPtr(path_to_destroy);
	return HF::Exceptions::OK;
}

C_INTERFACE CreateAllToAllPaths(
	const HF::SpatialStructures::Graph* g,
	const char* cost_type,
	HF::SpatialStructures::Path** out_path_ptr_holder,
	HF::SpatialStructures::PathMember** out_path_member_ptr_holder,
	int* out_sizes,
	int num_paths
) {

	// Create a string from cost_type
	std::string cost(cost_type);

	try {
		// Create a boost graph with the cost type
		auto bg = CreateBoostGraph(*g, cost_type);

		// Generate paths
		InsertAllToAllPathsIntoArray(bg.get(), out_path_ptr_holder, out_path_member_ptr_holder, out_sizes);
		
		// Return OK
		return HF_STATUS::OK;
	}

	// Handle the case that the cost type doesn't exist
	catch (HF::Exceptions::NoCost){
		return HF_STATUS::NO_COST;
	}

	// IF we get here something went wrong.
	return HF_STATUS::GENERIC_ERROR;
	
}

C_INTERFACE CalculateDistanceAndPredecessor(
	const Graph* g,
	const char* cost_name,
	vector<float>** out_dist_vector,
	float** out_dist_data,
	vector<int>** out_pred_vector,
	int** out_pred_data
) {

	try {
		//Try to create a boost graph from G and cost_type
		auto bg = CreateBoostGraph(*g, string(cost_name));
	
		// Calculate Distance and predecessor matricies
		DistanceAndPredecessor matricies = GenerateDistanceAndPred(*bg.get());

		// Update Output
		*out_dist_vector = matricies.dist;
		*out_dist_data = matricies.dist->data();
		*out_pred_vector = matricies.pred;
		*out_pred_data = matricies.pred->data();
	}
	catch (HF::Exceptions::NoCost){
		return HF_STATUS::NO_COST;
	}

	return HF_STATUS::OK;
}



C_INTERFACE CreateAllPredToPath(
	const HF::SpatialStructures::Graph* g,
	const char* cost_name,
	vector<int>** out_nodes_vector,
	int** out_nodes_data, // Output: Flat array of all path nodes
	vector<int>** out_lengths_vector,
	int** out_lengths_data // Output: Array of path lengths
) {

	try {

		// Create a boost graph with the cost type
		auto bg = CreateBoostGraph(*g, string(cost_name));

		// Prepare the parents and children vectors
		
		int nodeSize = bg->p.size();
		std::vector<int> start_points(nodeSize * nodeSize);
		std::vector<int> end_points(nodeSize * nodeSize);

		for (int i = 0; i < nodeSize; ++i) {
			std::fill_n(start_points.begin() + i * nodeSize, nodeSize, i);
			std::iota(end_points.begin() + i * nodeSize, end_points.begin() + (i + 1) * nodeSize, 0);
		}

		// Generate paths
		auto paths = HF::Pathfinding::FindPaths(bg.get(), start_points, end_points);
		std::vector<int> pathNodes;
		std::vector<int> pathLengths;

		// Collect all nodes and path lengths
		for (const auto& p : paths) 
		{
			for (const auto& pm : p.members) {
				pathNodes.push_back(pm.node);
			}
			pathLengths.push_back(p.members.size());
		}

		*out_nodes_vector = new vector<int>();
		*out_lengths_vector = new vector<int>();

		/*
		auto& out_nodes = **out_nodes_vector;
		auto& out_lengths = **out_lengths_vector;

		
		out_nodes.resize(pathNodes.size());
		out_lengths.resize(pathLengths.size());

		for (int i = 0; i < pathNodes.size(); i++) {
			out_nodes[i] = pathNodes[i];
		}
		for (int i = 0; i < pathLengths.size(); i++) {
			out_lengths[i] = pathLengths[i];
		}


		*out_nodes_data = out_nodes.data();
		*out_lengths_data = out_lengths.data();
		*/

		*out_nodes_vector = new vector<int>(pathNodes.begin(), pathNodes.end());
		*out_lengths_vector = new vector<int>(pathLengths.begin(), pathLengths.end());

		*out_nodes_data = (*out_nodes_vector)->data();
		*out_lengths_data = (*out_lengths_vector)->data();
	}

	catch (HF::Exceptions::NoCost) 
	{
		return HF_STATUS::GENERIC_ERROR;
	}
	return HF_STATUS::OK;

}
