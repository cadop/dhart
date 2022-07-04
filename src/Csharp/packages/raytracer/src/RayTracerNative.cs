using DHARTAPI.Exceptions;
using DHARTAPI.NativeUtils;
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

namespace DHARTAPI.RayTracing
{

	/*! \param NativeMethods for the RayTracing namespace */
	internal static class NativeMethods
	{
		// Path to the DHARTAPI C++ DLL
		private const string dllpath = NativeConstants.DLLPath;

		/*!
            \brief Create a Raytracer in C++

            \param mesh_info_ptr Pointer to an instance of MeshInfo in C++.

            \returns
            A Pointer to a raytracer containing a BVH constructed from the meshinfo
            pointed to by `mesh_info_ptr`.
                    
            \see EmbreeBVH for more information about the abstraction between
                           raytracers and BVHs.
        */
		internal static IntPtr C_ConstructRaytracer(IntPtr mesh_info_ptr, bool use_precise)
		{

			// Create a new pointer to hold the output of this function
			IntPtr ret_ptr = new IntPtr();

			// Call the function in C++. If this succeeds, then the ret_ptr will be
			// updated with a pointer to the new object
			HF_STATUS result = CreateRaytracer(mesh_info_ptr, ref ret_ptr, use_precise);

			// Right now this is never thrown due to an unset compiler switch. This
			// is a matter of changing a cmake option though, so it's here for when
			// we set that up.
			if (result == HF_STATUS.MISSING_DEPEND)
				throw new Exception("Missing embree3.dll or tbb.dll");

			// If it's not OK then something changed in the CInterface that wasn't reflected
			// in C#. This is developer problem.
			Debug.Assert(result == HF_STATUS.OK);

			// Return the new pointer.
			return ret_ptr;
		}

		/*!
			\brief Create a Raytracer in C++

			\param mesh_info_ptr arrays of pointers to a MeshInfo in C++.

			\returns
			A Pointer to a raytracer containing a BVH constructed from the meshinfo
			pointed to by `mesh_info_ptr`.

			\see EmbreeBVH for more information about the abstraction between
						   raytracers and BVHs.
		*/
		internal static IntPtr C_ConstructRaytracer(IntPtr[] mesh_info_ptr)
		{
			// Create a new pointer to hold the output of this function
			IntPtr ret_ptr = new IntPtr();

			// Call the function in C++. If this succeeds, then the ret_ptr will be
			// updated with a pointer to the new object
			HF_STATUS result = CreateRaytracerMultiMesh(mesh_info_ptr, mesh_info_ptr.Length, ref ret_ptr);

			// Right now this is never thrown due to an unset compiler switch. This
			// is a matter of changing a cmake option though, so it's here for when
			// we set that up.
			if (result == HF_STATUS.MISSING_DEPEND)
				throw new Exception("Missing embree3.dll or tbb.dll");

			// If it's not OK then something changed in the CInterface that wasn't reflected
			// in C#. This is developer problem.
			Debug.Assert(result == HF_STATUS.OK);

			// Return the new pointer.
			return ret_ptr;
		}

		internal static void C_AddMesh(IntPtr rt, IntPtr[] Meshes) => AddMeshes(rt, Meshes, Meshes.Length);
		
		/*! 
            \brief Cast a ray in C++ 
           
            \param rt_ptr The pointer to an existing raytracer
            \param x x component of the ray's origin point
            \param y y component of the ray's origin point
            \param z z component of the ray's origin point
            \param dx x component of the ray's direction
            \param dy y component of the ray's direction
            \param dz z component of the ray's direction
            \param max_distance Maximum distance to consider for intersections. Set to -1 for infinite

            \returns
            The point of intersection between the cast ray and the geometry
            in `rt_ptr` could be found, and an invalid Vector3D if the ray 
            didn't intersect any geometry.
            
        */
		internal static Vector3D C_IntersectPoint(
			IntPtr rt_ptr,
			float x,
			float y,
			float z,
			float dx,
			float dy,
			float dz,
			float max_distance = -1
		)
		{
			// Create parameter to use as an output parameter
			bool did_hit = false;

			// Call the C++ function to cast a ray
			_ = CastRay(rt_ptr, ref x, ref y, ref z, dx, dy, dz, max_distance, ref did_hit);

			// If it hit, return a new vector3d with it's coordinates
			if (did_hit) return new Vector3D(x, y, z);

			// otherwise return a vector3D of nans.
			else return new Vector3D(float.NaN, float.NaN, float.NaN);
		}

		/*! 
            \brief Cast multiple rays at once in C++.
            
            \param ert Embree Raytracer to use for this call
            \param origins Origins to cast rays from
            \param directions Directions to cast rays in
            \param max_distance The maximum distance.</param>

            \details
            Can be casted in 3 configurations: </para>
            <list type = "bullet">
            <item>
            Equal amount of directions/origins: Cast a ray for every pair of
            origin/direction in order.i.e. (origin[0], direction[0]), (origin[1], direction[1]), etc.
            </item>
            <item>
            One direction, multiple origins: Cast a ray in the given
            direction from each origin point in origins.
            </item>
            <item>
             One origin, multiple directions: Cast a ray from the origin point
            in each direction in directions
            </item>
            </list>

            \throws System.ArgumentException Arguments did not match one of the predefined cases!</exception>
            \throws System.Exception Multiple rays failed to cast 
        */
		internal static Vector3D[] C_IntersectPoints(
			IntPtr ert,
			IEnumerable<Vector3D> origins,
			IEnumerable<Vector3D> directions,
			float max_distance
		)
		{
			// Convert directions and origins to arrays. 
			float[] flat_origins = NativeUtils.HelperFunctions.FlattenVectorArray(origins);
			float[] flat_dirs = NativeUtils.HelperFunctions.FlattenVectorArray(directions);

			// Select C function to use based on number of origins/directions
			int num_origins = origins.Count();
			int num_directions = directions.Count();

			// Declare result here, since it needs to be written to later
			HF_STATUS res = HF_STATUS.GENERIC_ERROR;
			Vector3D[] out_points;

			// Depending on the configuration, this will call different functions in the 
			// C-Interface. All accomplish the same thing: Call a function, interpret the results,
			// capture the return code.
			if (num_origins == num_directions)
			{
				bool[] result_array = new bool[num_origins];
				res = CastMultipleRays(ert, flat_origins, flat_dirs, num_origins, max_distance, result_array);
				out_points = HelperFunctions.FloatArrayToVectorArray(flat_origins, result_array);
			}
			else if (num_origins > num_directions && num_directions == 1)
			{
				bool[] result_array = new bool[num_origins];
				res = CastMultipleOriginsOneDirection(ert, flat_origins, flat_dirs, num_origins, max_distance, result_array);
				out_points = HelperFunctions.FloatArrayToVectorArray(flat_origins, result_array);
			}
			else if (num_directions > num_origins && num_origins == 1)
			{
				bool[] result_array = new bool[num_directions];
				res = CastMultipleDirectionsOneOrigin(ert, flat_origins, flat_dirs, num_directions, max_distance, result_array);
				out_points = HelperFunctions.FloatArrayToVectorArray(flat_dirs, result_array);
			}
			else // If it doesn't match any configuration, throw
				throw new ArgumentException("Arguments did not match one of the predefined cases!");

			// If this is ever not HF_STATUS.OK, then something changed in the C-Interface that wasn't reflected here.
			if (res != HF_STATUS.OK) Debug.Assert(false);

			return out_points;
		}

		/*!
            \brief Cast multiple rays in C++ then recieve the distance and meshid of geometry intersected
                   by each in return.

            \param ray_tracer A pointer to the raytracer to use for intersections
            \param origins A list of x, y, z coordinates to cast rays in.
            \param directions A list of x, y, z directions to cast in.
            \param max_distance Maximum distance to consider for intersection.

            \returns
            A CVectorAndData containing the pointers to a C++ vector of RayRequests containing the results
            of the raycasts conducted by the given arguments.

            \details
            Can be casted in 3 configurations: </para>
            <list type = "bullet">
            <item>
            Equal amount of directions/origins: Cast a ray for every pair of
            origin/direction in order.i.e. (origin[0], direction[0]), (origin[1], direction[1]), etc.
            </item>
            <item>
            One direction, multiple origins: Cast a ray in the given
            direction from each origin point in origins.
            </item>
            <item>
             One origin, multiple directions: Cast a ray from the origin point
            in each direction in directions
            </item>
            </list>

            \throws System.ArgumentException
            Length of directions and origins did not match any of the valid cases.
        */
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

			HF_STATUS res = CastRaysDistance(
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

		/*!
            \brief Cast occlusion rays in C++ using embree

            \param rt_ptr A pointer to a valid raytracer in C++
            \param origins One or more origins.
            \param directions One or more directions.
            \param max_distance Maximum distance that a ray can travel. Any hits beyond this point are not counted.

            \returns An array of true or false values indicating hits or misses respectively.

            \details
            Can be casted in 3 configurations:
            </para>
            <list type = "bullet">
            <item>
            Equal amount of directions/origins: Cast a ray for every pair of
            origin/direction in order.i.e. (origin[0], direction[0]), (origin[1], direction[1]), etc.
            </item>
            <item>
            One direction, multiple origins: Cast a ray in the given
            direction from each origin point in origins.
            </item>
            <item>
             One origin, multiple directions: Cast a ray from the origin point
            in each direction in directions
            </item>
            </list>
        
        */
		internal static bool[] C_CastOcclusionRays(
			IntPtr rt_ptr,
			IEnumerable<Vector3D> origins,
			IEnumerable<Vector3D> directions,
			float max_distance
		)
		{

			// Get the size of both arrays
			int num_origins = origins.Count();
			int num_directions = directions.Count();

			// The number of results will be equal to the length
			// of the lonest input
			int result_size = Math.Max(num_origins, num_directions);

			// Create output array and convert origin/directions to arrays
			bool[] result_array = new bool[result_size];
			float[] origin_array = HelperFunctions.FlattenVectorArray(origins);
			float[] direction_array = HelperFunctions.FlattenVectorArray(directions);

			// Call to C++ and get results. This will update the result array. 
			HF_STATUS res = CastOcclusionRays(
				rt_ptr,
				origin_array,
				direction_array,
				num_origins,
				num_directions,
				max_distance,
				result_array
			);

			// Return results
			return result_array;
		}

		/*!
            \brief Cast a single ray in C++ and get the distance/meshid of the intersection
                   if it intersects anything. 

            \param rt_ptr A pointer to a valid raytracer
            \param origin Origin point of the ray to cast
            \param direction Direction to cast the ray in
            \param max_distance Maximum distance that a ray can travel. Any hits beyond this point are not counted.

            \returns A RayResult containing the distance to the ray intersection and the ID of the mesh that
            it intersected. If the ray didn't intersect anything, then both values will be set to -1.
             
       */
		internal static RayResult C_IntersectRay(
			IntPtr rt_ptr,
			Vector3D origin,
			Vector3D direction,
			float max_distance
		)
		{
			// Convert inputs to arrays
			float[] origin_arr = new float[3] { origin.x, origin.y, origin.z };
			float[] direction_arr = new float[3] { direction.x, direction.y, direction.z };

			// Setup output parameters
			int out_meshid = 0;
			float out_distance = 0.0f;

			// Cast the ray in C++. This will update out_distance, and out_meshid
			CastSingleRayDistance(rt_ptr, origin_arr, direction_arr, max_distance, ref out_distance, ref out_meshid);

			// Return the results
			return new RayResult(out_distance, out_meshid);
		}

		/*! \brief Free the memory allocated by a raytracer in C++.
            \param rt_ptr A pointer to an EmbreeRayTracer in C++
        */
		internal static void C_DestroyRayTracer(IntPtr rt_ptr) => DestroyRayTracer(rt_ptr);

		/*! \brief Free the memory allocated by a vector of rayresults in C++.
            \param ray_ptr A pointer to a vector of rayresults
        */
		internal static void C_DestroyRayResults(IntPtr ray_ptr) => DestroyRayResultVector(ray_ptr);

		[DllImport(dllpath)]
		private static extern HF_STATUS CreateRaytracer(
			IntPtr mesh,
			ref IntPtr out_raytracer,
			bool use_precise
		);

		[DllImport(dllpath)]
		private static extern HF_STATUS CreateRaytracerMultiMesh(
			IntPtr[] meshes,
			int num_meshes,
			ref IntPtr out_raytracer
		);	

		[DllImport(dllpath)]
		private static extern HF_STATUS AddMeshes(
			IntPtr ray_tracer,
			IntPtr[] meshes,
			int num_meshes
		);	
		
		[DllImport(dllpath)]
		private static extern HF_STATUS CastRay(
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
		private static extern HF_STATUS CastSingleRayDistance(
			IntPtr ert,
			[In] float[] origin,
			[In] float[] direction,
			float max_distance,
			ref float out_distance,
			ref int out_meshid
		);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		private static extern HF_STATUS CastMultipleRays(
			IntPtr ert,
			[In, Out] float[] origins,
			float[] directions,
			int size,
			float max_distance,
			[MarshalAs(UnmanagedType.LPArray, ArraySubType=UnmanagedType.I1)] // C# Bools are 1byte, C++ bools are 4 byte
            [Out]bool[] result_array                                          // Ensure this is checked for arrays or 1/4
		);                                                                    // of the values will be garbage

		[DllImport(dllpath)]
		private static extern HF_STATUS CastMultipleDirectionsOneOrigin(
			IntPtr bvh,
			[In] float[] origin,
			[In] float[] directions,
			int size,
			float max_distance,
			[MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.I1)]
			[Out] bool[] result_array
		);

		[DllImport(dllpath)]
		private static extern HF_STATUS CastMultipleOriginsOneDirection(
			IntPtr bvh,
			[In] float[] origin,
			[In] float[] directions,
			int size,
			float max_distance,
			[MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.I1)]
			[Out] bool[] result_array
		);

		[DllImport(dllpath)]
		private static extern HF_STATUS CastOcclusionRays(
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
		private static extern HF_STATUS CastRaysDistance(
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