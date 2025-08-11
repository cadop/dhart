/*!
	\file		spatialstructures_C.h
	\brief		Header file related to manipulating nodes, edges, and graphs via CInterface

	\author		TBA
	\date		19 Aug 2020
*/

#include <cinterface_utils.h>
#include <vector>
#include <array>

/*!
	\define		C_INTERFACE
	\brief		Specifies C linkage for functions defined in the C Interface.
				Used for exporting C Interface functions from the DHART DLL.
*/
#ifdef _WIN32
	#define C_INTERFACE extern "C" __declspec(dllexport) int
#else
    #define C_INTERFACE extern "C" __attribute__((visibility("default"))) int
#endif
namespace HF {
	namespace SpatialStructures {
		class Graph;
		struct Subgraph;

		class Path;

		enum class COST_AGGREGATE : int;

		struct Node;	// Careful with these forward declares!
		struct Edge;	// Spent like 2 hours reading mangled names 
						// because I accidentally defined these 
						// as classes (V) instead of structs (U)

		enum class Direction : int;
	}
}

/*!
	\enum		COST_ALG_KEY
	\brief		Indices of keys for costs returned from calling CalculateAndStore functions
*/
enum COST_ALG_KEY {
	CROSS_SLOPE,		///< Cost created by CalculateAndStoreCrossSlope.
	ENERGY_EXPENDITURE	///< Cost created by CalculateAndStoreEnergyExpenditure.
};

/*!
	\var	Key_To_Costs
	\brief	Keys of costs for calling CalculateAndStore functions
*/
const std::vector<std::string> Key_To_Costs {
	"CrossSlope",
	"EnergyExpenditure"
};

/*!
	\brief		Get the cost algorithm title (as std::string) 
				from the COST_ALG_KEY enum member

	\param		key		Enum member representing algorithm cost type:<br>
						\link COST_ALG_KEY::CROSS_SLOPE \endlink <br>
						\link COST_ALG_KEY::ENERGY_EXPENDITURE \endlink<br>
						are valid parameters.

	\return		A human-readable string representation of key.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AlgorithmCostTitle

	<br>
	\verbatim
	Cost type: CrossSlope
	\endverbatim
*/
inline std::string AlgorithmCostTitle(COST_ALG_KEY key) {
	return Key_To_Costs[key];
}

/*!
	\defgroup	SpatialStructures
	Contains the Graph and Node data types.

	@{

	\section graph_manipulation Graph Manipulation

	Although graphs can be generated (see \ref generate_graph),<br>
	they can <b>also be created by objects instantiated or obtained by the user</b>.

	\subsection	graph_setup				Graph setup
	Call \link CreateGraph \endlink :
	\code
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}
	\endcode

	\subsection	graph_add_edge_from_nodes		Adding edges from nodes
	Call \link AddEdgeFromNodes \endlink :
	\code
		// Create three nodes, in the form of { x, y, z } coordinates
		std::array<float, 3> n0 { 0, 0, 0 };
		std::array<float, 3> n1 { 0, 1, 2 };
		std::array<float, 3> n2 { 0, 1, 3 };

		// Leave cost_type blank. We do not need a specific cost type now.
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), 1, cost_type);
		status = AddEdgeFromNodes(g, n0.data(), n2.data(), 2, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n0.data(), 3, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n2.data(), 4, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n0.data(), 5, cost_type);
	\endcode


	\subsection	graph_add_edge_from_node_ids	Adding edges from node IDs
	Call \link AddEdgeFromNodeIds \endlink :
	\code
		const int node_id_0 = 2;
		const int node_id_1 = 1;
		const float edge_weight = 5;

		const char* cost_type = "";

		status = AddEdgeFromNodeIDs(g, node_id_0, node_id_1, edge_weight, cost_type);
	\endcode

	\subsection	graph_compress	Compressing the graph
	\code
		status = Compress(g);
	\endcode

	\subsection	graph_get_csr_pointers	Retrieve a CSR of the graph
	Call \link GetCSRPointers \endlink :
	\code
		// Retrieve the CSR from the graph
		HF::SpatialStructures::CSRPtrs csr;
		status = GetCSRPointers(g, &csr.nnz, &csr.rows, &csr.cols, &csr.data, &csr.inner_indices, &csr.outer_indices, cost_type);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		if (status != 1) {
			std::cerr << "Error at GetCSRPointers, code: " << status << std::endl;
		}
		else {
			std::cout << "GetCSRPointers ran successfully on graph addressed at " << g << ", code: " << status << std::endl;
		}
	\endcode

	<br>
	When you are finished with a graph, be sure to destroy it.<br>
	(see \ref graph_teardown)
	<br>

	\section destroy_node_edge_containers Destroying containers returned by graph operations

	Graph operations deal in std::vector<\link HF::SpatialStructures::Node \endlink> *<br>
	and std::vector<\link HF::SpatialStructures::Edge \endlink> *.

	\subsection	node_vector_teardown		Destroying a vector of node
	Call \link DestroyNodes \endlink :
	\code
		// Destroy vector<Node>
		status = DestroyNodes(node_vec);

		if (status != 1) {
			std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyNodes ran successfully on address " << node_vec << ", code: " << status << std::endl;
		}
	\endcode

	\subsection	edge_vector_teardown		Destroying a vector of edge
	Call \link DestroyEdges \endlink :
	\code
		// Destroy vector<Edge>
		status = DestroyEdges(edge_vec);

		if (status != 1) {
			std::cerr << "Error at DestroyEdges, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyEdges ran successfully on address " << edge_vec << ", code: " << status << std::endl;
		}
	\endcode
*/

/*!
	\brief		Get a vector of every node in the given graph pointer
	
	\param		graph			Graph to retrieve nodes from
	\param		out_vector_ptr	Output parameter for the new vector
	\param		out_data_ptr	Output parameter for the vector's data
	
	\returns	\link HF_STATUS::INVALID_PTR \endlink if the given pointer was invalid.
				\link HF_STATUS::GENERIC_ERROR \endlink if the graph is not valid.
				\link HF_STATUS::OK \endlink if successful.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref node_vector_teardown (how to destroy a vector of node)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>
	
	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetAllNodesFromGraph
	
	When you are finished with the node vector,<br>
	it must be destroyed. (\ref node_vector_teardown)

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetAllNodesFromGraph(
	const HF::SpatialStructures::Graph* graph,
	std::vector<HF::SpatialStructures::Node>** out_vector_ptr,
	HF::SpatialStructures::Node** out_data_ptr
);

/*!
	\brief		Get a vector of edges every node in the given graph pointer

	\param		graph				Graph to retrieve edges from
	\param		node				Node within graph to retrieve edges for
	\param		out_vector_ptr		Output parameter for retrieved edges
	\param		out_edge_list_ptr	Address of pointer to *out_vector_ptr's internal buffer
	\param		out_edge_list_size	Will store out_vector_ptr->size()

	\returns	\link HF_STATUS::OK \endlink on completion.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref edge_vector_teardown (how to destroy a vector of edge)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	When you are finished with the edge vector,<br>
	it must be destroyed. (\ref edge_vector_teardown)

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetEdgesForNode(
	const HF::SpatialStructures::Graph* graph,
	const HF::SpatialStructures::Node* Node,
	std::vector<HF::SpatialStructures::Edge>** out_vector_ptr,
	HF::SpatialStructures::Edge** out_edge_list_ptr,
	int* out_edge_list_size
);

/*!
	\brief		Get the size of a node vector

	\param		node_list	Node vector to get the size from
	\param		out_size	Size of the vector will be written to *out_size
	
	\returns	\link HF_STATUS::OK \endlink on completion.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref node_vector_teardown (how to destroy a vector of node)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetSizeOfNodeVector

	When you are finished with the node vector,<br>
	it must be destroyed. (\ref node_vector_teardown)

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetSizeOfNodeVector(
	const std::vector<HF::SpatialStructures::Node>* node_list,
	int* out_size
);

/*!
	\brief		Get the size of an edge vector

	\param		edge_list	Edge vector to get the size from
	\param		out_size	Size of the vector will be written to *out_size

	\returns	\link HF_STATUS::OK \endlink on completion

	\deprecated	This is never used. Do not include.
*/
C_INTERFACE GetSizeOfEdgeVector(
	const std::vector<HF::SpatialStructures::Edge>* edge_list,
	int* out_size
);


/*! 
	\brief		Get the cost of traversing from `parent` to `child`
	
	\param	g			The graph to traverse
	\param	parent		ID of the node being traversed from
	\param	child		ID of the node being traversed to
	\param	cost_type	name of the cost type to get the cost from
	\param	out_float	Output parameter for the cost of traversing from parent to child

	\pre		g is a valid graph.

	\post 
	`out_float` is updated with the cost of traversing from parent to child. If no
	edge exists between parent and child, -1 will be inserted into out_float.

	\returns \link HF_STATUS::OK \endlink on success
	\returns \link HF_STATUS::NO_COST \endlink if there was no cost with cost_name

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetEdgeCost(
	const HF::SpatialStructures::Graph * g,
	int parent,
	int child,
	const char * cost_type,
	float* out_float
);

/*!
	\brief		Get an ordered array of costs for each node 
				aggregated by the desired function.

	\param		graph			Graph to aggregate edges from
	\param		agg				Aggregation type to use

	\param		directed		If true, only consider edges for a node --
								otherwise, consider both outgoing and incoming edges.

	\param		cost_type		Node cost type string; type of cost to use for the graph
	\param		out_vector_ptr	Output parameter for the vector
	\param		out_data_ptr	Output parameter for the vector's internal buffer

	\returns	\link HF_STATUS::OK \endlink if successful.
	\returns	\link HF_STATUS::NOT_COMPRESSED \endlink if the graph wasn't compressed.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AggregateCosts

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE AggregateCosts(
	const HF::SpatialStructures::Graph* graph,
	int agg,
	bool directed,
	const char* cost_type,
	std::vector<float>** out_vector_ptr,
	float** out_data_ptr
);

/*!
	\brief		Create a new empty graph

	\param		nodes	(<b>unused parameter</b>) The node IDs to create nodes from
	\param		num_nodes (<b>unused parameter</b>) Size of nodes array
	\param		out_graph	Output parameter to store the graph in

	\returns	\link HF_STATUS::OK \endlink on completion.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_compress (how to compress a graph)
	\see \ref graph_teardown (how to destroy a graph)
*/
C_INTERFACE CreateGraph(
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph
);

/*!
	\brief		Add an edge between parent and child.
				If parent or child does not already exist in the graph,
				they will be added and automatically assigned new IDs.

	\param		graph		Graph to add the new edge to

	\param		parent		A parent node structure

	\param		child		A child node structure

	\param		score		The edge cost from parent to child

	\param		cost_type	Edge cost type

	\returns \link HF_STATUS::OK \endlink on success.

	\returns \link HF_STATUS::NOT_COMPRESSED \endlink Tried to add an edge to an alternate cost type
									   when the graph wasn't compressed.

	\returns \link HF_STATUS::OUT_OF_RANGE \endlink Tried to add an edge to an alternate cost
										  that didn't already exist in the default graph.

	\pre cost_type MUST be a valid delimited char array.
				   If the entire program crashes when this is called, this is why.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)
*/

C_INTERFACE AddEdgeFromNodeStructs(
	HF::SpatialStructures::Graph* graph,
	HF::SpatialStructures::Node* parent,
	HF::SpatialStructures::Node* child,
	float score,
	const char* cost_type
);

/*!
	\brief		Add an edge between parent and child.
				If parent or child does not already exist in the graph,
				they will be added and automatically assigned new IDs.

	\param		graph		Graph to add the new edge to

	\param		parent		A three-element array of float 
							containing the x, y, and z coordinates of the parent node

	\param		child		A three-element array of float
							containing the x, y, and z coordinates of the child node.

	\param		score		The edge cost from parent to child

	\param		cost_type	Edge cost type

	\returns \link HF_STATUS::OK \endlink on success.

	\returns \link HF_STATUS::INVALID_PTR \endlink on an invalid parent or child node.

	\returns \link HF_STATUS::NOT_COMPRESSED \endlink Tried to add an edge to an alternate cost type
									   when the graph wasn't compressed.

	\returns \link HF_STATUS::OUT_OF_RANGE \endlink Tried to add an edge to an alternate cost 
										  that didn't already exist in the default graph.

	\pre cost_type MUST be a valid delimited char array. 
				   If the entire program crashes when this is called, this is why. 

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)
*/
C_INTERFACE AddEdgeFromNodes(
	HF::SpatialStructures::Graph* graph,
	const float* parent,
	const float* child,
	float score,
	const char * cost_type
);

/*!
	\brief		Create a new edge between parent_id and child_id.
				If these IDs do not exist in the graph, they will be added.

	\param		graph		Graph to create the new edge in
	\param		parent		The parent's ID in the graph
	\param		child		The child's ID in the graph
	\param		score		The cost from parent to child.
	\param		cost_type	The type of cost to add this edge to.

	\returns \link HF_STATUS::OK \endlink on completion. 

	\returns \link HF_STATUS::NOT_COMPRESSED \endlink if an alternate cost was added 
								    without first compressing the graph

	\returns \link HF_STATUS::NO_COST \endlink The given cost string was invalid. 

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)
*/
C_INTERFACE AddEdgeFromNodeIDs(
	HF::SpatialStructures::Graph* graph,
	int parent_id,
	int child_id,
	float score,
	const char * cost_type
);

/*!
	\brief		Retrieve all information for a graph's CSR representation.
				This will compress the graph if it was not already compressed.

	\param		out_nnz			Number of non-zero values contained within the CSR
	\param		out_num_rows	Number of rows contained within the CSR
	\param		out_num_cols	Number of columns contained within the CSR
	\param		out_data_ptr	Pointer to the CSR's data array
	\param		out_inner_indices_ptr	Pointer to the graph's inner indices array (columns)
	\param		out_outer_indices_ptr	Pointer to the graph's outer indices array (rpws)
	\param		cost_type		Cost type to compress the CSR with.

	\returns	\link HF_STATUS::OK \endlink on success.
	\returns	\link HF_STATUS::NO_COST \endlink if the asked for cost doesn't exist.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)
*/
C_INTERFACE GetCSRPointers(
	HF::SpatialStructures::Graph* graph,
	int* out_nnz,
	int* out_num_rows,
	int* out_num_cols,
	float** out_data_ptr,
	int** out_inner_indices_ptr,
	int** out_outer_indices_ptr,
	const char* cost_type
);

/*!
	\brief		Get the ID of the given node in the graph.
				If the node does not exist,
				<paramref name="out_id"/> will be set to -1.

	\param		graph		The graph to get the ID from

	\param		point		A three-element array of float containing the
							x, y, and z coordinates of the point to get the ID for.

	\param		out_id		Output parameter for the ID.
							Set to -1 if <paramref name="point"> 
							could not be found in <paramref name="graph"/>.

	\returns	\link HF_STATUS::OK \endlink on completion.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeID

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetNodeID(
	HF::SpatialStructures::Graph* graph,
	const float* point,
	int* out_id
);

/*!
	\brief		Compress the given graph into a CSR representation.

	\param		graph	The graph to compress into a CSR.

	\returns	\link HF_STATUS::OK \endlink on completion.

	\remarks	This will reduce the memory footprint of the graph,
				and invalidate all existing CSR representation of it.
				If the graph is already compressed, this function will be a no-op.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)
*/
C_INTERFACE Compress(
	HF::SpatialStructures::Graph* graph
);

/*!
	\brief		Clear the nodes/edges for the given graph,
				or clear a specific cost type.

	\param		graph		Graph to clear nodes from
	\param		cost_type	Edge cost type string (if clearing a specific cost type).

	\param cost_type If blank, delete the graph, otherwise only clear the cost at this type.

	\returns \link HF_STATUS::OK \endlink if the operation succeeded
	\returns \link HF_STATUS::NO_COST \endlink if a cost was specified and it couldn't be found.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_ClearGraph

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE ClearGraph(
	HF::SpatialStructures::Graph* graph,
	const char* cost_type
);

/*!
	\brief		Delete the vector of nodes at the given pointer

	\param		nodelist_to_destroy		Vector of nodes to destroy

	\returns	\link HF_STATUS::OK \endlink on completion.

	\see \ref node_vector_teardown (how to destroy a vector of node)
*/
C_INTERFACE DestroyNodes(
	std::vector<HF::SpatialStructures::Node>* nodelist_to_destroy
);

/*!
	\brief		Delete the vector of edges at the given pointer.

	\param		edgelist_to_destroy		Vector of nodes to destroy

	\returns	\link HF_STATUS::OK \endlink on completion.

	\see \ref edge_vector_teardown (how to destroy a vector of edge)
*/
C_INTERFACE DestroyEdges(
	std::vector<HF::SpatialStructures::Edge>* edgelist_to_destroy
);

/*!
	\brief		Delete a graph.

	\param		graph_to_destroy		Graph to delete

	\returns	\link HF_STATUS::OK \endlink on completion

	\see \ref graph_teardown (how to destroy a graph)
*/
C_INTERFACE DestroyGraph(
	HF::SpatialStructures::Graph* graph_to_destroy
);

/*!
	\brief		Calculates cross slope for all subgraphs in *g
	\param		g	The graph to calculate cross slope on

	\returns	\link HF_STATUS::OK \endlink on completion

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_CalculateAndStoreCrossSlope

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE CalculateAndStoreCrossSlope(HF::SpatialStructures::Graph* g);

/*!
	\brief		Calculates energy expenditure for all subgraphs in *g 
				and stores them in the graph at AlgorithmCostTitle(ALG_COST_KEY::EnergyExpenditure)

	\param		g		The address of a Graph

	\returns	\link HF_STATUS::OK \endlink on completion

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_CalculateAndStoreEnergyExpenditure

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE CalculateAndStoreEnergyExpenditure(HF::SpatialStructures::Graph* g);

/*!
	\brief	Add a new node attribute in the graph for the nodes at ids.

	\param		g			Graph to add attributes to 
	\param		ids			IDs of nodes to add attributes to
	\param		attribute	The name of the attribute to add the scores to.

	\param		scores		An ordered array of null terminated char arrays 
							that correspond to the score of the ID in ids at the same index. 

	\param		num_nodes	Length of both the ids and scores arrays

	\returns	\link HF_STATUS::OK \endlink on completion. 
				 Note that this does not guarantee that some 
				or all of the node attributes have been added

	\details
	For any id in ids, if said ID doesn't already exist in the graph, then it and its cost will
	silently be ignored without error.

	\pre	ids and scores arrays must be the same length
	
	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \link GetNodeAttributes \endlink (how to get string node attributes)
	\see \link GetNodeAttributesByID \endlink (how to get string node attributes by node ID)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AddNodeAttributes

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE AddNodeAttributes(
	HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* attribute,
	const char** scores, 
	int num_nodes
);

/*!
	\brief	Add a new float node attribute in the graph for the nodes at ids.

	\param		g			Graph to add attributes to
	\param		ids			IDs of nodes to add attributes to
	\param		attribute	The name of the attribute to add the scores to.

	\param		scores		An ordered array of floats
							that correspond to the score of the ID in ids at the same index.

	\param		num_nodes	Length of both the ids and scores arrays

	\returns	\link HF_STATUS::OK \endlink on completion.
				 Note that this does not guarantee that some
				or all of the node attributes have been added

	\details
	For any id in ids, if said ID doesn't already exist in the graph, then it and its cost will
	silently be ignored without error.

	\pre	ids and scores arrays must be the same length

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \link GetNodeAttributesFloat \endlink (how to get float node attributes)
	\see \link GetNodeAttributesByIDFloat \endlink (how to get float node attributes by node ID)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AddNodeAttributesFloat

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE AddNodeAttributesFloat(
	HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* attribute,
	const float* scores,
	int num_nodes
);
/*!
	\brief		Retrieve node attribute values from *g

	\param		g			The graph that will be used to retrieve 
							node attribute values from

	\param		attribute		The node attribute type to retrieve from *g
	\param		out_scores		Pointer to array of (char *), allocated by the caller
	\param		out_score_size	Keeps track of the size of out_scores buffer, 
								updated as required

	\pre		`attribute` is a string attribute. That is, at least one string value has been added to this attribute.

	\returns	\link HF_STATUS::OK \endlink on completion.

	\details	Memory shall be allocated in *out_scores to hold the char arrays.
				out_scores is a pointer to an array of (char *),
				which will be allocated by the caller.
				The caller must call DeleteScores to deallocate the memory addressed
				by each pointer in out_scores.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \link AddNodeAttributes \endlink (how to add string node attributes)
	\see \link GetNodeAttributesByID \endlink (how to get string node attributes by node ID)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeAttributes

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetNodeAttributes(
	const HF::SpatialStructures::Graph* g,
	const char* attribute,
	char** out_scores,
	int* out_score_size
);

/*!
	\brief		Retrieve node attribute values from *g

	\param		g				The graph that will be used to retrieve
								node attribute values from
	\param		ids				The list of node IDs to get attributes for.
								If NULL, returns attributes for all nodes.
	\param		attribute		The node attribute type to retrieve from *g
	\param		num_nodes		The length of the ids array
	\param		out_scores		Pointer to array of (char *), allocated by the caller
	\param		out_score_size	Keeps track of the size of out_scores buffer,
								updated as required

	\pre		All node IDs in `ids` must exist in graph `g`.
	\pre		If `ids` is not NULL, `num_nodes` must be equal to the length of `ids`.
	\pre		`attribute` is a string attribute. That is, at least one string value has been added to this attribute.
	\returns	\link HF_STATUS::OK \endlink on completion.

	\details	For the ID at `ids[i]`, `out_scores[i]` is the value of the attribute for the
				node associated with that ID.

				If `ids` is NULL, `out_scores` is an array holding the value of the attribute for
				all nodes, sorted in ascending order by ID.

				Memory shall be allocated in *out_scores to hold the char arrays.
				out_scores is a pointer to an array of (char *),
				which will be allocated by the caller.
				The caller must call DeleteScores to deallocate the memory addressed
				by each pointer in out_scores.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \link AddNodeAttributes \endlink (how to add string node attributes)
	\see \link GetNodeAttributes \endlink (how to get string node attributes)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeAttributesByID

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetNodeAttributesByID(
	const HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* attribute,
	int num_nodes,
	char** out_scores,
	int* out_score_size
);
/*!
	\brief		Retrieve float node attribute values from *g

	\param		g			The graph that will be used to retrieve
							node attribute values from

	\param		attribute		The node attribute type to retrieve from *g
	\param		out_scores		Pointer to array of float, allocated by the caller
	\param		out_score_size	Keeps track of the size of out_scores buffer,
								updated as required

	\pre		`attribute` is a float attribute. That is, only float values have been added to this attribute.

	\returns	\link HF_STATUS::OK \endlink on completion.

	\details	The caller must deallocate the memory addressed by out_scores.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \link AddNodeAttributesFloat \endlink (how to add float node attributes)
	\see \link GetNodeAttributesByIDFloat \endlink (how to get float node attributes by node ID)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeAttributesFloat

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetNodeAttributesFloat(
	const HF::SpatialStructures::Graph* g,
	const char* attribute,
	float* out_scores,
	int* out_score_size
);

/*!
	\brief		Retrieve float node attribute values from *g

	\param		g				The graph that will be used to retrieve
								node attribute values from
	\param		ids				The list of node IDs to get attributes for.
								If NULL, returns attributes for all nodes.
	\param		attribute		The node attribute type to retrieve from *g
	\param		num_nodes		The length of the ids array
	\param		out_scores		Pointer to array of floats, allocated by the caller
	\param		out_score_size	Keeps track of the size of out_scores buffer,
								updated as required

	\pre		All node IDs in `ids` must exist in graph `g`.
	\pre		If `ids` is not NULL, `num_nodes` must be equal to the length of `ids`.
	\pre		`attribute` is a float attribute. That is, only float values have been added to this attribute.

	\returns	\link HF_STATUS::OK \endlink on completion.

	\details	For the ID at `ids[i]`, `out_scores[i]` is the value of the attribute for the
				node associated with that ID.

				If `ids` is NULL, `out_scores` is an array holding the value of the attribute for
				all nodes, sorted in ascending order by ID.

				The caller must deallocate the memory addressed by out_scores.

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \link AddNodeAttributesFloat \endlink (how to add float node attributes)
	\see \link GetNodeAttributesFloat \endlink (how to get float node attributes)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeAttributesByIDFloat

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetNodeAttributesByIDFloat(
	const HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* attribute,
	int num_nodes,
	float* out_scores,
	int* out_score_size
);

/*!
	\brief Check whether or not an attribute is stored with float values in a graph.
	\param		g			The pointer of the graph to check
	\param		attribute	The attribute to check

	\returns	1 if the attribute exists in the graph and contains only float values.
				0 otherwise.
*/
C_INTERFACE IsFloatAttribute(
	const HF::SpatialStructures::Graph* g,
	const char* attribute
);

/*!
	\brief		Free the memory of every (char *) in scores_to_delete.

	\param		scores_to_delete	Pointer to array of (char *), allocate by the caller
	\param		num_char_arrays		Block count of scores_to_delete

	\returns	\link HF_STATUS::OK \endlink on completion
	
	\see		\link GetNodeAttributes \endlink on using DeleteScoreArray
*/
C_INTERFACE DeleteScoreArray(char** scores_to_delete, int num_char_arrays);

/*!
	\brief		Deletes the node attribute values of the type denoted by s, from graph *g

	\param		g	The graph from which attributes of type s will be deleted
	\param		s	The attribute value type to be cleared from within g

	\returns	\link HF_STATUS::OK \endlink on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_ClearAttributeType

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \link AddNodeAttributes \endlink (how to add node attributes)
	\see \link GetNodeAttributes \endlink (how to retrieve node attributes)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE ClearAttributeType(HF::SpatialStructures::Graph* g, const char* s);

/*!
	\brief		Get the number of nodes in a graph

	\param		g			Pointer to the graph to get the size of
	\param		out_size	Location where the size of the graph will be written

	\returns	\link HF_STATUS::OK \endlink on completion

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_get_csr_pointers (how to retrieve a CSR representation of a graph)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>
	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetSizeOfGraph

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetSizeOfGraph(const HF::SpatialStructures::Graph* g, int* out_size);


/*! 
	\brief Create a cost in the graph based on a set of node parameters
	
	\param graph_ptr Graph to perform this operation on
	\param attr_key Attribute to create a new cost set from.
	\param cost_string Name of the new cost set.
	\param dir Direction that the cost of the edge should be calculated in. For example
			   INCOMING will use the cost of the node being traveled to by the edge. 

	\returns `HF_STATUS::OK` If the cost was successfully added to the graph
	\returns `HF_STATUS::NOT_FOUND` If `attr_key` is not the key of an already existing
									node parameter. 
*/
C_INTERFACE GraphAttrsToCosts(
	HF::SpatialStructures::Graph * graph_ptr, 
	const char * attr_key,
	const char * cost_string, 
	HF::SpatialStructures::Direction dir);

/*!
	\brief		Get all edge costs of type cost_type in the graph

	\param	g				The graph to traverse
	\param	cost_type		Name of the cost type to get the cost from
	\param	out_scores		Output array for the costs in the graph
	\param	out_scores_size Output parameter for the size of out_scores buffer, updated as required

	\pre		g is a valid graph.

	\post
	`out_scores` is updated with the cost of traversing from parent to child. If no
	edge exists between parent and child, it won't be added.

	\returns \link HF_STATUS::OK \endlink on success
	\returns \link HF_STATUS::NO_COST \endlink if there was no cost with cost_name

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetEdgeCosts(
	const HF::SpatialStructures::Graph* g,
	const char* cost_type,
	float* out_scores,
	int* out_score_size);
/*!
	\brief Count the number of edges associated with cost_type in a given graph.

	\param	g				The graph to query
	\param	cost_type		Name of the cost type to count edges for
	\param	out_size		Output parameter which holds the number of edges counted.

	\returns
*/
C_INTERFACE CountNumberOfEdges(
	const HF::SpatialStructures::Graph* g,
	const char* cost_type,
	int* out_size);
/*!
	\brief		Get the costs of traversing from `parent` to `child` in a given array

	\param	g				The graph to traverse
	\param	ids				An array of ids to get the costs from
	\param	cost_type		Name of the cost type to get the cost from
	\param	num_ids			The number of ids given (non-unique)
	\param	out_scores		Output array for the costs in the graph
	\param	out_scores_size Output parameter for the size of out_scores buffer, updated as required

	\pre		g is a valid graph.
	\pre		ids is in the format [parent1, child1, parent2, child2,...] which maps to [edge1,edge2...]

	\post
	`out_scores` is updated with the cost of traversing from parent to child. If no
	edge exists between parent and child, it won't be added.

	\returns \link HF_STATUS::OK \endlink on success
	\returns \link HF_STATUS::NO_COST \endlink if there was no cost with cost_name

	\see \ref graph_setup (how to create a graph)
	\see \ref graph_add_edge_from_nodes (how to add edges to a graph using nodes)
	\see \ref graph_add_edge_from_node_ids (how to add edges to a graph using node IDs)
	\see \ref graph_compress (how to compress a graph after adding/removing edges)
	\see \ref graph_teardown (how to destroy a graph)

	Begin by reviewing the example at \ref graph_setup to create a graph.<br>

	You may add edges to the graph using nodes (\ref graph_add_edge_from_nodes)<br>
	or alternative, you may provide node IDs (\ref graph_add_edge_from_node_IDs).<br>

	Be sure to compress the graph (\ref graph_compress) every time you add/remove edges.<br>

	Finally, when you are finished with the graph,<br>
	it must be destroyed. (\ref graph_teardown)
*/
C_INTERFACE GetEdgeCostsFromNodeIDs(
	const HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* cost_type,
	int num_ids,
	float* out_scores,
	int* out_score_size);

/*!
	\brief		Get the alternate costs of traversing a given path

	\param	g				The graph to traverse
	\param	ids				A path of node ids to get costs from (in the form [n1,n2,...,nk]
	\param	cost_type		Name of the cost type to get the cost from
	\param	num_ids			The number of ids given (may be non-unique)
	\param	out_scores		Output array for the costs in the graph
	\param	out_scores_size Output parameter for the size of out_scores buffer, updated as required

	\returns \link HF_STATUS::OK \endlink on success
	\returns \link HF_STATUS::NO_COST \endlink if there was no cost with cost_name
*/
C_INTERFACE AlternateCostsAlongPathWithIDs(
	const HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* cost_type,
	int num_ids,
	float* out_scores,
	int* out_score_size);

/*!
	\brief		Get the alternate costs of traversing a given path

	\param	g				The graph to traverse
	\param	path			A path structure to get the costs from.
	\param	cost_type		Name of the cost type to get the cost from
	\param	out_scores		Output array for the costs in the graph
	\param	out_scores_size Output parameter for the size of out_scores buffer, updated as required

	\returns \link HF_STATUS::OK \endlink on success
	\returns \link HF_STATUS::NO_COST \endlink if there was no cost with cost_name
*/

C_INTERFACE AlternateCostsAlongPathStruct(
	const HF::SpatialStructures::Graph* g,
	const HF::SpatialStructures::Path* path,
	const char* cost_type,
	float* out_scores,
	int* out_score_size);
/**@}*/
