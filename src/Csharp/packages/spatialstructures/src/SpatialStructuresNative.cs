using DHARTAPI.Exceptions;
using DHARTAPI.NativeUtils;
using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters;
using System.Diagnostics;
using System.Collections;

namespace DHARTAPI.SpatialStructures
{
	/*! 
        \brief Contains info necessary to reconstruct or access a CSR.
    */
	public struct CSRInfo
	{
		/// <summary> The number of nonzeros contained by the graph. </summary>
		public int nnz;

		/// <summary> The number of columns in the graph. </summary>
		public int cols;

		/// <summary> The number of rows in the graph </summary>
		public int rows;

		/// <summary> A pointer to the graph's data array </summary>
		public IntPtr data;

		/// <summary> A pointer to the graph's outer_indices array. </summary>
		public IntPtr outer_indices;

		/// <summary> A pointer to the graph's inner indices array. </summary>
		public IntPtr inner_indices;

		/// <summary> Initializes a new instance of the <see cref="CSRInfo" /> struct. </summary>
		/// <param name="nnz"> The number of non-zeros </param>
		/// <param name="cols"> The number of columns. </param>
		/// <param name="rows"> The number of rows. </param>
		/// <param name="data"> Pointer to the CSR's data array. </param>
		/// <param name="outer_indices"> Pointer to the CSR's outer_indices array. </param>
		/// <param name="inner_indices"> Pointer to the CSR's inner_indices array. </param>
		internal CSRInfo(int nnz, int cols, int rows, IntPtr data, IntPtr outer_indices, IntPtr inner_indices)
		{
			this.nnz = nnz;
			this.rows = rows;
			this.cols = cols;
			this.data = data;
			this.outer_indices = outer_indices;
			this.inner_indices = inner_indices;
		}

		/*!
            \brief Get a string representation of this object's nnz, rows, and cols. 
        
            \returns A string representation of this object's nnz, rows, and cols.
        */
		public override string ToString()
		{
			string out_string = "(";
			out_string += "nnz: " + nnz.ToString() + ", ";
			out_string += "rows: " + rows.ToString() + ", ";
			out_string += "cols: " + cols.ToString();
			out_string += ")";

			return out_string;
		}
	}

	/*!  \brief Native Functions for the SpatialStructures namespace */
	internal static class NativeMethods
	{

		/*! 
            \brief  Create  an empty graph in native memory 
            \returns A pointer to the new empty graph
        */
		internal static IntPtr C_CreateGraph()
		{
			// Define a pointer to serve as output for the c++ function
			IntPtr out_ptr = new IntPtr();

			// create a graph in C++, updating out_ptr
			var res = CreateGraph(new float[3], 0, ref out_ptr);

			// If this goes wrong then something changed in the C-Interface that wasn't also
			// changed in C#.
			Debug.Assert(res == HF_STATUS.OK);

			// Return the updated pointer
			return out_ptr;
		}

		/*!
            \brief Add an edge to an existing graph in native memory.
            
            \param graph_ptr A pointer to the graph to add this edge to.
            \param parent_id The id of the parent node for this edge.
            \param child_id the id of the child node for this edge.
            \param score cost of traversing from parent to child.
            \param cost_type The type of cost to add the edge to. 

            \attention
            This overload is meant for debugging. There are many issues that can occur with
            adding integers to the graph that don't already have nodes assigned. Instead
            use the overload of this function deals with vector 3.

            \pre 1) If adding edges to a new cost type, the graph has been compressed
            by calling CompressToCSR().
            \pre 2) If adding a cost to a type that isn't the default cost type, the edge must
            already exist in the default cost type.
            \pre 3) If the graph has already been compressed and alternate costs exist, then
            both `parent` and `child` already exist in the graph.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then New IDs will be assigned to them. 
            \post 2) Existing representations of the this graph's CSR will be invalidated. 
            \post 3) If `cost_type` is not blank, and does not refer to the default cost type
            or any other cost that already exists in the graph, a new cost type will be created. 
            
            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.

            \warning 
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.
         */
		internal static void C_AddEdge(IntPtr graph_ptr, int parent_id, int child_id, float score, string cost_type)
			=> AddEdgeFromNodeIDs(graph_ptr, parent_id, child_id, score, cost_type);

		/*!
            \brief Create a new edge between parent and child with cost.
            
            \param graph_ptr Pointer to the the graph to add edges to.
            \param parent The X,Y,Z location for the parent node.
            \param child x,y,z location for the child 
            \param cost cost for parent to child 
            \param cost_type The type of cost to add the edge to. 

            \attention
            This overload is meant for debugging. There are many issues that can occur with
            adding integers to the graph that don't already have nodes assigned. Instead
            use the overload of this function deals with vector 3.

            \pre 1) If adding edges to a new cost type, the graph has been compressed
            by calling CompressToCSR().
            \pre 2) If adding a cost to a type that isn't the default cost type, the edge must
            already exist in the default cost type.
            \pre 3) If the graph has already been compressed and alternate costs exist, then
            both `parent` and `child` already exist in the graph.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then New IDs will be assigned to them. 
            \post 2) Existing representations of the this graph's CSR will be invalidated. 
            \post 3) If `cost_type` is not blank, and does not refer to the default cost type
            or any other cost that already exists in the graph, a new cost type will be created. 
            
            \throws LogicError tried to add an alternate cost to the graph before it was compressed
            \throws InvalidCostOperation Tried to add an alternate cost that doesn't exist in the graph's
            default cost type.

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning 
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.
        */
		internal static void C_AddEdge(IntPtr graph_ptr, Vector3D parent, Vector3D child, float cost, string cost_type)
		{
			// Convert parent and child to arrays
			var parent_arr = parent.ToArray();
			var child_arr = child.ToArray();

			// Try to add them to the graph.
			HF_STATUS stat = AddEdgeFromNodes(graph_ptr, parent_arr, child_arr, cost, cost_type);

			// Handle error codes
			switch (stat)
			{
				// If it's ok then no error checking required
				case HF_STATUS.OK:
					return;

				// if NOT_COMPRESSED then inform the user
				case HF_STATUS.NOT_COMPRESSED:
					throw new LogicError("Tried to add an edge to an alternate cost before the graph was compressed!");

				// if OUT_OF_RANGE then inform the user
				case HF_STATUS.OUT_OF_RANGE:
					throw new InvalidCostOperation("Tried to add an alternate cost to an edge that doesn't already" +
						"exist in the default cost set!");
			}
		}

		/*! \brief Free the memory of a graph in C++ */
		internal static void C_DestroyGraph(IntPtr graph_ptr) => DestroyGraph(graph_ptr);

		/*! \brief get the ID of a node in a graph
            \param graph_ptr Graph to use to get the id of the node
            \param x X coordinate of the node.
            \param y Y coordinate of the node.
            \param z Z coordinate of the node.

            \returns The ID of the node in the graph pointed to by `graph_ptr`
            or -1 if the node doesn't exist in the graph.
        */
		internal static int C_GetNodeID(IntPtr graph_ptr, float x, float y, float z)
		{
			// Setup return parameter and input array
			int return_int = -1;
			float[] node_arr = { x, y, z };

			// Call GetNodeID in C++, updating return_int
			var res = GetNodeID(graph_ptr, node_arr, ref return_int);

			// Return the result
			return return_int;
		}

		/*!
            \brief Compress a graph and get the data required to reconstruct its CSR.

            \param graph_ptr Pointer to the graph to get the CSR of in native memory.
            \param cost_type Type of cost to getuse at the CSR's values array

            \returns CSRInfo Containing all information required to reconstruct the CSR.

            \throws KeyNotFoundException `cost_type` is not blank, the name of the graph's
            default cost type, or the name of any already defined cost type in
            the graph.

            \see CSRPtrs for more info on the CSR type and how to access it.
        */
		internal static CSRInfo C_GetCSRPointers(IntPtr graph_ptr, string cost_type)
		{

			// Define pointers to use as output parameters for the C++ function
			IntPtr data = new IntPtr();
			IntPtr outer_indices = new IntPtr();
			IntPtr inner_indices = new IntPtr();
			int nnz = 0; int cols = 0; int rows = 0;

			// Call the C++ function. This will update all the ref
			// parameters.
			HF_STATUS res = GetCSRPointers(
				graph_ptr,
				ref nnz,
				ref rows,
				ref cols,
				ref data,
				ref outer_indices,
				ref inner_indices,
				cost_type
			);

			// If the cost doesn't exist, inform the caller
			if (res == HF_STATUS.NO_COST)
				throw new KeyNotFoundException("Cost " + cost_type + " could not be found in the graph");
			else if (res != HF_STATUS.OK)
				Debug.Assert(false); // Programmer error! Handle whatever error code was returned here!


			// Construct a new CSRInfo form this information
			return new CSRInfo(nnz, cols, rows, data, outer_indices, inner_indices);
		}

		/*! 
            \brief Get the Nodes of a graph in C++
            
            \param graph_ptr A pointer to the graph to get nodes from.
            
            \returns 
            A CVector and data containing all the informtion required to read from the node vector of the graph.
        */
		internal static CVectorAndData C_GetNodes(IntPtr graph_ptr)
		{
			// Define output pointers
			IntPtr vector_ptr = new IntPtr();
			IntPtr data_ptr = new IntPtr();

			// Get all nodes from the graph, updating vector_ptr and data_ptr
			var res = GetAllNodesFromGraph(graph_ptr, ref vector_ptr, ref data_ptr);

			// If this fails, then something changed in the C interface that wasn't updated here.  
			Debug.Assert(res == HF_STATUS.OK);

			// Get the size of the nodes vector from C++
			int size = 0;
			res = GetSizeOfNodeVector(vector_ptr, ref size);

			// If this fails, then something changed in the C interface that wasn't updated here.  
			Debug.Assert(res == HF_STATUS.OK);

			// return all the information required to read from this vector 
			return new CVectorAndData(data_ptr, vector_ptr, size);
		}

		/*!
            \brief Summarize the edgecosts of every node in the graph.
            
            \param graph_ptr A pointer to the graph to summarize the edge costs of.
            \param directed
            Whether or not the graph is directed. If set to true then each nodes's 
            score will only consider incomning edges. Otherwise, each node's score will consider
            both outgoing and incoming edges.
            
            \param agg_type The type of aggregation method to use.
            \param cost_type The type of cost to use for aggregating the edges.
            
            \returns
            A CVectorAndData for a vector of floats in C++ containing the scores for each node in order.

            \pre If not left blank, cost_type must point to a valid cost in the graph. 

            \throws KeyNotFoundException Cost specified didn't match the default cost, or any other
            cost type defined in the graph.

            \warning 
            This will compress the graph if it is not compressed already. If any edges
            were added between lat call to CompressToCSR and now, then any active CSRPtrs
            will be invalidated.

            \remarks
            The order of the scores returned bythis function match the order of the scores returned from
            Graph.getNodes. This can be especially useful for summarizing the results of a VisibilityGraph.
            
        */
		internal static CVectorAndData C_AggregateEdgeCosts(
			IntPtr graph_ptr,
			bool directed,
			GraphEdgeAggregation agg_type,
			string cost_type
		 )
		{
			// Create a new CVector and data to return
			CVectorAndData out_ptrs = new CVectorAndData();

			// Aggregate costs in C++, updating  out_ptrs and out_ptrs.data 
			var res = AggregateCosts(graph_ptr, agg_type, directed, cost_type, ref out_ptrs.vector, ref out_ptrs.data);

			Debug.Assert(res != HF_STATUS.GENERIC_ERROR);  // An exception that wasn't given an error code was caught by C++
			Debug.Assert(res != HF_STATUS.NOT_COMPRESSED); // The graph wasn't compressed before calling this function.
														   // Ensure callers aren't allowed to make this happen.
														   // Handle return types
			switch (res)
			{
				// If ok then no further action is needed
				case HF_STATUS.OK:
					break;
				// Inform the user if the graph isn't compressed
				case HF_STATUS.NOT_COMPRESSED:
					throw new LogicError("The graph wasn't compressed.");
				// If the specified cost type doesn't exist, then inform the user
				case HF_STATUS.NO_COST:
					throw new KeyNotFoundException("The cost type '" + cost_type + "' did not exist already in the graph");
				default:
					Debug.Assert(false); // There's an error code being returned that
										 // isn't accounted for. All outputs from C++ should be handled
										 // by explicit asserts or new exceptions.
					break;
			}

			// Return out_ptrs
			return out_ptrs;
		}

		/*! 
            \brief Get the cost of traversing from one node to another for a graph in C++.
            
            \param graph_ptr A pointer to the graph to get the edgecost from.
            \param parent The id of parent of the edge.
            \param child the id of the child of the edge
            \param cost_type The type of cost to get the cost from. 
            
            \returns The cost of traversing from `parent` to `child`.

			\pre 1) cost_type must be the name of a cost that already exists in the graph,
			or blank. 
            \pre 2) The graph must first have been compressed using Graph.CompressToCSR().
            
            \throws LogicError The graph wasn't compressed before calling this function.
			\throws KeyNotFoundExeption The cost_type specified was not the default cost, blank, or 
			the name of any cost that currently belongs to the graph.

            \note This is not a high performance function, since each index into the CSR requires
            an search in the CSR. If multiple values are required, it is suggested to iterate through
            the pointers from Graph.CompressToCSR().

        */
		internal static float C_GetEdgeCost(IntPtr graph_ptr, int parent, int child, string cost_type)
		{
			// Create an output parameter
			float out_float = -1;

			// Call the C++ function which will update out_float.
			HF_STATUS res = GetEdgeCost(graph_ptr, parent, child, cost_type, ref out_float);

			// Handle returned error code
			switch (res)
			{
				// On OK no action is required
				case HF_STATUS.OK:
					break;
				// On NOT_COMPRESSED throw because the caller misued the function
				case HF_STATUS.NOT_COMPRESSED:
					throw new LogicError("The graph must be compressed to read edge costs");
				// If the cost didn't exist throw
				case HF_STATUS.NO_COST:
					throw new KeyNotFoundException("The cost " + cost_type + " was not found in the graph.");
				default:
					Debug.Assert(false, "Humanfactors is returning an error code that is not being handled");
					break;
			}

			// Return the result
			return out_float;
		}

		/*!
            \brief Add attribute to nodes in a graph in C++

            \param graph_ptr   A pointer to the graph in C++
            \param ids         IDs of nodes to assign scores to for `attribute`
            \param attribute   Name of the attribute to assigns cores to for each node in `ids`
            \param scores      Ordered ids of scores to add to the node at the id in `ids` at the same index

            \pre the length of `ids` and `scores` must match
        */
		internal static void C_AddNodeAttributes(IntPtr graph_ptr, string attribute, int[] ids, string[] scores)
		{
			// Call native function
			var res = AddNodeAttributes(graph_ptr, ids, attribute, scores, scores.Length);

			Debug.Assert(res == HF_STATUS.OK, "Native code returned an error code when adding attributes to the graph");
		}

		/*!
            \brief Get node attirbutes from a graph in C++.

            \param graph_ptr A pointer to a graph in C++ to get attributes from.
            \param attribute The unique name of the attribute type to get from the graph for every node.
            \param num_nodes Number of nodes in the graph.

            \returns 
            If an attribute with the name of `attribute`, type was found in the graph, then an array of scores
            for each node is returned in order of ID. For example, the score of the node with id 10 would be stored
            at index 10, id 12 stored at index 12, etc. Nodes without scores for `attribute` will have empty
            strings at their indexes. 
            
        */
		internal unsafe static string[] C_GetNodeAttributes(IntPtr graph_ptr, string attribute, int num_nodes)
		{
			// Set up arrays to fulfil precondition
			IntPtr[] str_arr = new IntPtr[num_nodes];
			int size = -1;

			// Get the attributes for the nodes in this graph
			var res = GetNodeAttributes(graph_ptr, attribute, str_arr, ref size);
			Debug.Assert(res == HF_STATUS.OK, "Something went wrong when trying to read attributes for " + attribute);

			// Return an empty array if the size is 0, indicating the attribute doesn't exist
			if (size == 0)
				return new string[0];

			// Convert each pointer into a string using marshal
			string[] out_strings = new string[size];
			for (int i = 0; i < size; i++)
				out_strings[i] = Marshal.PtrToStringAnsi(str_arr[i]);

			// Finally, deallocate the native memory that was allocated to return the string array
			DeleteScoreArray(str_arr, size);

			// return output array
			return out_strings;
		}

		/*!
            \brief Get the number of nodes in the graph in C++/

            \param graph_ptr A pointer to the graph to get the number of nodes of.

            \returns the number of nodes in graph_ptr
        */
		internal static int C_GetGraphSize(IntPtr graph_ptr)
		{
			// define output parameter
			int out_size = -1;

			// Call the function in C++ updating out_size
			_ = GetSizeOfGraph(graph_ptr, ref out_size);

			return out_size;
		}

		/*!
            \brief Delete the a vector of nodes in native memory. 
            \param node_ptr A pointer to a vector of nodes in C++.
        */
		internal static void C_DestroyNodeVector(IntPtr node_ptr) => DestroyNodes(node_ptr);

		/*!
            \brief Delete the a vector of nodes in native memory. 
            \param float_vector A pointer to the vector of floats to destroy in C++.
        */
		internal static void C_DestroyFloatVector(IntPtr float_vector) => DestroyFloatVector(float_vector);

		/*! 
            \brief Compress a graph in C++.

            \param graph A pointer to a graph in C++.
            
            \post 
            The graph pointed to by `graph` will be compressed, or nothing will happen
            if the graph is already compressed
        */
		internal static void C_CompressGraph(IntPtr graph) => Compress(graph);

		/*! 
            \brief Calculate the Cross Slope for a graph in C++.
            
            \param graph A pointer to the graph to calculate Cross Slope for

            \post The graph will have have cross slope calculated and stored as an alternate cost type.
        */
		internal static void C_CalculateAndStoreCrossSlope(IntPtr graph) => CalculateAndStoreCrossSlope(graph);

		/*! 
            \brief Calculate Energy Expenditure for graph in C++.
            
            \param graph A pointer to the graph to calculate Energy Expenditure  for

            \post The graph will have have Energy Expenditure calculated and stored as an alternate cost type.
        */
		internal static void C_CalculateAndStoreEnergyExpenditure(IntPtr graph) => CalculateAndStoreEnergyExpenditure(graph);

		/*!
            \brief Clear an attribute type from the graph

            \param graph Graph to clear the attribute type from
            \param attribute_name the name of the atribute to clear from the graph.

            \post The attribute at `attribute_name` and all of its scores are cleared ffrom `graph`.
        */
		internal static void C_ClearAttributeType(IntPtr graph, string attribute_name)
			=> ClearAttributeType(graph, attribute_name);

		/*! 
			\brief Generate a cost set based on a set of node parameters

			\param graph_ptr Graph to perform this operation on.
			\param attribute_key Attribute to create a new cost set from.
			\param cost_key Key for the newly generated cost set. 
			\param dir Direction to use for calculating the cost of any edge. For example
					   INCOMING will use the cost of the node being traveled to by the edge. 

			\throws KeyNotFoundException The parameter assinged by parameter_name is not
										 the key of any node parameter in the graph. 
		*/
		internal static void C_AttrsToCosts(IntPtr graph_ptr, string attribute_key,  string cost_key, Direction dir)
		{
			HF_STATUS status = GraphAttrsToCosts(graph_ptr, attribute_key, cost_key, dir);

			if (status == HF_STATUS.NOT_FOUND)
				throw new KeyNotFoundException(attribute_key + " is not the key of any node attribute in the graph!");
			else
				Debug.Assert(status == HF_STATUS.OK);
		}

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetNodes(
			IntPtr graph,
			ref IntPtr out_vector_ptr,
			ref IntPtr out_data_ptr
		);


		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GraphAttrsToCosts(
			IntPtr graph,
			string attr_key,
			string cost_string,
			Direction dir
		);


		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetSizeOfNodeVector(
			IntPtr node_vector,
			ref int out_size
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetCSRPointers(
			IntPtr Graph,
			ref int out_nnz,
			ref int out_num_rows,
			ref int out_num_cols,
			ref IntPtr out_data_ptr,
			ref IntPtr out_inner_indices_ptr,
			ref IntPtr out_outer_indices_ptr,
			string cost_type
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS Compress(IntPtr graph);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS AggregateCosts(
			IntPtr graph,
			GraphEdgeAggregation agg,
			bool directed,
			string cost_type,
			ref IntPtr out_vector_ptr,
			ref IntPtr out_data_ptr
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetNodeID(IntPtr graph, [In] float[] point, ref int out_id);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS AddEdgeFromNodes(
			IntPtr Graph,
			[In] float[] parent,
			[In] float[] child,
			float score,
			string cost_type
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS AddEdgeFromNodeIDs(
			IntPtr Graph,
			int parent,
			int child,
			float score,
			string cost_type
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS AggregateCosts(
			IntPtr graph,
			int agg,
			bool directed,
			ref IntPtr out_vector_ptr,
			ref IntPtr out_float_ptr
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetAllNodesFromGraph(
			IntPtr graph_ptr,
			ref IntPtr out_vector_ptr,
			ref IntPtr out_data_ptr
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS CreateGraph([In] float[] nodes, int num_nodes, ref IntPtr out_graph);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS DestroyGraph(
			IntPtr graph_to_destroy
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS DestroyNodes(IntPtr list_to_destroy);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS DestroyFloatVector(IntPtr float_vector);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetEdgeCost(
			IntPtr graph_ptr,
			int parent_id,
			int child_id,
			string cost_type,
			ref float out_float
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS CalculateAndStoreCrossSlope(IntPtr graph_pointer);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS CalculateAndStoreEnergyExpenditure(IntPtr graph_pointer);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS AddNodeAttributes(
			IntPtr g,
			int[] ids,
			string attribute,
			string[] scores,
			int num_nodes
		);

		[DllImport(NativeConstants.DLLPath)]
		private unsafe static extern HF_STATUS DeleteScoreArray(
			IntPtr[] scores_to_delete,
			int num_char_arrays
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetNodeAttributes(
			IntPtr g,
			string attribute,
			IntPtr[] out_scores, // Don't say that this uses strings because it'll break marshalling behavior
			ref int out_score_size
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS ClearAttributeType(
			IntPtr g,
			string attribute
		);

		[DllImport(NativeConstants.DLLPath)]
		private static extern HF_STATUS GetSizeOfGraph
		(
			IntPtr graph_ptr,
			ref int out_size
		);
	}
}

