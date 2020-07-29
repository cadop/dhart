#include <pathfinder_C.h>

#include <memory>
#include <vector>

#include <HFExceptions.h>
#include <graph.h>
#include <path_finder.h>
#include <path.h>

using std::unique_ptr;
using std::make_unique;
using std::vector;

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