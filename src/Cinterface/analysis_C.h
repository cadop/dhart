/*!
	\file		analysis_C.h
	\brief		Header file for C Interface functions related to graph generation

	\author		TBA
	\date		11 Aug 2020
*/

#ifndef ANALYSIS_C_H
#define ANALYSIS_C_H

#include <cinterface_utils.h>
#include <vector>
#include <array>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {class Graph;}
	namespace GraphGenerator {class GraphGenerator;}
	namespace RayTracer {class EmbreeRayTracer;}
}

/*!
	\defgroup	GraphGenerator
	Perform a breadth-first search on a mesh to find accessible space.

	@{
*/

/*!
	\brief		Construct a graph by performing a breadth-first search of accessible space

	\param		ray_tracer				Raytracer containing the geometry to use for graph generation. 

	\param		start_point				The starting point for the graph generator to begin searching from. 
										If this isn't above solid ground, no nodes will be generated.

	\param		spacing					Space between nodes for each step of the search. 
										Lower values will yield more nodes for a higher resolution graph.

	\param		MaxNodes				Stop generation after this many nodes.
										A value of -1 will generate an infinite amount of nodes. 
										Note that the final node count may be greater than this value.

	\param		UpStep					Maximum height of a step the graph can traverse. 
										Any steps higher this will be considered inaccessible.

	\param		UpSlope					Maximum upward slope the graph can traverse in degrees. 
										Any slopes steeper than this will be considered inaccessible.

	\param		DownStep				Maximum step down the graph can traverse. 
										Any steps steeper than this will be considered inaccessible.

	\param		DownSlope				The maximum downward slope the graph can traverse. 
										Any slopes steeper than this will be considered inaccessible.

	\param		max_step_connection		Multiplier for number of children to generate for each node. 
										Increasing this value will increase the number of edges in the graph, 
										and as a result the amount of memory the algorithm requires.

	\param		core_count				Number of cores to use. -1 will use all available cores, 
										and 0 or 1 will run a serialized version of the algorithm.

	\param		out_graph				Address of a (\link HF::SpatialStructures::Graph \endlink *);
										*out_graph will address heap-allocated memory to an initialized 
										\link HF::SpatialStructures::Graph \endlink on success.

	\returns	\link HF_STATUS::OK \endlink if graph creation was successful.
				\link HF_STATUS::NO_GRAPH \endlink if \link GenerateGraph \endlink failed to generate a graph with more than a single node.

	\see		\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	You must <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, determine the <b>start point</b>, <b>spacing of nodes for each axis</b>, and <b>maximum nodes to generate</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_setup_0

	Then, determine the remainder of the values required by \link GenerateGraph \endlink before calling it.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph

	<b>Very important!</b> <b>Compress the graph</b> after generating a graph or adding edges.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_compress

	To output the graph to the console, we must <b>retrieve its nodes</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_GetNodes

	We are now ready to <b>output</b> the generated graph to the <b>console</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_output

	When we are finished, we must destroy <b>node_vector</b> and <b>graph</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used by the graph --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.

	<br>
	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 000002468EEBBEB0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 00000246849C59B0, code: 1`\n
	`>>> Node count: 594`\n
	`>>> [(-1, -6, 0, 0) (-1.5, -6.5, -0, 1) (-1.5, -6, -0, 2)]`\n
*/
C_INTERFACE GenerateGraph(
	HF::RayTracer::EmbreeRayTracer* ray_tracer,
	const float* start_point,
	const float* spacing,
	int MaxNodes,
	float UpStep,
	float UpSlope,
	float DownStep,
	float DownSlope,
	int max_step_connection,
	int core_count,
	HF::SpatialStructures::Graph** out_graph
);

/*!
	\brief		Construct a graph by performing a breadth-first search of accessible space, seperating obstacles from walkable geometry

	\param		ray_tracer				Raytracer containing the geometry to use for graph generation.

	\param		start_point				The starting point for the graph generator to begin searching from.
										If this isn't above solid ground, no nodes will be generated.

	\param		spacing					Space between nodes for each step of the search.
										Lower values will yield more nodes for a higher resolution graph.

	\param		MaxNodes				Stop generation after this many nodes.
										A value of -1 will generate an infinite amount of nodes.
										Note that the final node count may be greater than this value.

	\param		UpStep					Maximum height of a step the graph can traverse.
										Any steps higher this will be considered inaccessible.

	\param		UpSlope					Maximum upward slope the graph can traverse in degrees.
										Any slopes steeper than this will be considered inaccessible.

	\param		DownStep				Maximum step down the graph can traverse.
										Any steps steeper than this will be considered inaccessible.

	\param		DownSlope				The maximum downward slope the graph can traverse.
										Any slopes steeper than this will be considered inaccessible.

	\param		max_step_connection		Multiplier for number of children to generate for each node.
										Increasing this value will increase the number of edges in the graph,
										and as a result the amount of memory the algorithm requires.

	\param		core_count				Number of cores to use. -1 will use all available cores,
										and 0 or 1 will run a serialized version of the algorithm.
	
	\param		obstacle_ids			Array of geometry IDs to consider obstacles
	\param		walkable_ids			Array of geometry IDs to consider as walkable surfaces
	\param		num_obstacles			number of elements in `obstacle_ids`
	\param		num_walkables			number of elements in `walkable_ids`

	\param		out_graph				Address of a (\link HF::SpatialStructures::Graph \endlink *);
										*out_graph will address heap-allocated memory to an initialized
										\link HF::SpatialStructures::Graph \endlink on success.

	\returns	\link HF_STATUS::OK \endlink if graph creation was successful.
				\link HF_STATUS::NO_GRAPH \endlink if \link GenerateGraph \endlink failed to generate a graph with more than a single node.

	\see		\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	You must <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, determine the <b>start point</b>, <b>spacing of nodes for each axis</b>, and <b>maximum nodes to generate</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_setup_0

	Then, determine the remainder of the values required by \link GenerateGraph \endlink before calling it.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph

	<b>Very important!</b> <b>Compress the graph</b> after generating a graph or adding edges.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_compress

	To output the graph to the console, we must <b>retrieve its nodes</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_GetNodes

	We are now ready to <b>output</b> the generated graph to the <b>console</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_output

	When we are finished, we must destroy <b>node_vector</b> and <b>graph</b>.<br>
	\snippet tests\src\analysis_C_cinterface.cpp snippet_analysis_cinterface_GenerateGraph_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used by the graph --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.

	<br>
	`>>> LoadOBJ loaded mesh successfully into loaded_obj at address 000002468EEBBEB0, code: 1`\n
	`>>> CreateRaytracer created EmbreeRayTracer successfully into bvh at address 00000246849C59B0, code: 1`\n
	`>>> Node count: 594`\n
	`>>> [(-1, -6, 0, 0) (-1.5, -6.5, -0, 1) (-1.5, -6, -0, 2)]`\n
*/
C_INTERFACE GenerateGraphObstacles(
	HF::RayTracer::EmbreeRayTracer* ray_tracer,
	const float* start_point,
	const float* spacing,
	int MaxNodes,
	float UpStep,
	float UpSlope,
	float DownStep,
	float DownSlope,
	int max_step_connection,
	int core_count,
	const int* obstacle_ids,
	const int* walkable_ids,
	int num_obstacles,
	int num_walkables,
	HF::SpatialStructures::Graph** out_graph
);

/**@}*/

#endif /* ANALYSIS_C_H */
