using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Security.Cryptography.X509Certificates;

namespace HumanFactors.RayTracing
{
    internal static class NativeMethods
    {
        private const string dllpath = NativeConstants.DLLPath;

        internal static IntPtr C_ConstructRaytracer(IntPtr mesh_info_ptr)
        {
            IntPtr ret_ptr = new IntPtr();
            HF_STATUS result = CreateRaytracer(mesh_info_ptr, ref ret_ptr);

            if (result == HF_STATUS.MISSING_DEPEND)
                throw new Exception("Missing embree3.dll or tbb.dll");
            else if (result != HF_STATUS.OK)
                throw new Exception("Something went wrong during BVH construction");
            else return ret_ptr;
        }

        internal static Vector3D C_IntesectPoint(IntPtr rt_ptr, float x, float y, float z, float dx, float dy, float dz, float max_distance = -1)
        {
            bool did_hit = false;
            _ = FireRay(rt_ptr, ref x, ref y, ref z, dx, dy, dz, max_distance, ref did_hit);

            if (did_hit) return new Vector3D(x, y, z);
            else return new Vector3D(float.NaN, float.NaN, float.NaN);
        }

        /// <summary>
        /// cs the intersect points.
        /// </summary>
        /// <param name="ert">The ert.</param>
        /// <param name="origins">The origins.</param>
        /// <param name="directions">The directions.</param>
        /// <param name="max_distance">The maximum distance.</param>
        /// <returns></returns>
        /// <exception cref="System.ArgumentException">Arguments did not match one of the predefined cases!</exception>
        /// <exception cref="System.Exception">Multiple rays failed to fire</exception>
        internal static Vector3D[] C_IntersectPoints(IntPtr ert, IEnumerable<Vector3D> origins, IEnumerable<Vector3D> directions, float max_distance)
        {
            float[] flat_origins = NativeUtils.HelperFunctions.FlattenVectorArray(origins);
            float[] flat_dirs = NativeUtils.HelperFunctions.FlattenVectorArray(directions);

            // Select C function to use based on number of origins/directions
            int num_origins = origins.Count();
            int num_directions = directions.Count();
            HF_STATUS res = HF_STATUS.GENERIC_ERROR;
            Vector3D[] out_points;
            if (num_origins == num_directions)
            {
                bool[] result_array = new bool[num_origins];
                res = FireMultipleRays(ert, flat_origins, flat_dirs, num_origins, max_distance, result_array);
                out_points = HelperFunctions.FloatArrayToVectorArray(flat_origins, result_array);
            }
            else if (num_origins > num_directions && num_directions == 1)
            {
                bool[] result_array = new bool[num_origins];
                res = FireMultipleOriginsOneDirection(ert, flat_origins, flat_dirs, num_origins, max_distance, result_array);
                out_points = HelperFunctions.FloatArrayToVectorArray(flat_origins, result_array);
            }
            else if (num_directions > num_origins && num_origins == 1)
            {
                bool[] result_array = new bool[num_directions];
                res = FireMultipleDirectionsOneOrigin(ert, flat_origins, flat_dirs, num_directions, max_distance, result_array);
                out_points = HelperFunctions.FloatArrayToVectorArray(flat_dirs, result_array);
            }
            else
                throw new ArgumentException("Arguments did not match one of the predefined cases!");

            if (res != HF_STATUS.OK)
                throw new Exception("Multiple rays failed to fire");

            return out_points;
        }

        internal static CVectorAndData C_IntersectRays(
            IntPtr ray_tracer,
            IEnumerable<Vector3D> origins,
            IEnumerable<Vector3D> directions,
            float max_distance
        )
        {
            var flat_origins = HelperFunctions.FlattenVectorArray(origins);
            var flat_directions = HelperFunctions.FlattenVectorArray(directions);
            int num_origins = origins.Count();
            int num_directions = directions.Count();

            IntPtr vector_ptr = new IntPtr();
            IntPtr data_ptr = new IntPtr();

            HF_STATUS res = FireRaysDistance(
                ray_tracer,
                flat_origins,
                num_origins,
                flat_directions,
                num_directions,
                ref vector_ptr,
                ref data_ptr
            );

            if (res != HF_STATUS.OK)
                throw new Exception("RAYS FOR DISTANCE FAIL");

            return new CVectorAndData(data_ptr, vector_ptr, origins.Count());
        }

        internal static bool[] C_FireOcclusionRays(
            IntPtr rt_ptr,
            IEnumerable<Vector3D> origins,
            IEnumerable<Vector3D> directions,
            float max_distance
        ) {
            int num_origins = origins.Count();
            int num_directions = directions.Count();
            int result_size = Math.Max(num_origins, num_directions);
            
            bool[] result_array = new bool[result_size];
            float[] origin_array = HelperFunctions.FlattenVectorArray(origins);
            float[] direction_array = HelperFunctions.FlattenVectorArray(directions);

            HF_STATUS res = FireOcclusionRays(
                rt_ptr,
                origin_array,
                direction_array,
                num_origins,
                num_directions,
                max_distance,
                result_array
            );

            return result_array;
        }

        internal static RayResult C_IntersectRay(
            IntPtr rt_ptr,
            Vector3D origin,
            Vector3D direction,
            float max_distance
        ){
            float[] origin_arr = new float[3] { origin.x, origin.y, origin.z };
            float[] direction_arr = new float[3] { direction.x, direction.y, direction.z };

            int out_meshid = 0;
            float out_distance = 0.0f;

            FireSingleRayDistance(rt_ptr, origin_arr, direction_arr, max_distance, ref out_distance, ref out_meshid);

            return new RayResult(out_distance, out_meshid);
        }

        internal static void C_DestroyRayTracer(IntPtr rt_ptr) => DestroyRayTracer(rt_ptr);

        internal static void C_DestroyRayResults(IntPtr ray_ptr) => DestroyRayResultVector(ray_ptr);

        [DllImport(dllpath)]
        private static extern HF_STATUS CreateRaytracer(
            IntPtr mesh,
            ref IntPtr out_raytracer
        );

        [DllImport(dllpath)]
        private static extern HF_STATUS FireRay(
            IntPtr ert,
            ref float x,
            ref float y,
            ref float z,
            float dx,
            float dy,
            float dz,
            float max_distance,
            ref bool result
        );

        [DllImport(dllpath)]
        private static extern HF_STATUS FireSingleRayDistance(
            IntPtr ert,
            [In] float[] origin,
            [In] float[] direction,
            float max_distance,
            ref float out_distance,
            ref int out_meshid
        );

        [DllImport(dllpath, CharSet = CharSet.Ansi)]
        private static extern HF_STATUS FireMultipleRays(
            IntPtr ert,
            [In, Out] float[] origins,
            float[] directions,
            int size,
            float max_distance,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType=UnmanagedType.I1)] // C# Bools are 1byte, C++ bools are 4 byte
            [Out]bool[] result_array                                          // Ensure this is checked for arrays or 1/4
        );                                                                    // of the values will be garbage

        [DllImport(dllpath)]
        private static extern HF_STATUS FireMultipleDirectionsOneOrigin(
            IntPtr bvh,
            [In] float[] origin,
            [In] float[] directions,
            int size,
            float max_distance,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.I1)]
            [Out] bool[] result_array
        );

        [DllImport(dllpath)]
        private static extern HF_STATUS FireMultipleOriginsOneDirection(
            IntPtr bvh,
            [In] float[] origin,
            [In] float[] directions,
            int size,
            float max_distance,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.I1)]
            [Out] bool[] result_array
        );
        
        [DllImport(dllpath)]
        private static extern HF_STATUS FireOcclusionRays(
            IntPtr bvh,
            [In] float[] origins,
            [In] float[] directions,
            int origin_size,
            int direction_size,
            float max_distance,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.I1)]
            [Out] bool[] result_array
        );
        
        [DllImport(dllpath, CharSet = CharSet.Ansi)]
        private static extern HF_STATUS DestroyRayTracer(IntPtr MeshPointer);

        [DllImport(dllpath)]
        private static extern HF_STATUS DestroyRayResultVector(IntPtr analysis);

        [DllImport(dllpath)]
        private static extern HF_STATUS FireRaysDistance(
            IntPtr ert,
            [In] float[] origins,
            int num_origins,
            [In] float[] directions,
            int num_directions,
            ref IntPtr out_results,
            ref IntPtr out_data
        );
    }
}