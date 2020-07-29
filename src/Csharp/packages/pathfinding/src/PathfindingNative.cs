
using HumanFactors.NativeUtils;
using HumanFactors.Exceptions;
using System;
using System.Runtime.InteropServices;
using HumanFactors.SpatialStructures;
using System.Runtime.Serialization.Json;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Diagnostics;

namespace HumanFactors.Pathfinding {
    internal static class NativeMethods{

        const string dll_path = NativeConstants.DLLPath;


        /// <summary>
        /// Generate a path from start to end. 
        /// </summary>
        /// <returns>A <see cref="CVectorAndData"/> to the generated path.</returns>
        /// <exception cref="System.IndexOutOfRangeException">Start or end points were not in the graph</exception>
        internal static CVectorAndData C_CreatePath(IntPtr graph_ptr, int start, int end, string cost_type)
        {
            // Setup our output variable so we can use its members as reference parameters.
            CVectorAndData out_ptrs = new CVectorAndData();

            // Find the path in unmanaged code. If this returns OK, then the parameters
            // of out_ptrs will be updated with the data for the path.
            HF_STATUS res = CreatePath(
                graph_ptr,
                start,
                end,
                cost_type,
                ref out_ptrs.size,
                ref out_ptrs.vector, 
                ref out_ptrs.data
            );

            // If NO_PATH is returned, that means no path exists from start to end
            // so make sure our output reflects this.

            if (res == HF_STATUS.OK)
                return out_ptrs;
            else if (res == HF_STATUS.NO_PATH)
            {
                out_ptrs.size = -1;
                out_ptrs.data = IntPtr.Zero;
                out_ptrs.vector = IntPtr.Zero;
            }
            else if (res == HF_STATUS.NO_COST) // Cost type doesn't exist in the graph
                throw new KeyNotFoundException("cost_type " + cost_type + " is not the valid key of a cost in the graph");
            else
                Debug.Assert(false);
            return out_ptrs;
        }

        /// <summary>
        /// Generate a path from start to end. 
        /// </summary>
        /// <returns>An array of <see cref="CVectorAndData"/> to the generated paths.</returns>
        /// <exception cref="System.IndexOutOfRangeException">Start or end points were not in the graph</exception>
        internal static CVectorAndData[] C_CreatePaths(
            IntPtr graph_ptr,
            IEnumerable<int> start,
            IEnumerable<int> end,
            string cost_type
        ){
            // Preallocate memory to fulfil pre-conditions
            int size = start.Count();
            int[] path_sizes = new int[size];
            IntPtr[] data = new IntPtr[size];
            IntPtr[] vectors = new IntPtr[size];


            // Call the Native funciton
            HF_STATUS res = CreatePaths(
                graph_ptr,
                start.ToArray(),
                end.ToArray(),
                cost_type,
                vectors,
                data,
                path_sizes,
                size
            );

            if (res == HF_STATUS.NO_COST)
                throw new KeyNotFoundException("Cost Type (" + cost_type + ") could not be found in the graph");

            // Read through results and fill out CVectorsAndDatas
            CVectorAndData[] out_cvads = new CVectorAndData[size];
            for(int i = 0; i < size; i++)
            {
                IntPtr data_ptr = data[i];
                IntPtr vector_ptr = vectors[i];
                int node_count = path_sizes[i];

                // If the count of this path is 0, that means no path could be found
                // and both of its pointers are null, so don't try to access them at all
                if (node_count > 0)
                    out_cvads[i] = new CVectorAndData(data_ptr, vector_ptr, node_count);
                // An empty CVectorAndData is our signal for a failed path.
                else
                    out_cvads[i] = new CVectorAndData();
                    
            }
            return out_cvads;

        }
        internal static CVectorAndData[] C_AllToAllPaths(IntPtr graph_ptr, int graph_size, string cost_type)
        {

            // Setup variables to fulfill our preconditions
            int num_paths = graph_size * graph_size; // Will return n^2 nodes
            IntPtr[] data = new IntPtr[num_paths];
            IntPtr[] vectors = new IntPtr[num_paths];
            int size = num_paths;
            int[] path_sizes = new int[size];

            // Call the Native function, populating our arrays
            HF_STATUS res = CreateAllToAllPaths(
                graph_ptr,
                cost_type,
                vectors,
                data,
                path_sizes,
                size
            );

            // Throw if the cost couldn't be found
            if (res == HF_STATUS.NO_COST)
                throw new KeyNotFoundException("Cost Type (" + cost_type + ") could not be found in the graph");

            // Read through results and fill output CVectorsAndDatas
            CVectorAndData[] out_cvads = new CVectorAndData[size];
            for (int i = 0; i < size; i++)
            {
                IntPtr data_ptr = data[i];
                IntPtr vector_ptr = vectors[i];
                int node_count = path_sizes[i];

                // If the count of this path is 0, that means no path could be found
                // and both of its pointers are null, so don't try to access them at all
                // and instead put an invalid CVectorAndData at that index
                if (node_count > 0)
                    out_cvads[i] = new CVectorAndData(data_ptr, vector_ptr, node_count);
                else
                    out_cvads[i] = new CVectorAndData();

            }
            return out_cvads;

        }

        /// <summary>
        /// Destroy an existing path object. 
        /// </summary>
        internal static void C_DestroyPath(IntPtr path_ptr) => DestroyPath(path_ptr);

        [DllImport(dll_path)]
        private static extern HF_STATUS DestroyPath(IntPtr path_ptr);
        [DllImport(dll_path)]
        private static extern HF_STATUS CreatePath(
            IntPtr graph_ptr,
            int start,
            int end,
            string cost_type,
            ref int out_size,
            ref IntPtr out_path,
            ref IntPtr out_data
        );

        [DllImport(dll_path)]
        private static extern HF_STATUS CreatePaths(
            IntPtr graph_ptr,
            [In] int[] start,
            [In] int[] end,
            string cost_type,
            [Out] IntPtr[] out_path_ptr_holder,
            [Out] IntPtr[] out_path_member_ptr_holder,
            [Out] int[] out_sizes,
            int num_paths
        );

        [DllImport(dll_path)]
        private static extern HF_STATUS CreateAllToAllPaths(
            IntPtr graph_ptr,
            string cost_type,
            [Out] IntPtr[] out_path_ptr_holder,
            [Out] IntPtr[] out_path_member_ptr_holder,
            [Out] int[] out_sizes,
            int num_paths
        );

    }
}
