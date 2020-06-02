#include <cinterface_utils.h>
#include <raytracer_c.h>

namespace HF {
	namespace RayTracer {
		class EmbreeRayTracer;
	}
	namespace SpatialStructures {
		struct Node;
		class Graph;
	}
}

/**
* @defgroup VisibilityGraph
Determine which points are visibile from other points in space.
* @{
*/

/// <summary> Create a new directed visibility graph between all nodes in nodes. </summary>
/// <param name="ert"> The raytracer to fire rays from. </param>
/// <param name="nodes">
/// Nodes to use in generating the visibility graph. Every 3 floats should represent a single point
/// </param>
/// <param name="num_nodes">
/// Nodes to use in generating the visibility graph. Should be equal to a the length of nodes/3
/// since every 3 floats represents a single point.
/// </param>
/// <param name="nodes"> Nodes to use in generating the visibility graph. </param>
/// <param name="out_graph"> Output parameter for the completed graph. </param>
/// <param name="height"> How far to offset nodes from the ground. </param>
/// <returns> HS_STATUS::OK on completion. </returns>
C_INTERFACE CreateVisibilityGraphAllToAll(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph,
	float height
);

/// <summary> Create a new undirected visibility graph between all nodes in nodes. </summary>
/// <param name="ert"> The raytracer to fire rays from. </param>
/// <param name="nodes">
/// Nodes to use in generating the visibility graph. Every 3 floats should represent a single point
/// </param>
/// <param name="num_nodes">
/// Nodes to use in generating the visibility graph. Should be equal to a the length of nodes/3
/// since every 3 floats represents a single point.
/// </param>
/// <param name="nodes"> Nodes to use in generating the visibility graph. </param>
/// <param name="out_graph"> Output parameter for the completed graph. </param>
/// <param name="height"> How far to offset nodes from the ground. </param>
/// <returns> HS_STATUS::OK on completion. </returns>
C_INTERFACE CreateVisibilityGraphAllToAllUndirected(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph,
	float height,
	const int cores
);

/// <summary> Create a new visibility from the nodes in group_a to the nodes in group_b. </summary>
/// <param name="ert"> The raytracer to fire rays from. </param>
/// <param name="group_a"> Nodes to fire from. Every 3 floats should represent a single point. </param>
/// <param name="size_a">
/// Number of nodes in group_a. Should be equal to a the length of group_a/3 since every 3 floats
/// represents a single point.
/// </param>
/// <param name="group_b"> Nodes to fire to. Every 3 floats should represent a single point. </param>
/// <param name="size_b">
/// Number of nodes in group_b. Should be equal to a the length of group_b/3 since every 3 floats
/// represents a single point.
/// </param>
/// <param name="out_graph"> Output parameter for the completed graph. </param>
/// <param name="height"> How far to offset nodes from the ground. </param>
/// <returns> HS_STATUS::OK on completion. </returns>
C_INTERFACE CreateVisibilityGraphGroupToGroup(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* group_a,
	const int size_a,
	const float* group_b,
	const int size_b,
	HF::SpatialStructures::Graph** out_graph,
	float height
);

/**@}*/
