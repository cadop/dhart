
using HumanFactors.NativeUtils;
using HumanFactors.Exceptions;
using System;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Json;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics;

namespace HumanFactors.ViewAnalysis
{

	/*! \brief Native functions for the ViewAnalysis namespace */
	internal static class NativeMethods
	{
		private const string dll_path = NativeConstants.DLLPath;

		/*!
            \brief  Conduct view analysis and aggregate the results for each node.
            
            \param ert A pointer to an Embree Raytracer.
            \param nodes Observer locations to perform view analysis from.
            \param  ray_count Number of rays to cast. Higher values provide more 
                    accurate analysis, but increase the runtime of this function
            \param upward_fov Maximum angle in degrees above the viewer's eye level that is considered.
            \param downward_fov Maximum angle in degrees below the viewer's eye level that is considered.
            \param height Height of the observer. Nodes are offset this distance from the 
                    ground before the analysis is performed.
            \param AT How the distances for all hits should be aggregated. See \link ViewAggregateType \endlink
                        for a list of all aggregation methods.
            
            \returns
            CVectorAndData for a vector of floats containing the score of each node in `nodes` in order.
            
            \details 
            The rays for each node will be casted in parallel using all available cores on the host's machine.
        */
		internal static CVectorAndData C_SphericalViewAnalysisAggregate(
			IntPtr ert,
			IEnumerable<Vector3D> nodes,
			int ray_count,
			float upward_fov,
			float downward_fov,
			float height,
			ViewAggregateType AT
		)
		{
			// Define output parameter to write pointers to
			CVectorAndData out_vector = new CVectorAndData();

			// Setup inputs
			var flat_nodes = HelperFunctions.FlattenVectorArray(nodes);
			int size = nodes.Count();

			// Call the C++ function updating the contents of out_vector
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

			// If this isn't OK then something changed in C++ that wasn't updated here. 
			Debug.Assert(res == HF_STATUS.OK);

			return out_vector;

		}

		/*!
            \brief Conduct View Analysis and access the results of every ray casted for every node.
            1
            \param ert Ray tracer to use for the view analysis
            \param nodes Observer locations to perform view analysis from.
            \param ray_count Number of rays to cast.Higher values provide more
                   accurate analysis, but increase the runtime and memory consumption
                   of this function.
            \param upward_fov Maximum angle in degrees above the viewer's eye level that is considered.
            \param downward_fov Maximum angle in degrees below the viewer's eye level that is considered.
            \param height Height of the observer. Nodes are offset this distance from the
                          ground before the analysis is performed.
            
            \return
            A `nodes.Count()` by `num_rays` 2 dimensional array of results for each node and each ray casted. 
            For example, every ray for node one is located at row 1, and the results for every ray casted
            from node 2 are located at row 2. 
        
            \details
            The rays for each node will be casted in parallel using all available cores on the host's machine.
            
            \warning
            The number of rays casted may not exactly match ray_count depending on the provided
            field of view restrictions.
        */
		internal static CVectorAndData C_SphericalViewAnalysisNoAggregate(
			IntPtr ert,
			IEnumerable<Vector3D> nodes,
			int ray_count,
			float upward_fov,
			float downward_fov,
			float height
		)
		{

			// Define pointers for C++ to update
			CVectorAndData out_vector = new CVectorAndData();

			// Set up outputs
			var flat_nodes = HelperFunctions.FlattenVectorArray(nodes);
			out_vector.size2 = ray_count;
			int size = nodes.Count();

			// Call C++ analysis, updating out_vector
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

			// The size of the vector is equal to the number of nodes in our input
			out_vector.size = nodes.Count();

			// If this isn't OK then something changed in C++ that wasn't updated here. 
			Debug.Assert(res == HF_STATUS.OK);

			return out_vector;
		}

		/*!
            \brief Distribute a set of points equally on a unit sphere.

            \param num_rays The number of points to distribute.
            \param up_fov Maximum angle above the sphere to generate points in degrees,
            \param down_fov Maximum angle in degrees downwards to generate.
            
            \returns 
            A CVectorAndData containing an array of floats for the x,y,z coordinates of each
            direction generated. 

            \warning
            The number of rays casted may not exactly match `ray_count` depending on the provided
            field of view restrictions. 

            \note
            This is the same function used internally by view analysis to equally distribute
            ray directions.
        */
		internal static CVectorAndData SphericalDistribute(int num_rays, float up_fov, float down_fov)
		{
			// Create a CVectorAndData to use for output
			CVectorAndData out_ptrs = new CVectorAndData();
			out_ptrs.size2 = 3;
			out_ptrs.size = num_rays;

			// Call the function, updating the CVectorAndData
			_ = SphericalDistribute(ref out_ptrs.size, ref out_ptrs.vector, ref out_ptrs.data, up_fov, down_fov);

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
