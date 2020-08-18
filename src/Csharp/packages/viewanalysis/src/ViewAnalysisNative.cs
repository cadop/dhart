
using HumanFactors.NativeUtils;
using HumanFactors.Exceptions;
using System;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Json;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace HumanFactors.ViewAnalysis {

    internal static class NativeMethods
    {
        private const string dll_path = NativeConstants.DLLPath;


        internal static CVectorAndData C_SphericalViewAnalysisAggregate(
            IntPtr ert, 
            IEnumerable<Vector3D> nodes,
            int ray_count,
            float upward_fov,
            float downward_fov,
            float height,
            ViewAggregateType AT
        ) {
            CVectorAndData out_vector = new CVectorAndData();
            var flat_nodes = HelperFunctions.FlattenVectorArray(nodes);
            int size = nodes.Count();

            var res = SphereicalViewAnalysisAggregateFlat(
                ert,
                flat_nodes,
                size,
                ray_count,
                upward_fov,
                downward_fov,
                height,
                AT,
                ref out_vector.vector,
                ref out_vector.data,
                ref out_vector.size
            );

            if (res != HF_STATUS.OK)
                throw new Exception("View Analysis");
            else
                return out_vector;
            
        }

        internal static CVectorAndData C_SphericalViewAnalysisNoAggregate(
            IntPtr ert,
            IEnumerable<Vector3D> nodes,
            int ray_count,
            float upward_fov,
            float downward_fov,
            float height
        ) {
            CVectorAndData out_vector = new CVectorAndData();
            var flat_nodes = HelperFunctions.FlattenVectorArray(nodes);
            out_vector.size2 = ray_count;
            int size = nodes.Count();

            var res = SphericalViewAnalysisNoAggregateFlat(
                ert,
                flat_nodes,
                nodes.Count(),
                ref out_vector.size2,
                upward_fov,
                downward_fov,
                height,
                ref out_vector.vector,
                ref out_vector.data
            );

            out_vector.size = nodes.Count();

            if (res != HF_STATUS.OK)
                throw new Exception("View Analysis");
            else
                return out_vector;

        }

        internal static CVectorAndData SphericalDistribute(int num_rays, float up_fov, float down_fov)
        {
            CVectorAndData out_ptrs = new CVectorAndData();
            out_ptrs.size2 = 3;
            out_ptrs.size = num_rays;

            SphericalDistribute(ref out_ptrs.size, ref out_ptrs.vector, ref out_ptrs.data, up_fov, down_fov);

            return out_ptrs;
        }
        internal static HF_STATUS C_DeleteResultVector(IntPtr ptr) => DestroyRayResultVector(ptr);

        [DllImport(dll_path)]
        private static extern HF_STATUS SphereicalViewAnalysisAggregateFlat(
            IntPtr ERT,
            [In] float[] node_ptr,
            int node_size,
            int max_rays,
            float upward_fov,
            float downward_fov,
            float height,
            ViewAggregateType AT,
            ref IntPtr out_scores,
            ref IntPtr out_scores_ptr,
            ref int out_scores_size
        );

        [DllImport(dll_path)]
        private static extern HF_STATUS SphericalViewAnalysisNoAggregateFlat(
            IntPtr ERT,
            [In] float[] node_ptr,
            int node_size,
            ref int max_rays,
            float upward_fov,
            float downward_fov,
            float height,
            ref IntPtr out_scores,
            ref IntPtr out_scores_ptr
        );


        [DllImport(dll_path)]
        private static extern HF_STATUS SphericalDistribute(
            ref int num_rays,
            ref IntPtr out_direction_vector,
            ref IntPtr out_direction_data,
            float upward_fov,
            float downward_fov
        );
        [DllImport(dll_path)]
        private static extern HF_STATUS DestroyRayResultVector(IntPtr vector_ptr);


    }

}
