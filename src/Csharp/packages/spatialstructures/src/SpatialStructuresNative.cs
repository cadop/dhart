using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters;
using System.Diagnostics;
using System.Collections;

namespace HumanFactors.SpatialStructures
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

    internal static class NativeMethods
    {
        internal static IntPtr C_CreateGraph()
        {
            IntPtr out_ptr = new IntPtr();

            var res = CreateGraph(new float[3], 0, ref out_ptr);

            if (res != HF_STATUS.OK)
                throw new Exception("Something went wrong creating the graph");

            return out_ptr;
        }

        internal static void C_AddEdge(IntPtr graph_ptr, int parent_id, int child_id, float score, string cost_type)
            => AddEdgeFromNodeIDs(graph_ptr, parent_id, child_id, score, cost_type);

        internal static void C_AddEdge(IntPtr graph_ptr, Vector3D parent, Vector3D child, float cost, string cost_type)
        {
            // Convert parent and child to arrays
            var parent_arr = parent.ToArray();
            var child_arr = child.ToArray();

            // Try to add them to the graph
            HF_STATUS stat = AddEdgeFromNodes(graph_ptr, parent_arr, child_arr, cost, cost_type);

            // Handle error codes
            switch (stat)
            {
                case HF_STATUS.OK:
                    return;
                case HF_STATUS.NOT_COMPRESSED:
                    throw new LogicError("Tried to add an edge to an alternate cost before the graph was compressed!");
                case HF_STATUS.OUT_OF_RANGE:
                    throw new InvalidCostOperation("Tried to add an alternate cost to an edge that doesn't already" +
                        "exist in the default cost set!");
            }
        }

        internal static void C_DestroyGraph(IntPtr graph_ptr) => DestroyGraph(graph_ptr);

        internal static int C_GetNodeID(IntPtr graph_ptr, float x, float y, float z)
        {
            int return_int = -1;

            float[] node_arr = { x, y, z };
            var res = GetNodeID(graph_ptr, node_arr, ref return_int);
            return return_int;
        }

        internal static CSRInfo C_GetCSRPointers(IntPtr graph_ptr, string cost_type)
        {
            IntPtr data = new IntPtr();
            IntPtr outer_indices = new IntPtr();
            IntPtr inner_indices = new IntPtr();
            int nnz = 0; int cols = 0; int rows = 0;


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

            if (res == HF_STATUS.NO_COST)
                throw new KeyNotFoundException("Cost " + cost_type + " could not be found in the graph");
            else if (res != HF_STATUS.OK)
                Debug.Assert(false); // Programmer error! Handle whatever error code was returned here!
           

            return new CSRInfo(nnz, cols, rows, data, outer_indices, inner_indices);
        }

        internal static CVectorAndData C_GetNodes(IntPtr graph_ptr)
        {
            IntPtr vector_ptr = new IntPtr();
            IntPtr data_ptr = new IntPtr();

            var res = GetAllNodesFromGraph(graph_ptr, ref vector_ptr, ref data_ptr);
            if (res != HF_STATUS.OK)
                throw new Exception("Getting nodes failed");

            int size = 0;
            res = GetSizeOfNodeVector(vector_ptr, ref size);
            if (res != HF_STATUS.OK || size <= 0)
                throw new Exception("Could not get the size of the node vector!");

            return new CVectorAndData(data_ptr, vector_ptr, size);
        }

        internal static CVectorAndData C_AggregateEdgeCosts(IntPtr graph_ptr, bool directed, GraphEdgeAggregation agg_type, string cost_type)
        {
            CVectorAndData out_ptrs = new CVectorAndData();

            var res = AggregateCosts(graph_ptr, agg_type, directed, cost_type, ref out_ptrs.vector, ref out_ptrs.data);

            Debug.Assert(res != HF_STATUS.GENERIC_ERROR);  // An exception that wasn't given an error code was caught by C++
            Debug.Assert(res != HF_STATUS.NOT_COMPRESSED); // The graph wasn't compressed before calling this function.
                                                           // Ensure callers aren't allowed to make this happen.
            switch (res)
            {
                case HF_STATUS.OK:
                    break;
                case HF_STATUS.NOT_COMPRESSED:
                    throw new LogicError("The graph wasn't compressed.");
                case HF_STATUS.NO_COST:
                    throw new KeyNotFoundException("The cost type '" + cost_type + "' did not exist already in the graph");
                default:
                    Debug.Assert(false); // There's an error code being returned that
                                         // isn't accounted for. All outputs from C++ should be handled
                                         // by explicit asserts or new exceptions.
                    break;
            }

            return out_ptrs;
        }

        internal static float C_GetEdgeCost(IntPtr graph_ptr, int parent, int child, string cost_type)
        {
           float out_float = -1;
           HF_STATUS res = GetEdgeCost(graph_ptr, parent, child, cost_type, ref out_float);

            switch (res){
                case HF_STATUS.OK:
                    break;
                case HF_STATUS.NOT_COMPRESSED:
                    throw new LogicError("The graph must be compressed to read edge costs");
                case HF_STATUS.NO_COST:
                    throw new KeyNotFoundException("The cost " + cost_type + " was not found in the graph.");
                default:
                    Debug.Assert(false, "Humanfactors is returning an error code that is not being handled");
                    break;
            }

            return out_float;
        }

        internal static void C_AddNodeAttributes(IntPtr graph_ptr, string attribute, int[] ids, string[] scores)
        {
            // Call native function
            var res = AddNodeAttributes(graph_ptr, ids, attribute, scores, scores.Length);

            Debug.Assert(res == HF_STATUS.OK, "Native code returned an error code when adding attributes to the graph");
        }

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

        internal static int C_GetGraphSize(IntPtr graph_ptr)
        {
            int out_size = -1;
            var res = GetSizeOfGraph(graph_ptr, ref out_size);


            return out_size;

        }

        internal static void C_DestroyNodeVector(IntPtr node_ptr) => DestroyNodes(node_ptr);

        internal static void C_DestroyFloatVector(IntPtr float_vector) => DestroyFloatVector(float_vector);

        internal static void C_CompressGraph(IntPtr graph) => Compress(graph);

        internal static void C_CalculateAndStoreCrossSlope(IntPtr graph) => CalculateAndStoreCrossSlope(graph);

        internal static void C_CalculateAndStoreEnergyExpenditure(IntPtr graph) => CalculateAndStoreEnergyExpenditure(graph);

        internal static void C_ClearAttributeType(IntPtr graph, string attribute_name) => ClearAttributeType(graph, attribute_name);

        [DllImport(NativeConstants.DLLPath)]
        private static extern HF_STATUS GetNodes(
            IntPtr graph,
            ref IntPtr out_vector_ptr,
            ref IntPtr out_data_ptr
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