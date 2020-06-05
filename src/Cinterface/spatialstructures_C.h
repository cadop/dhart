#include <cinterface_utils.h>
#include <vector>
#include <array>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {
		class Graph;
		enum class COST_AGGREGATE : int;
		struct Node; // Careful with these forward declares!
		struct Edge; // Spent like 2 hours reading mangled names because I accidentally
	}				 // defined these as classes (V) instead of structs (U)
}

/**
* @defgroup SpatialStructures
* Contains the Graph and Node datatypes.
* @{
*/

/// <summary> Get a vector of every node in the given graph pointer. </summary>
/// <param name="graph"> Graph to retrieve nodes from. </param>
/// <param name="out_vector_ptr"> Output parameter for the new vector. </param>
/// <param name="out_data_ptr"> Output parameter for the vector's data. </param>
/// <returns>
/// HF_STATUS::INVALID_PTR if the given pointer was invalid. HF::GENERIC_ERROR if the graph is not
/// valid. HF::OK if successful.
/// </returns>
C_INTERFACE GetAllNodesFromGraph(
	const HF::SpatialStructures::Graph* graph,
	std::vector<HF::SpatialStructures::Node>** out_vector_ptr,
	HF::SpatialStructures::Node** out_data_ptr
);

// DOn't document this because it's not ready yet.
C_INTERFACE GetEdgesForNode(
	const HF::SpatialStructures::Graph* graph,
	const HF::SpatialStructures::Node* Node,
	std::vector<HF::SpatialStructures::Edge>** out_vector_ptr,
	HF::SpatialStructures::Edge** out_edge_list_ptr,
	int* out_edge_list_size
);

/// <summary> Get the size of a node vector. </summary>
/// <param name="node_list"> Node vector to get the size from. </param>
/// <param name="out_size"> Size of the vector will be written to this int. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE GetSizeOfNodeVector(
	const std::vector<HF::SpatialStructures::Node>* node_list,
	int* out_size
);

// This is never used. Don't include.
C_INTERFACE GetSizeOfEdgeVector(
	const std::vector<HF::SpatialStructures::Edge>* edge_list,
	int* out_size
);

/// <summary> Get an ordered array of costs for each node aggregatted by the desired method. </summary>
/// <param name="graph"> Graph to aggregare edges from. </param>
/// <param name="agg"> Aggregation type to use. </param>
/// <param name="directed">
/// If true, only count outgoing edges for a node, otherwise count both outgoing and incoming edges.
/// </param>
/// <param name="out_vector_ptr"> Output parameter for the vector. </param>
/// <param name="out_vector_ptr"> Output parameter for the vector's held data. </param>
/// <returns> HF_STATUS::OK if successful. If the graph wasn't valid HF_STATUS::NO_GRAPH. </returns>
C_INTERFACE AggregateCosts(
	const HF::SpatialStructures::Graph* graph,
	int agg,
	bool directed,
	std::vector<float>** out_vector_ptr,
	float** out_data_ptr
);

/// <summary> Create a new empty graph. </summary>
/// <param name="nodes"> Unused. </param>
/// <param name="num_nodes"> Unused. </param>
/// <param name="out_graph"> Output parameter to store the graph in. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE CreateGraph(
	const float* nodes,
	int num_nodes,
	HF::SpatialStructures::Graph** out_graph
);

/// <summary>
/// Add an edge between parent and child. If parent or child doesn't already exist in the graph,
/// they will be added and automatically assigned new IDS.
/// </summary>
/// <param name="graph"> Graph to add the new edge to. </param>
/// <param name="parent">
/// A 3 element float array containing the x, y, and z coordinate of the parent node.
/// </param>
/// <param name="child">
/// A 3 element float array containing the x, y, and z coordinate of the child node,
/// </param>
/// <param name="score"> The cost from parent to child </param>
/// <returns>
/// HF_STATUS::OK on success. HF_STATUS::INVALID_PTR on an invalidparent or child node.
/// </returns>
C_INTERFACE AddEdgeFromNodes(
	HF::SpatialStructures::Graph* graph,
	const float* parent,
	const float* child,
	float score
);

/// <summary>
/// Create a new edge between parent_id and child_id. If these IDs don't yet exist in the graph they
/// will be added.
/// </summary>
/// <param name="graph"> Graph to create the new edge in. </param>
/// <param name="parent"> The parent's id in the graph. </param>
/// <param name="child"> The child's id in the graph. </param>
/// <param name="score"> The cost from parent to child. </param>
/// <param name="returns"> HF_STATUS::OK on completion. </param>
C_INTERFACE AddEdgeFromNodeIDs(
	HF::SpatialStructures::Graph* graph,
	int parent_id,
	int child_id,
	float score
);

/// <summary>
/// Retrieve all information for a graph's CSR representation. This will compress the graph if it
/// was not yet already compressed.
/// </summary>
/// <param name="out_nnz"> Number of non-zeros contained within the graph. </param>
/// <param name="out_num_rows"> Number of rows contained within the graph. </param>
/// <param name="out_num_rows"> Number of columns contained within the graph. </param>
/// <param name="out_data_ptr"> Pointer to the graph's data array. </param>
/// <param name="out_inner_indices_ptr"> Pointer to the graph's inner indices array. </param>
/// <param name="out_inner_indices_ptr"> Pointer to the graph's outer indices array. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE GetCSRPointers(
	HF::SpatialStructures::Graph* graph,
	int* out_nnz,
	int* out_num_rows,
	int* out_num_cols,
	float** out_data_ptr,
	int** out_inner_indices_ptr,
	int** out_outer_indices_ptr
);

/// <summary>
/// Get the id of the given node in the graph. If the node does not exist, <paramref name="out_id"
/// /> will be set to -1.
/// </summary>
/// <param name="graph"> The graph to get the ID from. </param>
/// <param name="point">
/// A 3 float array containing the x,y, and z coordinates of the point to get the ID for.
/// </param>
/// <param name="out_id">
/// Output parameter for the id. Set to -1 if that point couldn't be found in <paramref name="graph" />
/// </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE GetNodeID(
	HF::SpatialStructures::Graph* graph,
	const float* point,
	int* out_id
);

/// <summary> Compress the given graph into a csr representation. </summary>
/// <returns> HF_STATUS::OK on completion. </returns>
/// <remarks>
/// This will reduce the memory footprint of the graph, and invalidate all existing CSR
/// representations of it. If the graph is already compressed, this will be a no-op.
/// </remarks>
C_INTERFACE Compress(
	HF::SpatialStructures::Graph* graph
);

/// <summary> Clear the nodes/edges for the given graph. </summary>
/// <param name="graph"> Graph to clear nodes from. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE ClearGraph(
	HF::SpatialStructures::Graph* graph
);

/// <summary> Delete the vector of nodes at the given pointer. </summary>
/// <param name="nodelist_to_destroy"> Vector of nodes to destroy. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE DestroyNodes(
	std::vector<HF::SpatialStructures::Node>* nodelist_to_destroy
);

/// <summary> Delete the vector of edges at the given pointer. </summary>
/// <param name="edgelist_to_destroy"> Vector of nodes to destroy. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE DestroyEdges(
	std::vector<HF::SpatialStructures::Edge>* edgelist_to_destroy
);

/// <summary> Delete a graph. </summary>
/// <param name="graph_to_destroy"> Graph to delete. </param>
/// <returns> HF_STATUS::OK on completion. </returns>
C_INTERFACE DestroyGraph(
	HF::SpatialStructures::Graph* graph_to_destroy
);

/**@}*/