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

	\param		out_graph	Address of (\link HF::SpatialStructures::Graph \endlink *); address of a pointer to a \link HF::SpatialStructures::Graph \endlink.
							*(out_graph) will point to memory allocated by \link CreateVisibilityGraphAllToAll \endlink.

	\param		height		How far to offset nodes from the ground.

	\returns	HF_STATUS::OK on completion

	\see		\ref raytracer_setup (How to create a BVH), \ref raytracer_teardown (How to destroy a BVH)

	You must first <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, set up the parameters for the visibility graph.
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_setup

	Now we are ready to call \link CreateVisibilityGraphAllToAll \endlink .
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_call

	Very important: <b>after generating/adding edges to a graph -- it must be compressed.</b>
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_compress

	You are now ready to use the visibility graph. After use, its resources must be <b>relinquished</b>:
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used by the visibility graph -- <br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.
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

	\param		out_graph	Address of (\link HF::SpatialStructures::Graph \endlink *); address of a pointer to a \link HF::SpatialStructures::Graph \endlink.
							*(out_graph) will point to memory allocated by \link CreateVisibilityGraphAllToAllUndirected \endlink.

	\param		height		How far to offset nodes from the ground.

	\param		cores		CPU core count. A value of (-1) means to use all available cores on the system.

	\returns	HF_STATUS::OK on completion

	\see		\ref raytracer_setup (How to create a BVH), \ref raytracer_teardown (How to destroy a BVH)

	You must first <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, set up the parameters for the visibility graph.
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_setup

	Now we are ready to call \link CreateVisibilityGraphAllToAllUndirected \endlink .
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_call

	Very important: <b>after generating/adding edges to a graph -- it must be compressed.</b>
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_compress

	You are now ready to use the visibility graph. After use, its resources must be <b>relinquished</b>:
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used by the visibility graph -- <br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.
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

	\param		out_graph	Address of (\link HF::SpatialStructures::Graph \endlink *); address of a pointer to a \link HF::SpatialStructures::Graph \endlink.
							*(out_graph) will point to memory allocated by \link CreateVisibilityGraphGroupToGroup \endlink.

	\param		height		How far to offset nodes from the ground.

	\returns	HF_STATUS::OK on completion

	\see		\ref raytracer_setup (How to create a BVH), \ref raytracer_teardown (How to destroy a BVH)

	You must first <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, set up the parameters for the visibility graph.
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_setup

	Now we are ready to call \link CreateVisibilityGraphGroupToGroup \endlink .
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_call

	Very important: <b>after generating/adding edges to a graph -- it must be compressed.</b>
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_compress

	You are now ready to use the visibility graph. After use, its resources must be <b>relinquished</b>:
	\snippet	tests\src\visibility_graph_cinterface.cpp snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used by the visibility graph -- <br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.
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
