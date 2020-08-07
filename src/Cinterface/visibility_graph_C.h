/*!
	\file		visibility_graph_C.h
	\brief		Header file for functions related to creating a visibility graph

	\author		TBA
	\date		07 Aug 2020
*/
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

/*!
	\defgroup	VisibilityGraph
	Determine which points are visible from other points in space.

	@{
*/

/*!
	\brief		Create a new directed visibility graph between all nodes in parameter nodes.
	
	\param		ert			The raytracer to fire rays from

	\param		nodes		Coordinates of nodes to use in generating the visibility graph.
							Every three floats (every three members in nodes) 
							should represent a single node (point) {x, y, z}

	\param		num_nodes	Amount of nodes (points) that will be used to generate the visibility graph.
							This should be equal to (size of nodes / 3), 
							since every three floats represents a single point.

	\param		out_graph	Address of (HF::SpatialStructures::Graph *); address of a pointer to a HF::SpatialStructures::Graph.
							*(out_graph) will point to memory allocated by CreateVisibilityGraphAllToAll.

	\param		height		How far to offset nodes from the ground.

	\returns	HF_STATUS::OK on completion

	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAll
*/
C_INTERFACE CreateVisibilityGraphAllToAll(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph,
	float height
);

/*!
	\brief		Create a new undirected visibility graph between all nodes in nodes.

	\param		ert			The raytracer to fire rays from

	\param		nodes		Coordinates of nodes to use in generating the visibility graph.
							Every three floats (every three members in nodes)
							should represent a single node (point) {x, y, z}

	\param		num_nodes	Amount of nodes (points) that will be used to generate the visibility graph.
							This should be equal to (size of nodes / 3),
							since every three floats represents a single point.

	\param		out_graph	Address of (HF::SpatialStructures::Graph *); address of a pointer to a HF::SpatialStructures::Graph.
							*(out_graph) will point to memory allocated by CreateVisibilityGraphAllToAllUndirected.

	\param		height		How far to offset nodes from the ground.

	\param		cores		CPU core count. A value of (-1) means to use all available cores on the system.

	\returns	HF_STATUS::OK on completion

	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected
*/
C_INTERFACE CreateVisibilityGraphAllToAllUndirected(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph,
	float height,
	const int cores
);

/*!
	\brief		Create a new visibility graph from the nodes in group_a, into the nodes of group_b.

	\param		ert			The raytracer to fire rays from

	\param		group_a		Coordinates of nodes to fire rays from. (source node coordinates)
							Every three floats (every three members in nodes)
							should represent a single node (point) {x, y, z}

	\param		size_a		Amount of nodes (points) in group_a.
							This should be equal to (size of group_a / 3),
							since every three floats represents a single point.

	\param		group_b		Coordinates of nodes to fire rays at. (destination node coordinates)

	\param		size_b		Amount of nodes (points) in group_b.
							This should be equal to (size of group_b / 3),
							since every three floats represents a single point.

	\param		out_graph	Address of (HF::SpatialStructures::Graph *); address of a pointer to a HF::SpatialStructures::Graph.
							*(out_graph) will point to memory allocated by CreateVisibilityGraphGroupToGroup.

	\param		height		How far to offset nodes from the ground.

	\returns	HF_STATUS::OK on completion

	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup
*/
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
