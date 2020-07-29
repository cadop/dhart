#include <cinterface_utils.h>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures { class Graph; struct Path; struct PathMember; }
	namespace Pathfinding { class BoostGraph; }
}

/**
* @defgroup Pathfinding
Find paths between differnt points in a graph.
* @{
*/

/*!

	\brief	 Find the shortest path from start to end.

	\param	g			The graph to conduct the search on.
	\param	start		Start node of the path.
	\param	end			End node of the path.

	\param	cost_name	The name of the cost in `g` to use for shortest path calculations. Set to an empty string
						to use the cost `g` was constructed with.

	\param	out_size	Updated to the length of the found path on success. Set to 0 if no path could be found.
	\param	out_path	Output parameter for a pointer to the generated path. Will be null if no path could be found
	\param	out_data	Output parameter for a pointer to the data of the generated path. Will be null if no path could be found.

	\returns			`HF_STATUS::OK` The function completed successfully.
	\returns			`HF_STATUS::NO_PATH` No path could be found
	\returns			`HF_STATUS::NO_COST` `cost_name` is not an empty string or the key of a cost that already exists in G

	\pre 1) `start` and `end` contain both contain the Ids of nodes already in the graph
	\pre 2) If not set to the empty string, `cost_name` is the key to a valid cost type already defined in `g`.

	\post If `OK` is returned, a path between start and end was found and `out_size`, `out_path`, and `out_data`,
	are updated to contain the number of nodes in the path, a pointer to the path itself, and a pointer to the
	PathMembers it holds respectively.

	\warning
	The caller is responsible for deleting the path returned by out_path by calling DestroyPath
	if this function completes successfully. Freeing the memory for a path will also free the memory
	for its path members, so do not attempt to access its members after deletion.

	\see DestroyPath for information on deleting the path after usage.

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
	const char * cost_type,
	int* out_size,
	HF::SpatialStructures::Path** out_path,
	HF::SpatialStructures::PathMember** out_data
);


/*!
	\brief	 Find multiple shortest paths in paralllel.

	\param	g			The graph to conduct the search on
	\param	start		An array of ids for starting nodes.
						Length must match that of end and all the IDS must belong to
						nodes that already exist within the graph.
	\param	end			An array of ids for ending nodes.
						Length must match that of start and all the IDS must belong to
						nodes that already exist within the graph.
	\param	cost_name	The name of the cost type to use for generating paths. Leaving as an empty string
						will use the default cost of `g`.
	\param	out_size	An empty array of integers that will be updated to contain the length of every path in `path_members`.
						Paths that could not be generated will be left with a length of zero.
	\param	out_path	Location for the the path pointer array to be created. Paths that could not be
						generated will be left as null pointers.
	\param	out_data	Location for the pathmember pointer array to be created. All path member pointers will
						point to the PathMembers of the Path in paths at the same index. Paths that could not
						be generated will be left as null pointers.

	\returns			`HF_STATUS::OK` if the function completes successfully
	\returns			`HF_STATUS::NO_COST` is `cost_name` is not a valid cost type name

	\pre 1) The length of `start_ids`, `end_ids`, and `out_size` must be equal.
	\pre 2) If `cost_type` is specified, `cost_type` must be the the key of an already existing cost in `g`

	\post 1) `out_path_members` will point to a vector of pointers to vectors of PathMembers with an element for every path.
			 paths that could not be generated will be set to null pointers.
	\post 2) `out_paths` will point to a vector of pointers to paths with an element for every path. Paths that could not
			 be generated will be set to null pointers.
	\post 3) `out_sizes` will point to an array of integers containing the size of every path in `out_paths`.
			Paths that could not be generated will have a size of 0.

	\warning
	The caller is responsible for freeing all of the memory allocated in `out_paths` and `out_sizes`. The contents of
	`out_path_members` will automatically be deleted when the path they belong to is deleted. Do not try
	to manually delete `out_path_members` or the path that owns it will throw a null pointer exception
	when it is deleted.

	\code

		// for brevity
		using HF::SpatialStructures::Node;
		using HF::SpatialStructures::Graph;
		using HF::Pathfinding::BoostGraph;
		using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;

		// Create the nodes
		Node node_0(1.0f, 1.0f, 2.0f);
		Node node_1(2.0f, 3.0f, 4.0f);
		Node node_2(11.0f, 22.0f, 140.0f);
		Node node_3(62.9f, 39.1f, 18.0f);
		Node node_4(19.5f, 27.1f, 29.9f);

		// Create a graph. No nodes/edges for now.
		Graph graph;

		// Add edges. These will have the default edge values, forming the default graph.
		graph.addEdge(node_0, node_1, 1);
		graph.addEdge(node_0, node_2, 2.5);
		graph.addEdge(node_1, node_3, 54.0);
		graph.addEdge(node_2, node_4, 39.0);
		graph.addEdge(node_3, node_4, 1.2);

		// Always compress the graph after adding edges!
		graph.Compress();

		// Retrieve a Subgraph, parent node ID 0 -- of alternate edge costs.
		// Add these alternate edges to graph.
		std::string desired_cost_type = AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE);
		auto edge_set = CalculateEnergyExpenditure(graph.GetSubgraph(0));
		graph.AddEdges(edge_set, desired_cost_type);

		// Prepare parameters for CreatePath
		HF::SpatialStructures::Path* out_path = nullptr;
		HF::SpatialStructures::PathMember* out_path_member = nullptr;
		int out_size = -1;

		// Use CreatePathCostType, be sure to use the .c_str() method if using a std::string for desired_cost_type
		CreatePath(&graph, 0, 4, desired_cost_type.c_str(), &out_size, &out_path, &out_path_member);

		///
		/// Use out_path, out_path_member
		///

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
	const char * cost_type,
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
		g.addEdge(2, 4, 1);a
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


/// <summary>
/// Create multiple paths from start to end in C++. both out_ members return as arrays of pointers,
/// with one for each element. Sizes of zero represent non-existant paths, and will be set to null pointers
/// </summary>
/// <param name="g"> The graph to conduct the search on. </param>
/// <param name="out_path_ptr_holder"> Return parameter for path objects. </param>
/// <param name="out_path_member_ptr_holder"> Return parameter for pointers to the path objects' underlying data. </param>
/// <param name="out_sizes">
/// Output array of integers representing the length of of each path in out_data's arrays. Sizes of
/// 0 indicate that no path could be generated.
/// </param>
/// <param name="num_paths">Size of start and end arrays</param>
/// <returns> HF::OK on completion. </returns>

/*!
	\code
		HF::SpatialStructures::Graph g;

		// Add the edges
		g.addEdge(0, 1, 1);
		g.addEdge(0, 2, 2);
		g.addEdge(1, 3, 3);
		g.addEdge(1, 4, 4);
		g.addEdge(2, 4, 4);
		g.addEdge(3, 5, 5);
		g.addEdge(4, 6, 3);
		g.addEdge(5, 6, 1);

		// Always compress the graph after adding edges
		g.Compress();

		// Create a BoostGraph (std::unique_ptr)
		auto bg = CreateBoostGraph(g);

		// Total paths is node_count ^ 2
		size_t node_count = g.Nodes().size();
		size_t path_count = node_count * node_count;

		// Pointer to buffer of (Path *)
		Path** out_paths = new Path * [path_count];
		// out_paths[i...path_count - 1] will be alloc'ed by InsertPathsIntoArray

		// Pointer to buffer of (PathMember *)
		PathMember** out_path_member = new PathMember * [path_count];
		// out_path_member[i...path_count - 1] points to out_paths[i...path_count - 1]->GetPMPointer();

		// Pointer to buffer of (int)
		int* sizes = new int[path_count];

		//
		// The two loops for start_points and end_points
		// are just for the output.
		//
		int curr_id = 0;
		std::vector<int> start_points(path_count);
		// Populate the start points,
		// size will be (node_count)^2
		for (int i = 0; i < node_count; i++) {
			for (int k = 0; k < node_count; k++) {
				start_points[curr_id++] = i;
			}
		}

		curr_id = 0;

		std::vector<int> end_points(path_count);
		// Populate the end points,
		// size will be (node_count)^2
		for (int i = 0; i < node_count; i++) {
			for (int k = 0; k < node_count; k++) {
				end_points[curr_id++] = k;
			}
		}

		CreateAllToAllPaths(&g, out_paths, out_path_member, sizes, path_count);

		for (int i = 0; i < path_count; i++) {
			if (out_paths[i]) {
				// Always check if out_paths[i] is nonnull!
				int total_cost = 0;
				std::cout << "Path from " << start_points[i] << " to " << end_points[i] << std::endl;

				Path p = *out_paths[i];
				for (auto m : p.members) {
					total_cost += m.cost;
					std::cout << "node ID: " << m.node << "\tcost " << m.cost << std::endl;
				}

				std::cout << "Total cost: " << total_cost << std::endl;
				std::cout << "--------------------------" << std::endl;
			}
		}

		//
		// Resource cleanup
		//
		if (sizes) {
			delete[] sizes;
			sizes = nullptr;
		}

		if (out_path_member) {
			delete[] out_path_member;
			out_path_member = nullptr;
		}

		if (out_paths) {
			for (int i = 0; i < path_count; i++) {
				if (out_paths[i]) {
					delete out_paths[i];
					out_paths[i] = nullptr;
				}
			}
		}
	}
	\endcode
*/
C_INTERFACE CreateAllToAllPaths(
	const HF::SpatialStructures::Graph* g,
	HF::SpatialStructures::Path** out_path_ptr_holder,
	HF::SpatialStructures::PathMember** out_path_member_ptr_holder,
	int* out_sizes,
	int num_paths
);

/**@}*/
