using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters;
using System.Diagnostics;

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
                    throw new InvalidOperationException("Tried to add an edge to an alternate cost before the graph was compressed!");
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

        internal static CSRInfo C_GetCSRPointers(IntPtr graph_ptr)
        {
            IntPtr data = new IntPtr();
            IntPtr outer_indices = new IntPtr();
            IntPtr inner_indices = new IntPtr();
            int nnz = 0; int cols = 0; int rows = 0;

            HF_STATUS res = GetCSRPointers(graph_ptr, ref nnz, ref rows, ref cols, ref data, ref outer_indices, ref inner_indices);

            if (res != HF_STATUS.OK)
                throw new Exception("Failed to get CSRPtrs");

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
                    throw new InvalidOperationException("The graph wasn't compressed.");
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

        internal static void C_DestroyNodeVector(IntPtr node_ptr) => DestroyNodes(node_ptr);

        internal static void C_DestroyFloatVector(IntPtr float_vector) => DestroyFloatVector(float_vector);

        internal static void C_CompressGraph(IntPtr graph) => Compress(graph);

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
            ref IntPtr out_outer_indices_ptr
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
    }
}