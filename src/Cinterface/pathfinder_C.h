/*!
	\file		pathfinder_C.h
	\brief		Header file for C Interface pathfinding functionality

	\author		TBA
	\date		12 Aug 2020
*/

#ifndef PATHFINDER_C_H
#define PATHFINDER_C_H

#include <cinterface_utils.h>

#ifdef _WIN32
    #define C_INTERFACE extern "C" __declspec(dllexport) int
#else
    #define C_INTERFACE extern "C" __attribute__((visibility("default"))) int
#endif

namespace HF {
	namespace SpatialStructures { class Graph; class Path; class PathMember; }
	namespace Pathfinding { class BoostGraph; }
}

/*!
	\defgroup	Pathfinding
	Find paths between different points in a graph.

	@{

	\section	mesh_setup_energy_blob Mesh setup (energy blob)
	Begin by loading an .obj file:<br>

	\code
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "energy_blob_zup.obj";

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());

		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

		// Load mesh.
		// Notice that we pass the address of the loaded_obj pointer
		// to LoadOBJ. We do not want to pass loaded_obj by value, but by address --
		// so that we can dereference it and assign it to the address of (pointer to)
		// the free store memory allocated within LoadOBJ.
		const std::array<float, 3> rot { 0.0f, 0.0f, 0.0f };	// No rotation.
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}
	\endcode

	Then, review the example at \ref raytracer_setup (how to create a BVH).<br>

	\section	generate_graph Graph generation
	You will then generate a graph, using the BVH:<br>

	\code
		// Define start point.
		// These are Cartesian coordinates.
		// If not above solid ground, no nodes will be generated.
		const std::array<float, 3> start_point{ -30.0f, 0.0f, 20.0f };

		// Define spacing.
		// This is the spacing between nodes, with respect to each axis.
		// Lower values create more nodes, yielding a higher resolution graph.
		const std::array<float, 3> spacing{ 2.0f, 2.0f, 180.0f };

		// Value of - 1 will generate infinitely many nodes.
		// Final node count may be greater than this value.
		const int max_nodes = 5000;

		const int up_step = 30;					// Maximum step height the graph can traverse.
												// Steps steeper than this are considered to be inaccessible.

		const int down_step = 70;				// Maximum step down the graph can traverse.
												// Steps steeper than this are considered to be inaccessible.

		const int up_slope = 60;				// Maximum upward slope the graph can traverse (in degrees).
												// Slopes steeper than this are considered to be inaccessible.

		const int down_slope = 60;				// Maximum downward slope the graph can traverse (in degrees).
												// Slopes steeper than this are considered to be inaccessible.

		const int max_step_connections = 1;		// Multiplier for number of children to generate for each node.
												// Increasing this value increases the number of edges in the graph,
												// therefore increasing the memory footprint of the algorithm overall.
			
		const int min_connections = 1;          // The required out-degree for a node to be valid and stored.
												// This must be greater than 0 and equal or less than the total connections created from max_step_connections.
												// Default is 1. A value of 8 when max_step_connections=1 would be a grid.

		const int core_count = -1;				// CPU core count. A value of (-1) will use all available cores.

		// Generate graph.
		// Notice that we pass the address of the graph pointer into GenerateGraph.
		//
		// GenerateGraph will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
		HF::SpatialStructures::Graph* graph = nullptr;

		status = GenerateGraph(bvh, start_point.data(), spacing.data(), max_nodes,
			up_step, down_step, up_slope, down_slope,
			max_step_connections, min_connections, core_count, &graph);

		if (status != 1) {
			std::cerr << "Error at GenerateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "Generate graph ran successfully - graph stored at address " << graph << ", code: " << status << std::endl;
		}

		// Always compress the graph after generating a graph/adding new edges
		status = Compress(graph);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
	\endcode

	\section get_all_nodes Get nodes from graph
	You can retrieve the nodes from the generated graph like this:<br>

	\code
		// Get nodes.
		//
		// The address of the local variable node_vector will be passed to GetAllNodesFromGraph;
		// it will be dereferenced inside that function and assigned memory via operator new.
		//
		// We will have to call DestroyNodes on node_vector to properly release this memory.
		// node_vector_data points to the internal buffer that resides within *(node_vector).
		//
		// When we call DestroyNodes, node_vector_data's memory will also be released.
		std::vector<HF::SpatialStructures::Node>* node_vector = nullptr;
		HF::SpatialStructures::Node* node_vector_data = nullptr;

		status = GetAllNodesFromGraph(graph, &node_vector, &node_vector_data);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetAllNodesFromGraph, code: " << status << std::endl;
		}

		// Get size of node vector
		int node_vector_size = -1;
		status = GetSizeOfNodeVector(node_vector, &node_vector_size);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetSizeOfNodeVector code: " << status << std::endl;
		}

		// Print number of nodes in the graph
		const int max_node = node_vector_size - 1;	// This is the max index of *node_vector
		std::cout << "Graph Generated with " << node_vector_size << " nodes" << std::endl;
	\endcode

	\section nodes_teardown Destroy nodes from graph
	When you are finished with the graph node container, you must destroy it:<br>

	\code
		// destroy vector<Node>
		status = DestroyNodes(node_vector);

		if (status != 1) {
			std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
		}
	\endcode

	\section graph_teardown Destroy graph
	When you are finished with the graph, you must destroy it:<br>

	\code
		// destroy graph
		status = DestroyGraph(graph);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
	\endcode
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

	\see \ref mesh_setup (how to load an .OBJ file/create a mesh)
	\see \ref raytracer_setup (how to create a BVH)
	\see \ref generate_graph (how to generate a graph from a BVH)
	\see \ref get_all_nodes (how to retrieve nodes from a graph)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
	\see \ref nodes_teardown (how to destroy nodes from a graph)
	\see \ref graph_teardown (how to destroy a graph)
	\see \ref raytracer_teardown (how to destroy a BVH)
	\see \ref mesh_teardown (how to destroy a mesh)

	Begin by reviewing the example at \ref raytracer_setup to load an .obj file (mesh), and create a BVH from the mesh.<br>
	Then generate a graph (\ref generate_graph) from the BVH -- see \ref get_all_nodes on how to retrieve nodes from the graph.<br>

	Now we are ready to create a path. We will find the path from the first node to the last indexed node. <br>
	\snippet tests\src\pathfinder_C_cinterface.cpp snippet_CreatePath

	From here, please review the examples on how to destroy a path (\link DestroyPath \endlink),<br>
	as well as how to destroy a vector<\link HF::SpatialStructures::Node \endlink> (\ref nodes_teardown),<br>
	how to destroy a \link HF::SpatialStructures::Graph \endlink (\ref graph_teardown),<br>
	how to destroy a \link HF::RayTracer::EmbreeRayTracer \endlink (\ref raytracer_teardown),<br>
	and how to destroy a vector<\link HF::Geometry::MeshInfo \endlink> (\ref mesh_teardown).

	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 000001DF01EB9470, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 000001DF01EB5100, code: 1`\n
	`>>> Generate graph ran successfully - graph stored at address 000001DF0BCEDDA0, code: 1`\n
	`>>> Graph Generated with 886 nodes`\n
	`>>> CreatePath stored path successfully - path stored at address 000001DF01EB9590, code: 1`\n
	`>>> Total path cost: 77.6772`\n
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

	\see \ref mesh_setup (how to load an .OBJ file/create a mesh)
	\see \ref raytracer_setup (how to create a BVH)
	\see \ref generate_graph (how to generate a graph from a BVH)
	\see \ref get_all_nodes (how to retrieve nodes from a graph)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
	\see \ref nodes_teardown (how to destroy nodes from a graph)
	\see \ref graph_teardown (how to destroy a graph)
	\see \ref raytracer_teardown (how to destroy a BVH)
	\see \ref mesh_teardown (how to destroy a mesh)

	No cost type:<br>

	\snippet tests\src\PathFinding.cpp snippet_pathfinder_C_CreatePaths

	With a cost type:<br>

	\snippet tests\src\PathFinding.cpp snippet_pathfinder_C_CreatePathsCostType

	`>>> TODO output`\n
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

/*!
	\brief		Get the size of a path and a pointer to its path members

	\param		p					Pointer to the path to get information from. This can handle null values.
	\param		out_member_ptr		Pointer to the path to get information from. Should not be null.
	\param		out_size			The number of path members in the path. (PathMember count within *p)

	\returns	HF_STATUS::OK on success,
				HF_STATUS::NO_PATH if the path *p is invalid

	\see \ref mesh_setup (how to load an .OBJ file/create a mesh)
	\see \ref raytracer_setup (how to create a BVH)
	\see \ref generate_graph (how to generate a graph from a BVH)
	\see \ref get_all_nodes (how to retrieve nodes from a graph)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
	\see \ref nodes_teardown (how to destroy nodes from a graph)
	\see \ref graph_teardown (how to destroy a graph)
	\see \ref raytracer_teardown (how to destroy a BVH)
	\see \ref mesh_teardown (how to destroy a mesh)

	\snippet tests\src\PathFinding.cpp snippet_pathfinder_C_GetPathInfo

	`>>> TODO output`\n
*/
C_INTERFACE GetPathInfo(
	HF::SpatialStructures::Path* p,
	HF::SpatialStructures::PathMember** out_member_ptr,
	int* out_size
);

/*!
	\brief		Delete a path.

	\param		path_to_destroy		Pointer to the path to delete.

	\returns	HF_STATUS::OK on return.

	\see \ref mesh_setup (how to load an .OBJ file/create a mesh)
	\see \ref raytracer_setup (how to create a BVH)
	\see \ref generate_graph (how to generate a graph from a BVH)
	\see \ref get_all_nodes (how to retrieve nodes from a graph)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
	\see \ref nodes_teardown (how to destroy nodes from a graph)
	\see \ref graph_teardown (how to destroy a graph)
	\see \ref raytracer_teardown (how to destroy a BVH)
	\see \ref mesh_teardown (how to destroy a mesh)

	\snippet tests\src\PathFinding.cpp snippet_pathfinder_C_DestroyPath

	`>>> TODO output`\n
*/
C_INTERFACE DestroyPath(HF::SpatialStructures::Path* path_to_destroy);

/*!
	\brief	 Find a path from every node in a graph to every other node

	\param	g			The graph to conduct the search on nodes that already exist within the graph.

	\param	cost_name	The name of the cost type to use for generating paths. Leaving as an empty string
						will use the default cost of `g`.

	\param	out_size	An empty array of integers that will be updated to contain the length of every path in `path_members`.
						Paths that could not be generated will be left with a length of zero.

	\param	out_path	Location for the the path pointer array to be created.
						Paths that could not be generated will be left as null pointers.

	\param	out_data	Location for the pathmember pointer array to be created.
						All path member pointers will point to the PathMembers of the Path in paths at the same index.
						Paths that could not be generated will be left as null pointers.

	\returns	`HF_STATUS::OK` The completed successfully and all postconditions were fulfilled.
	\returns	`HF_STATUS::NO_COST` `cost_name` is not a valid cost type name

	\pre If `cost_type` is specified, `cost_type` must be the the key of an already existing cost in `g`

	\post 1) `out_path_members` will point to a vector of pointers to vectors of PathMembers with an element for every path.
			 Paths that could not be generated will be set to null pointers.
	\post 2) `out_paths` will point to a vector of pointers to paths with an element for every path. Paths that could not
			 be generated will be set to null pointers.
	\post 3) `out_sizes` will point to an array of integers containing the size of every path in `out_paths`.
		     Paths that could not be generated will have a size of 0.

	\warning
	The caller is responsible for freeing all of the memory allocated in `out_paths`and `out_sizes`. 
	The contents of `out_path_members` will automatically be deleted when the path they belong to is deleted.
	Do not try to manually delete `out_path_members` or the path that owns it will throw 
	a null pointer exception when it is deleted.

	\see \ref mesh_setup (how to load an .OBJ file/create a mesh)
	\see \ref raytracer_setup (how to create a BVH)
	\see \ref generate_graph (how to generate a graph from a BVH)
	\see \ref get_all_nodes (how to retrieve nodes from a graph)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
	\see \ref nodes_teardown (how to destroy nodes from a graph)
	\see \ref graph_teardown (how to destroy a graph)
	\see \ref raytracer_teardown (how to destroy a BVH)
	\see \ref mesh_teardown (how to destroy a mesh)

	Now we are ready to invoke \link CreateAllToAllPaths \endlink .
	\snippet tests\src\PathFinding.cpp snippet_pathfinder_C_CreateAllToAllPaths

	<br>
	\snippet \tests\src\PathFinding.cpp snippet_pathfinder_C_CreateAllToAllPaths_output
*/
C_INTERFACE CreateAllToAllPaths(
	const HF::SpatialStructures::Graph* g,
	const char * cost_type,
	HF::SpatialStructures::Path** out_path_ptr_holder,
	HF::SpatialStructures::PathMember** out_path_member_ptr_holder,
	int* out_sizes,
	int num_paths
);

/*!
	\brief	 Calculate the distance and predecessor matricies for a graph

	\param	g				The graph calculate the distance and predecessor matricies for
		
	\param	cost_name		The name of the cost type to use for generating paths. Leaving as an empty string
							will use the default cost of `g`.

	\param out_dist_vector	Pointer to be updated with a vector containing the distance matrix
	\param out_dist_data	Pointer to be updated with a pointer to the data contained by `out_dist_vector`
		
	\param out_pred_vector	Pointer to be updated with the vector containing the predecessor matrix.
	\param out_pred_data	Pointer to be updated with the data of `out_pred_vector`

	\returns `HF_STATUS::OK`		If the function completed successfully.
	\returns `HF_STATUS::NO_COST`	If `cost type` was not the key of any existing cost type in the graph.

	\post 1) `out_dist_vector` is updated to contain a pointer to the newly created distance matrix
	\post 2) `out_dist_data` is updated to contain a pointer to the data of `out_dist_vector`
	\post 3) `out_pred_vector` is updated to contain a pointer to the newly created predecessor matrix
	\post 4) `out_pred_data` is updated to contain a pointer to the data of `out_pred_vector`

	\warning
	It is the caller's responsibility to deallocate the distance and predecessor matricies by calling 
	DestroyIntVector and DestroyFloatVector. Failing to do so WILL leak memory. Do NOT attempt to delete
	the data arrays, they will automatically be deleted when their vectors are deleted. 
	
	\see \ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
	\see \link DestroyFloatVector \endlink
	\see \link DestroyIntVector \endlink
	\see \link DestroyPath \endlink for information on deleting the path after usage.

	Begin by reviewing the example at \ref raytracer_setup to load an .obj file (mesh), and create a BVH from the mesh.<br>
	Then generate a graph (\ref generate_graph) from the BVH -- see \ref get_all_nodes on how to retrieve nodes from the graph.<br>

	\par Example

	Create a graph, add some edges -- then invoke \link CalculateDistanceAndPredecessor \endlink <br>
	\snippet tests\src\PathFinding.cpp EX_DistPred_C
	
	Print the result:<br>
	\snippet tests\src\PathFinding.cpp EX_DistPred_C_2

	<br>
	`>>> Distance Matrix: [0.000000, 10.000000, 5.000000, 10.000000, 0.000000, 15.000000, -1.000000, -1.000000, 0.000000]`\n
	`>>> Predecessor Matrix: [0, 0, 0, 1, 1, 0, -1, -1, 2]\n`
*/
C_INTERFACE CalculateDistanceAndPredecessor(
	const HF::SpatialStructures::Graph* g,
	const char * cost_name,
	std::vector<float>** out_dist_vector,
	float** out_dist_data,
	std::vector<int>** out_pred_vector,
	int** out_pred_data
);
	
C_INTERFACE CreateAllPredToPath(
	const HF::SpatialStructures::Graph* g,
	const char* cost_name,
	std::vector<int>** out_nodes_vector,
	int** out_nodes_data, // Output: Flat array of all path nodes
	std::vector<int>** out_lengths_vector,
	int** out_lengths_data // Output: Array of path lengths
);

/**@}*/

#endif /* PATHFINDER_C_H */
