using DHARTAPI.Exceptions;
using DHARTAPI.NativeUtils;
using DHARTAPI.NativeUtils.CommonNativeArrays;
using DHARTAPI.RayTracing;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Runtime.InteropServices;

/*! \brief Analyze the view from from points in the environment

    \details
    View analysis is based on taking the perspective of an observer in the environemnt
    from the human scale, then using raycasting to evaluate what is visible to them.
    All rays casted are distributed equally in a sphere around the observer, ensuring equal
    coverage. The observer's vertical field of view can be limited to only cast rays
    within a certain angle above and below their eye level.

    \see ViewAnalysis for all ViewAnalysis functions.
    \see DHARTAPI.RayTracing for details on raycasting. 
*/
namespace DHARTAPI.ViewAnalysis
{
	/*! 
        \brief The type of aggregation method to use for ViewAnalysis.ViewAnalysisAggregate
    */
	public enum ViewAggregateType
	{
		/// <summary> Count the number of rays that intersected any geometry. </summary>
		COUNT = 0,

		/// <summary> Add the distance from the observer to the point of intersection for all rays casted </summary>
		SUM = 1,

		/// <summary>  Average the distance from the origin to each point of intersection. </summary>
		AVERAGE = 2,

		/// <summary> The maximum distance from the origin to any hitpoint. </summary>
		MAX = 3,

		/// <summary> The minimum distance from the origin to any hitpoint. </summary>
		MIN = 4
	}

	/*!
        \brief Functions for analyzing the view of an observer in an environment at human scale.
        
        \note All of these functions internally use a raytracer, so a DHARTAPI.RayTracing.EmbreeBVH is required.
    */
	public static class ViewAnalysis
	{
		/*! 
            \brief  Conduct view analysis and aggregate the results for each node.
            
            \param bvh the Geometry to intersect with.
            \param nodes Observer locations to perform view analysis from.
            \param  ray_count Number of rays to cast. Higher values provide more 
                    accurate analysis, but increase the runtime of this function
            \param upward_fov Maximum angle in degrees above the viewer's eye level that is considered.
            \param downward_fov Maximum angle in degrees below the viewer's eye level that is considered.
            \param height Height of the observer. Nodes are offset this distance from the 
                    ground before the analysis is performed.
            \param type How the distances for all hits should be aggregated. See \link ViewAggregateType \endlink
                        for a list of all aggregation methods.
            
            \return An ordered array of floats corresponding to the aggregated score for each node in nodes.
            
            \details 
            The rays for each node will be casted in parallel using all available cores on the host's machine.
            
            \remarks 
            This function is much lighter in memory than \link ViewAnalysisStandard \endlink since all
            operations are done in place on single floats. Use this as a faster alternative 
            if the scores of nodes are all that's needed.

            \par Examples

            \snippet viewanalysis\test_viewanalysis.cs EX_GetBVH
            \snippet viewanalysis\test_viewanalysis.cs EX_ViewAnalysisAggregate
            `[480, 451, 428]`
        */
		public static ManagedFloatArray ViewAnalysisAggregate(
			EmbreeBVH bvh,
			IEnumerable<Vector3D> nodes,
			int ray_count,
			float upward_fov = 50.0f,
			float downward_fov = 70.0f,
			float height = 1.7f,
			ViewAggregateType type = ViewAggregateType.AVERAGE)
			=> new ManagedFloatArray(NativeMethods.C_SphericalViewAnalysisAggregate(
					bvh.Pointer,
					nodes,
					ray_count,
					upward_fov,
					downward_fov,
					height,
					type
				));

		/*!
            \brief Conduct View Analysis and access the results of every ray casted for every node.
            
            \param bvh Geometry to intersect with.
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

            \note 
            The hitpoints for each ray in the returned array can be determined using the directions
            SphericallyDistributeRays() with the same fov and ray_count parameters. Failures to
            intersect geometry will return -1.
            
            \par Example
            \snippet viewanalysis\test_viewanalysis.cs EX_GetBVH
            \snippet viewanalysis\test_viewanalysis.cs EX_ViewAnalysisStandard

            ```
            (0, 0, 1): ([-1, -1], [5.292, 0], [-1, -1], [-1, -1], [3.098, 0], [5.407, 0], [13.07, 0], [-1, -1], [3.529, 0], [-1, -1])
            (1, 1, 1): ([-1, -1], [5.292, 0], [-1, -1], [-1, -1], [3.098, 0], [5.407, 0], [13.07, 0], [-1, -1], [3.529, 0], [-1, -1])
            (0, 0, 3): ([-1, -1], [9.211, 0], [-1, -1], [-1, -1], [5.394, 0], [9.413, 0], [-1, -1], [-1, -1], [6.144, 0], [-1, -1])
            ```
             */
		public static ResultArray ViewAnalysisStandard(
			EmbreeBVH bvh,
			IEnumerable<Vector3D> nodes,
			int ray_count,
			float upward_fov = 50.0f,
			float downward_fov = 70.0f,
			float height = 1.7f)
			=> new ResultArray(NativeMethods.C_SphericalViewAnalysisNoAggregate(
					bvh.Pointer,
					nodes,
					ray_count,
					upward_fov,
					downward_fov,
					height
				));

		/*!
            \brief Distribute a set of points equally on a unit sphere.

            \param num_rays The number of points to distribute.
            \param upward_fov Maximum angle above the sphere to generate points in degrees,
            \param downward_fov Maximum angle in degrees downwards to generate.
            
            \returns An array of equally distributed points on a unit sphere.

            \warning
            The number of rays casted may not exactly match `ray_count` depending on the provided 
            field of view restrictions. 

            \note 
            This is the same function used internally by view analysis to equally distribute
            ray directions. 

            \par Example
            \snippet viewanalysis\test_viewanalysis.cs EX_SphericallyDistributeRays 
            `[0, -1, 0, -0.2650034, -0.8181818, . . . -0.7649929, 0.4396428, 0.8181818, 0.3705303]`
        */
		public static DirectionArray SphericallyDistributeRays(
			int num_rays,
			float upward_fov = 50f,
		    float downward_fov = 70f
		) => new DirectionArray(NativeMethods.SphericalDistribute(num_rays, upward_fov, downward_fov));
	}
}
