
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

namespace HumanFactors.Pathfinding {
    internal static class NativeMethods{

        const string dll_path = NativeConstants.DLLPath;


        /// <summary>
        /// Generate a path from start to end. 
        /// </summary>
        /// <returns>A <see cref="CVectorAndData"/> to the generated path.</returns>
        /// <exception cref="System.IndexOutOfRangeException">Start or end points were not in the graph</exception>
        internal static CVectorAndData C_CreatePath(IntPtr graph_ptr, int start, int end)
        {
            CVectorAndData out_ptrs = new CVectorAndData();

            HF_STATUS res = CreatePath(graph_ptr, start, end, ref out_ptrs.size, ref out_ptrs.vector, ref out_ptrs.data);

            if (res == HF_STATUS.NO_PATH) {
                out_ptrs.size = -1;
                out_ptrs.data = IntPtr.Zero;
                out_ptrs.vector = IntPtr.Zero;
            }
            else if (res == HF_STATUS.OUT_OF_RANGE)
                throw new IndexOutOfRangeException("Start or end points were not in the graph");

            return out_ptrs;
        }

        /// <summary>
        /// Generate a path from start to end. 
        /// </summary>
        /// <returns>An array of <see cref="CVectorAndData"/> to the generated paths.</returns>
        /// <exception cref="System.IndexOutOfRangeException">Start or end points were not in the graph</exception>
        internal static CVectorAndData[] C_CreatePaths(IntPtr graph_ptr, IEnumerable<int> start, IEnumerable<int> end)
        {
            int size = start.Count();
            int[] path_sizes = new int[size];
            IntPtr[] data = new IntPtr[size];
            IntPtr[] vectors = new IntPtr[size];

            HF_STATUS res = CreatePaths(graph_ptr, start.ToArray(), end.ToArray(), vectors, data, path_sizes, size);

            CVectorAndData[] out_cvads = new CVectorAndData[size];
            for(int i = 0; i < size; i++)
            {
                IntPtr data_ptr = data[i];
                IntPtr vector_ptr = vectors[i];
                int node_count = path_sizes[i];

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
            ref int out_size,
            ref IntPtr out_path,
            ref IntPtr out_data
        );

        [DllImport(dll_path)]
        private static extern HF_STATUS CreatePaths(
            IntPtr graph_ptr,
            [In] int[] start,
            [In] int[] end,
            [Out] IntPtr[] out_path_ptr_holder,
            [Out] IntPtr[] out_path_member_ptr_holder,
            [Out] int[] out_sizes,
            int num_paths
        );
    }
}
