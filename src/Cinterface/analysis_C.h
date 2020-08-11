/*!
	\file		analysis_C.h
	\brief		Header file for C Interface functions related to graph generation

	\author		TBA
	\date		11 Aug 2020
*/

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
										and 0 will run a serialized version of the algorithm.

	\param		out_graph				Address of a (\link HF::SpatialStructures::Graph \endlink *);
										*out_graph will address heap-allocated memory to an initialized 
										\link HF::SpatialStructures::Graph \endlink on success.

	\returns	\link HF_STATUS::OK \endlink if graph creation was successful.
				\link HF_STATUS::NO_GRAPH \endlink if \link GenerateGraph \endlink failed to generate a graph with more than a single node.

	\code
		// Requires #include "analysis_C.h", #include "embree_raytracer.h", #include "graph.h"

		// Create a container of coordinates
		std::vector<std::array<float, 3>> directions = {
			{0, 0, 1},
			{0, 1, 0},
			{1, 0, 0},
			{-1, 0, 0},
			{0, -1, 0},
			{0, 0, -1},
		};

		// Create the EmbreeRayTracer
		auto ert = HF::RayTracer::EmbreeRayTracer(directions);

		// Have a pointer to Graph ready
		HF::SpatialStructures::Graph* g = nullptr;

		// Prepare parameters
		float start[] = { 0, 0, 0 };
		float spacing[] = { 1, 1, 1 };
		const int max_nodes = 6;
		const float up_step = 1.5;
		const float up_slope = 1.0;
		const float down_step = 2.0;
		const float down_slope = 0.5;
		const int maximum_step_connections = 2;
		const int cores = 4;

		if (GenerateGraph(&ert, start, spacing, max_nodes, up_step, up_slope, down_step, down_slope, maximum_step_connections, cores, &g)) {
			std::cout << "GenerateGraph successful" << std::endl;
		}
		else {
			std::cout << "GenerateGraph failed" << std::endl;
		}

		// Free memory resources once finished with Graph
		if (g) {
			delete g;
			g = nullptr;
		}
	\endcode

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

/**@}*/
