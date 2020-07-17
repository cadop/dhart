using HumanFactors.NativeUtils;

using System;
using System.Runtime.InteropServices;

namespace HumanFactors.RayTracing
{
    /*!
        \brief MeshID and Distance for a casted ray. 
        
        \remarks Used for returning large amounts of results from the raytracer.

        \internal 
            \todo Function .checkhit that returns false if in_distance or in_mesh_id are true
        \internal
    */
    [StructLayout(LayoutKind.Sequential)]
    public struct RayResult
    {
        public float distance; ///< Distance from origin to the point of intersection. -1 if missed.
        public Int32 mesh_id; ///< ID of the mesh intersected. -1 if missed.

        internal RayResult(float in_distance, int in_mesh_id)
        {
            this.distance = in_distance;
            this.mesh_id = in_mesh_id;
        }
    }

    /// \brief An array of RayResults stored in unmanaged memory
    public class RayResults : NativeArray<RayResult>
    {
        /// \brief Initializes a new instance of <see cref="RayResults"/>.
        /// \param in_ptr Info needed to wrap the managed array.
        internal RayResults(CVectorAndData in_ptr) : base(in_ptr) {}

        /*!
         \brief Free the me 
         \note the garbage collector will handle this automatically
         \warning Do not attempt to use this class after freeing it!
         \returns True. This is guaranteed to execute properly.  
        */
        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyRayResults(handle);
            return true;
        }
    }
}