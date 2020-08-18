
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

namespace HumanFactors.VisibilityGraph {
    internal static class NativeMethods
    {
        private const string dllpath = NativeConstants.DLLPath;

        internal static IntPtr C_AllToAllVisibilityGraph(IntPtr bvh, IEnumerable<Vector3D> nodes, float height, bool directed = false)
        {
            IntPtr out_ptr = new IntPtr();
            HF_STATUS res;
            var flat_nodes = HelperFunctions.FlattenVectorArray(nodes);
            if (directed == true)
                res = CreateVisibilityGraphAllToAll(bvh, flat_nodes, nodes.Count(), ref out_ptr, height);
            else
                res = CreateVisibilityGraphAllToAllUndirected(bvh, flat_nodes, nodes.Count(), ref out_ptr, height);
                
            if (res == HF_STATUS.NO_GRAPH) return IntPtr.Zero;

            else return out_ptr;
        }

        internal static IntPtr C_GroupToGroupVisibilityGraph(IntPtr bvh, IEnumerable<Vector3D> group_a, IEnumerable<Vector3D> group_b, float height)
        {
            IntPtr out_ptr = new IntPtr();
            var flat_group_a = HelperFunctions.FlattenVectorArray(group_a);
            var flat_group_b = HelperFunctions.FlattenVectorArray(group_b);
            
            HF_STATUS res = CreateVisibilityGraphGroupToGroup(bvh, flat_group_a, flat_group_a.Length/3, flat_group_b, flat_group_b.Length/3, ref out_ptr, height);

            if (res == HF_STATUS.NO_GRAPH) return IntPtr.Zero;

            else return out_ptr;
        }

        [DllImport(dllpath)]
        private static extern HF_STATUS CreateVisibilityGraphAllToAll(
            IntPtr ert,
            [In] float[] nodes,
            int num_nodes,
            ref IntPtr out_graph,
            float height
        );

        [DllImport(dllpath)]
        private static extern HF_STATUS CreateVisibilityGraphAllToAllUndirected(
            IntPtr ert,
            [In] float[] nodes,
            int num_nodes,
            ref IntPtr out_graph,
            float height
        );

        [DllImport(dllpath)]
        private static extern HF_STATUS CreateVisibilityGraphGroupToGroup(
            IntPtr ert,
            [In] float[] group_a,
            int size_a,
            [In] float[] group_b,
            int size_b,
            ref IntPtr out_graph,
            float height
        );




    }
  
}
