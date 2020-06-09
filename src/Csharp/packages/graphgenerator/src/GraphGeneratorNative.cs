
using HumanFactors.NativeUtils;
using HumanFactors.Exceptions;
using System;
using System.Runtime.InteropServices;

namespace HumanFactors.GraphGenerator {

    internal static class NativeMethods {
        const string dllpath = NativeConstants.DLLPath;


        internal static IntPtr C_GenerateGraph(
            IntPtr bvh,
            Vector3D start_point,
            Vector3D spacing,
            int max_nodes = -1,
            float up_step = 0.2f,
            float up_slope = 20,
            float down_step = 0.2f,
            float down_slope = 20,
            int max_step_connections = 1,
            int core_count = -1
        ) {
            IntPtr out_graph = new IntPtr();
            float[] start_arr = start_point.ToArray();
            float[] spacing_arr = spacing.ToArray();

            HF_STATUS res = GenerateGraph(
                bvh,
                start_arr,
                spacing_arr,
                max_nodes,
                up_step,
                up_slope,
                down_step,
                down_slope,
                max_step_connections,
                core_count,
                ref out_graph
            );


            // This indicates that a graph has failed to generate. 
            if (res == HF_STATUS.NO_GRAPH)
                return IntPtr.Zero;
            else 
                return out_graph;

        }

        [DllImport(dllpath)]
        private static extern HF_STATUS GenerateGraph(
            IntPtr ray_tracer,
            [In] float[] start_point,
            [In] float[] end_point,
            int MaxNodes,
            float UpStep,
            float UpSlope,
            float DownStep,
            float DownSlope,
            int max_step_connection,
            int core_count,
            ref IntPtr out_graph
        );

    }
}
