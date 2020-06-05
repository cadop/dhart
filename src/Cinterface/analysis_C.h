#include <cinterface_utils.h>
#include <vector>
#include <array>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {class Graph;}
	namespace AnalysisMethods {class GraphGenerator;}
	namespace RayTracer {class EmbreeRayTracer;}
}

/**
* @defgroup GraphGenerator
* Perform a breadth first search on a mesh to find accessible space. 
* @{
*/


//---- Graph ----//
/// <summary> Construct a graph by performing a breadth first search of accessible space. </summary>
/// <param name="ray_tracer"> Raytracer containing the geometry to use for graph generation. </param>
/// <param name="start_point">  The starting point for the graph generator to begin searching from. If this isn't above solid ground, no nodes will be generated.  </param>
/// <param name="spacing">Space between nodes for each step of the search. Lower values will yield more nodes for a higher resolution graph.</param>
/// <param name="MaxNodes">
/// Stop generation after this many nodes. -1 to generate an infinite amount of nodes. Note that The final
/// count may be greater than this number.
/// </param>
/// <param name="UpStep">
/// Maximum height of a step the graph can traverse. Any steps higher this will be considered inaccessible.
/// </param>
/// <param name="UpSlope">
/// Maximum upward slope the graph can traverse in degrees. Any slopes steeper than this will be
/// considered inaccessible.
/// </param>
/// <param name="DownStep">
/// Maximum step down the graph can traverse. Any steps steeper than this will be considered inaccessible.
/// </param>
/// <param name="DownSlope">
/// The maximum downward slope the graph can traverse. Any slopes steeper than this will be
/// considered inaccessible.
/// </param>
/// <param name="max_step_connections">
/// Multiplier for number of children to generate for each node. Increasing this value will increase
/// the number of edges in the graph, and as a result the amount of memory the algorithm requires.
/// </param>
/// <param name="core_count">
/// Number of cores to use. -1 will use all available cores, and 0 will run a serialized version of
/// the algorithm.
/// </param>
/// <returns>
/// <see cref="HF_STATUS::OK"/> if the graph creation was successful. HF_STATUS::NO_GRAPH if it failed to generate
/// a graph with more than a single node.
/// </returns>
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