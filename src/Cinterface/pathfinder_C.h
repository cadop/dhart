#include <cinterface_utils.h>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures { class Graph; class Path; class PathMember; }
	namespace Pathfinding { class BoostGraph; }
}

/**
* @defgroup Pathfinding
Find paths between differnt points in a graph.
* @{
*/

/// <summary>
/// Create a path from start to end in C++. If no path exists, the NO_PATH error code will be
/// returned, and both pointers will be left as they were before the function was called.
/// </summary>
/// <param name="g"> The graph to conduct the search on. </param>
/// <param name="start">
/// The id of the node for the starting point of the path. The ID must belong to a node already
/// within the graph!
/// </param>
/// <param name="end">
/// The id of the node for the end point of the path. The ID must belong to a node already within
/// the graph!
/// </param>
/// <param name="out_path"> Return parameter for the path object. </param>
/// <param name="out_data"> Return parameter for the path's members. </param>
/// <returns>
/// HF_STATUS::NO_PATH if a path could not be generated. HF_STATUS::OK if a path was generated.
/// </returns>
/// <remarks> Uses the Boost Graph Library to perform Dijkstra's shortest path algorithm. For multiple paths, use <see cref="CreatePaths"/> for multiple paths at once. </remarks>
/*!
	\code
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		// Prepare parameters for CreatePath
		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		CreatePath(&g, 0, 4, &out_size, &out_path, &out_path_member);

		// Use out_path, out_path_member

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	\endcode
*/
C_INTERFACE CreatePath(
	const HF::SpatialStructures::Graph* g,
	int start,
	int end,
	int* out_size,
	HF::SpatialStructures::Path** out_path,
	HF::SpatialStructures::PathMember** out_data
);

/// <summary>
/// Create multiple paths from start to end in C++. both out_ members return as arrays of pointers,
/// with one for each element. Sizes of zero represent non-existant paths, and will be set to null pointers
/// </summary>
/// <param name="g"> The graph to conduct the search on. </param>
/// <param name="start">
/// An array of ids for starting nodes. Length must match that of end and all the IDS must belong to
/// nodes that already exist within the graph.
/// </param>
/// <param name="end">
/// An array of ids for ending nodes. Length must match that of end and all the IDS must belong to
/// nodes that already exist within the graph.
/// </param>
/// <param name="out_path_ptr_holder"> Return parameter for path objects. </param>
/// <param name="out_data"> Return parameter for pointers to the path objects' underlying data. </param>
/// <param name="out_sizes">
/// Output array of integers representing the length of of each path in out_data's arrays. Sizes of
/// 0 indicate that no path could be generated.
/// </param>
/// <param name="num_paths">Size of start and end arrays</param>
/// <returns> HF::OK on completion. </returns>

/*!
	\code
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		// Prepare starting and ending nodes
		int start_nodes[] = { 0, 0, 1, 2, 3 };
		int end_nodes[] = { 1, 2, 3, 4, 4 };

		// Prepare parameters for CreatePaths
		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;
		const int path_size = 5;

		// Sizes of paths
		int out_sizes[] = { 1, 2, 3, 1, 5 };

		CreatePaths(&g, start_nodes, end_nodes, &out_path, &out_path_member, out_sizes, path_size);

		// Use out_path, out_path_member

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	\endcode
*/
C_INTERFACE CreatePaths(
	const HF::SpatialStructures::Graph* g,
	const int* start,
	const int* end,
	HF::SpatialStructures::Path** out_path_ptr_holder,
	HF::SpatialStructures::PathMember** out_path_member_ptr_holder,
	int* out_sizes,
	int num_paths
);

/// <summary> Get the size of a path and a pointer to its path members. </summary>
/// <param name="p"> Pointer to the path to get information from. This can handle null values. </param>
/// <param name="out_member_ptr"> Pointer to the path to get information from. Should not be null. </param>
/// <param name="out_size"> The number of path members in the path. </param>
/// <returns> HF_STATUS::NO_PATH if the path is not valid. HF_OK otherwise. </returns>

/*!
	\code
		// Requires #include "pathfinder_C.h", #include "path.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		CreatePath(&g, 0, 4, &out_size, &out_path, &out_path_member);

		// Get p's info, store results in out_path_member and out_size
		GetPathInfo(p, &out_path_member, &out_size);

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	\endcode
*/
C_INTERFACE GetPathInfo(
	HF::SpatialStructures::Path* p,
	HF::SpatialStructures::PathMember** out_member_ptr,
	int* out_size
);

/// <summary> Delete a path. </summary>
/// <param name="path_to_destroy"> Pointer to the path to delete. </param>

/*!
	\code
		// Requires #include "pathfinder_C.h", #include "graph.h", #include "path.h", #include "path_finder.h"

		// Create a Graph g, and compress it.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(2, 4, 1);
		g.addEdge(3, 4, 5);
		g.Compress();

		// Create a boostGraph from g
		auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		CreatePath(&g, 0, 4, &out_size, &out_path, &out_path_member);

		// Use out_path, out_path_member

		// Remember to free resources when finished
		DestroyPath(out_path);

		// At this point, out_path_member has also been destroyed, so we set this to nullptr
		out_path_member = nullptr;
	\endcode
*/
C_INTERFACE DestroyPath(HF::SpatialStructures::Path* path_to_destroy);

/**@}*/
