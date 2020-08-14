/*!
	\file		pathfinder_C.h
	\brief		Header file for C Interface pathfinding functionality

	\author		TBA
	\date		12 Aug 2020
*/

#ifndef PATHFINDER_C_H
#define PATHFINDER_C_H

#include <cinterface_utils.h>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures { class Graph; class Path; class PathMember; }
	namespace Pathfinding { class BoostGraph; }
}

/*!
	\defgroup	Pathfinding
	Find paths between different points in a graph.

	@{

	\section path_setup Path setup
	
	\section path_teardown Path teardown
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

	\see \ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
	\see \link DestroyPath \endlink for information on deleting the path after usage.

	
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

	\see \ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
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

	\see \ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
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

	\see \ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)
	\see \link DestroyPath \endlink for information on deleting the path after usage.
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

	\par Example
	\snippet tests\src\Pathfinding.cpp EX_DistPred_C
	\snippet tests\src\Pathfinding.cpp EX_DistPred_C_2
	`Distance Matrix: [0.000000, 10.000000, 5.000000, 10.000000, 0.000000, 15.000000, -1.000000, -1.000000, 0.000000]`\n
	`Predecessor Matrix: [0, 0, 0, 1, 1, 0, -1, -1, 2]`
*/
C_INTERFACE CalculateDistanceAndPredecessor(
	const HF::SpatialStructures::Graph* g,
	const char * cost_name,
	std::vector<float>** out_dist_vector,
	float** out_dist_data,
	std::vector<int>** out_pred_vector,
	int** out_pred_data
);
	
/**@}*/

#endif /* PATHFINDER_C_H */
