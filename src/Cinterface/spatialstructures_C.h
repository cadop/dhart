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
				Used for exporting C Interface functions from the Human Factors DLL.
*/
#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {
		class Graph;
		struct Subgraph;

		enum class COST_AGGREGATE : int;

		struct Node;	// Careful with these forward declares!
		struct Edge;	// Spent like 2 hours reading mangled names 
						// because I accidentally defined these 
						// as classes (V) instead of structs (U)
	}
}

/*!
	\enum		COST_ALG_KEY
	\brief		Indices of keys for costs returned from calling CalculateAndStore functions
*/
const enum COST_ALG_KEY {
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

	Although graphs can be generated (see \ref generate_graph),<br>
	they can <b>also be created by objects instantiated or obtained by the user</b>.

	\section	graph_setup				Graph setup
	TODO Graph setup

	\section	graph_add_edge_from_nodes		Adding edges from nodes
	TODO Adding edges from nodes

	\section	graph_add_edge_from_node_ids	Adding edges from node IDs
	TODO Adding edges from node IDs

	\section	graph_compress	Compressing the graph
	TODO Compressing the graph

	\section	graph_get_csr_pointers	Retrieve a CSR of the graph
	TODO Retrieve a CSR of the graph

	When you are finished with the graph, be sure to destroy it.<br>
	(see \ref graph_teardown)
*/

/*!
	\brief		Get a vector of every node in the given graph pointer
	
	\param		graph			Graph to retrieve nodes from
	\param		out_vector_ptr	Output parameter for the new vector
	\param		out_data_ptr	Output parameter for the vector's data
	
	\returns	HF_STATUS::INVALID_PTR if the given pointer was invalid.
				HF_STATUS::GENERIC_ERROR if the graph is not valid.
				HF_STATUS::OK if successful.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetAllNodesFromGraph

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetEdgesForNode

	<br>
	\verbatim
	TODO output
	\endverbatim
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
	
	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetSizeOfNodeVector

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE GetSizeOfNodeVector(
	const std::vector<HF::SpatialStructures::Node>* node_list,
	int* out_size
);

/*!
	\brief		Get the size of an edge vector

	\param		edge_list	Edge vector to get the size from
	\param		out_size	Size of the vector will be written to *out_size

	\returns	HF_STATUS::OK on completion

	\deprecated	This is never used. Do not include.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetSizeOfEdgeVector

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns HF_STATUS::OK on success
	\returns HF_STATUS::NO_COST if there was no cost with cost_name

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetEdgeCost

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns	HF_STATUS::OK if successful.
	\returns	HF::Exceptions::STATUS::NOT_COMPRESSED if the graph wasn't compressed.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AggregateCosts

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_CreateGraph

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\param		parent		A three-element array of float 
							containing the x, y, and z coordinates of the parent node

	\param		child		A three-element array of float
							containing the x, y, and z coordinates of the child node.

	\param		score		The edge cost from parent to child

	\param		cost_type	Edge cost type

	\returns HF_STATUS::OK on success.
	\returns HF_STATUS::INVALID_PTR on an invalid parent or child node

	\returns HF_STATUS::NOT_COMPRESSED Tried to add an edge to an alternate cost type
									   when the graph wasn't compressed

	\returns HF::Exceptions::OUT_OF_RANGE Tried to add an edge to an alternate cost 
										  that didn't already exist in the default graph.

	\pre cost_type MUST be a valid delimited char array. 
				   If the entire program crashes when this is called, this is why. 

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AddEdgeFromNodes

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns STATUS::OK on completion. 

	\returns STATUS::NOT_COMPRESSED if an alternate cost was added 
								    without first compressing the graph

	\returns STATUS::NO_COST The given cost string was invalid. 

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AddEdgeFromNodeIDs

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns	HF_STATUS::OK on success.
	\returns	HF_STATUS::NO_COST if the asked for cost doesn't exist.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetCSRPointers

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeID

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE GetNodeID(
	HF::SpatialStructures::Graph* graph,
	const float* point,
	int* out_id
);

/*!
	\brief		Compress the given graph into a CSR representation.

	\param		graph	The graph to compress into a CSR.

	\returns	HF_STATUS::OK on completion.

	\remarks	This will reduce the memory footprint of the graph,
				and invalidate all existing CSR representation of it.
				If the graph is already compressed, this function will be a no-op.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_Compress

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns HF_STATUS::OK if the operation succeeded
	\returns HF_STATUS::NO_COST if a cost was specified and it couldn't be found.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_ClearGraph

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE ClearGraph(
	HF::SpatialStructures::Graph* graph,
	const char* cost_type
);

/*!
	\brief		Delete the vector of nodes at the given pointer

	\param		nodelist_to_destroy		Vector of nodes to destroy

	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_DestroyNodes

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE DestroyNodes(
	std::vector<HF::SpatialStructures::Node>* nodelist_to_destroy
);

/*!
	\brief		Delete the vector of edges at the given pointer.

	\param		edgelist_to_destroy		Vector of nodes to destroy

	\returns	HF_STATUS::OK on completion.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_DestroyEdges

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE DestroyEdges(
	std::vector<HF::SpatialStructures::Edge>* edgelist_to_destroy
);

/*!
	\brief		Delete a graph.

	\param		graph_to_destroy		Graph to delete

	\returns	HF_STATUS::OK on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_DestroyGraph

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE DestroyGraph(
	HF::SpatialStructures::Graph* graph_to_destroy
);

/*!
	\brief		Calculates cross slope for all subgraphs in *g
	\param		g	The graph to calculate cross slope on

	\returns	HF_STATUS::OK on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_CalculateAndStoreCrossSlope

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE CalculateAndStoreCrossSlope(HF::SpatialStructures::Graph* g);

/*!
	\brief		Calculates energy expenditure for all subgraphs in *g 
				and stores them in the graph at AlgorithmCostTitle(ALG_COST_KEY::EnergyExpenditure)

	\param		g		The address of a Graph

	\returns	HF_STATUS::OK on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_CalculateAndStoreEnergyExpenditure

	<br>
	\verbatim
	TODO output
	\endverbatim
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

	\returns	HF_STATUS::OK on completion. 
				 Note that this does not guarantee that some 
				or all of the node attributes have been added

	\detail 
	For any id in ids, if said ID doesn't already exist in the graph, then it and its cost will
	silently be ignored without error.

	\pre	ids and scores arrays must be the same length
	
	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_AddNodeAttributes

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE AddNodeAttributes(
	HF::SpatialStructures::Graph* g,
	const int* ids,
	const char* attribute,
	const char** scores, 
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

	\returns	HF_STATUS::OK on completion.

	\details	Memory shall be allocated in *out_scores to hold the char arrays.
				out_scores is a pointer to an array of (char *),
				which will be allocated by the caller.
				The caller must call DeleteScores to deallocate the memory addressed
				by each pointer in out_scores.

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetNodeAttributes

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE GetNodeAttributes(
	const HF::SpatialStructures::Graph* g,
	const char* attribute,
	char** out_scores,
	int* out_score_size
);

/*!
	\brief		Free the memory of every (char *) in scores_to_delete.

	\param		scores_to_delete	Pointer to array of (char *), allocate by the caller
	\param		num_char_arrays		Block count of scores_to_delete

	\returns	HF_STATUS::OK on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_DeleteScoreArray

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE DeleteScoreArray(char** scores_to_delete, int num_char_arrays);

/*!
	\brief		Deletes the node attribute values of the type denoted by s, from graph *g

	\param		g	The graph from which attributes of type s will be deleted
	\param		s	The attribute value type to be cleared from within g

	\returns	HF_STATUS::OK on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_ClearAttributeType

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE ClearAttributeType(HF::SpatialStructures::Graph* g, const char* s);

/*!
	\brief		Get the number of nodes in a graph

	\param		g			Pointer to the graph to get the size of
	\param		out_size	Location where the size of the graph will be written

	\returns	HF_STATUS::OK on completion

	\snippet tests\src\spatialstructures_C_cinterface.cpp snippet_spatialstructuresC_GetSizeOfGraph

	<br>
	\verbatim
	TODO output
	\endverbatim
*/
C_INTERFACE GetSizeOfGraph(const HF::SpatialStructures::Graph* g, int* out_size);

/**@}*/
