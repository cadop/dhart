using DHARTAPI.NativeUtils;

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace DHARTAPI.RayTracing
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
		public Int32 prim_id; ///< ID of the primitive intersected. -1 if missed

		/// </summary>
		/// <param name="in_distance"></param>
		/// <param name="in_mesh_id"></param>
		/// <param name="in_prim_id"></param>

		/*! 
            \brief Construct a new RayResult to contain the distance to an intersection and the id of the mesh intersected
            
            \param in_distance Distance to ray intersection.
            \param in_mesh_id ID of the mesh intersected.
			\param in_prim_id ID of the mesh intersected.
       */
		internal RayResult(float in_distance, int in_mesh_id, int in_prim_id)
		{
			this.distance = in_distance;
			this.mesh_id = in_mesh_id;
			this.prim_id = in_prim_id;
		}
		/*! \brief Get a string representation of this ray result
            \returns The ID and distance of this rayresult as a formatted string 
        */
		public override string ToString() => String.Format("[{0:0.###},{1}, {2}]", distance, mesh_id, prim_id);

	}

	/*! \brief An array of RayResults stored in unmanaged memory .*/
	public class RayResults : NativeArray<RayResult>
	{
		/*! 
            \brief Initializes a new instance of <see cref="RayResults"/>.
            
            \param in_ptr Info needed to wrap the managed array.
        */
		internal RayResults(CVectorAndData in_ptr) : base(in_ptr) { }

		/*!
			 \brief Free the array of structs that this pointer points to in native memory.
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