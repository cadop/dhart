using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using HumanFactors.RayTracing;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Runtime.InteropServices;

namespace HumanFactors.ViewAnalysis
{
    /// <summary>
    /// The type of aggregation method to use for <see
    /// cref="ViewAnalysis.ViewAnalysisAggregate(EmbreeBVH, IEnumerable{Vector3D}, int, float,
    /// float, float, ViewAggregateType)" />.
    /// </summary>
    public enum ViewAggregateType
    {
        /// <summary> The number of rays that hit an object. </summary>
        COUNT = 0,

        /// <summary> The sum of distances from the origin to each hitpoint. </summary>
        SUM = 1,

        /// <summary> The average of the distance from the origin to each hit point. </summary>
        AVERAGE = 2,

        /// <summary> The maximum distance from the origin to any hitpoint. </summary>
        MAX = 3,

        /// <summary> The minimum distance from the origin to any hitpoint. </summary>
        MIN = 4
    }

    /// <summary> Contains functions for performing view analysis. </summary>
    public static class ViewAnalysis
    {
        /// <summary> Conduct view analysis and aggregate the results for each node. </summary>
        /// <param name="bvh"> The Geometry to intersect with. </param>
        /// <param name="nodes"> Points to perform view analysis from. </param>
        /// <param name="ray_count">
        /// Number of rays to fire. Higher values provide more accurate analysis, but increase the runtime.
        /// </param>
        /// <param name="upward_fov">
        /// Maximum angle in degrees above the viewer's eye level that is considered.
        /// </param>
        /// <param name="downward_fov">
        /// Maximum angle in degrees below the viewer's eye level that is considered.
        /// </param>
        /// <param name="height">
        /// Height of the observer. Nodes are offset this distance from the ground before analysis
        /// is performed
        /// </param>
        /// <param name="type">
        /// How the distances for all hits should be aggregated. See <see cref="ViewAggregateType"
        /// /> for a list of all types and what they do.
        /// </param>
        /// <returns>
        /// An ordered array of floats corresponding to the score for each node in nodes.
        /// </returns>
        /// <remarks>
        /// This function is much lighter in memory than <see
        /// cref="ViewAnalysis.ViewAnalysisStandard(EmbreeBVH, IEnumerable{Vector3D}, int, float,
        /// float, float)" /> since all operations are done in place on single floats. Use this as a
        /// faster alternative if the scores are all that's needed.
        /// </remarks>
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

        /// <summary> Conduct view analysis and get the results for each ray hit. </summary>
        /// <param name="bvh"> The Geometry to intersect with. </param>
        /// <param name="nodes"> Points to perform view analysis from. </param>
        /// <param name="ray_count">
        /// Number of rays to fire. Higher values provide more accurate analysis, but increase the runtime.
        /// </param>
        /// <param name="upward_fov">
        /// Maximum angle in degrees above the viewer's eye level that is considered.
        /// </param>
        /// <param name="downward_fov">
        /// Maximum angle in degrees below the viewer's eye level that is considered.
        /// </param>
        /// <param name="height">
        /// Height of the observer. Nodes are offset this distance from the ground before analysis
        /// is performed.
        /// </param>
        /// <returns>
        /// A 2 dimensional array of results for each node and each ray fired. For example, every
        /// ray for node one is located at row 1, and the results for node 2 are located at row 2.
        /// </returns>
        /// <remarks>
        /// Note that the number of rays fired may exactly match <paramref name="ray_count" />
        /// depending on the provided field of view restrictions. The hitpoints for each ray in the
        /// returned array can be determined using the directions from <see
        /// cref="SphericallyDistributeRays(int, float, float)" />.
        /// </remarks>
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

        /// <summary> Distribute directions equally in a sphere. </summary>
        /// <param name="num_rays"> The number of rays to generate. </param>
        /// <param name="upward_fov"> Maximum angle in degrees upwards to generate . </param>
        /// <param name="downward_fov"> Maximum angle in degrees downwards to generate. </param>
        /// <returns> An array of equally distributed directions. </returns>
        public static DirectionArray SphericallyDistributeRays(
            int num_rays,
            float upward_fov = 50f
            , float downward_fov = 70f
        ) => new DirectionArray(NativeMethods.SphericalDistribute(num_rays, upward_fov, downward_fov));
    }
}