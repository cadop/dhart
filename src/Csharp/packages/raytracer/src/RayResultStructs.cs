using HumanFactors.NativeUtils;

using HumanFactors.NativeUtils;

using System;
using System.Runtime.InteropServices;

namespace HumanFactors.RayTracing
{
    /// <summary>
    /// Contains distance and mesh_id. Used for returning large amounts of results from the raytracer
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct RayResult
    {
        public float distance;
        public Int32 mesh_id;

        internal RayResult(float in_distance, int in_mesh_id)
        {
            this.distance = in_distance;
            this.mesh_id = in_mesh_id;
        }
    }

    /// <summary>
    /// An array of rayresults pointed to in unmanaged memory
    /// </summary>
    public class RayResults : NativeArray<RayResult>
    {
        private static readonly int size_multi = 8;
        /// <summary>
        /// Initializes a new instance of <see cref="RayResults"/>.
        /// </summary>
        /// <param name="in_ptr">Info needed to wrap the managed array.</param>
        internal RayResults(CVectorAndData in_ptr) : base(in_ptr)
        {
        }

        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyRayResults(handle);
            return true;
        }
    }
}