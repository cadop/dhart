using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;

namespace HumanFactors.RayTracing
{

    /// <summary> A wrapper for Intel's Embree raytracing library. </summary>
    public static class EmbreeRaytracer
    {
        /// <summary>
        /// Fire a single ray, and get a point in return if it intersects. If it misses, an invalid
        /// point is returned.
        /// </summary>
        /// <param name="bvh"> Geometry to intersect. </param>
        /// <param name="x"> Origin X component </param>
        /// <param name="y"> Origin Y component </param>
        /// <param name="z"> Origin Z component </param>
        /// <param name="dx"> Direction X component </param>
        /// <param name="dy"> Direction Y component </param>
        /// <param name="dz"> Direction Z component </param>
        /// <param name="max_distance">
        /// Maximum distance to consider for intersections. Set to -1 for infinite
        /// </param>
        /// <returns>
        /// A Vector3D containing the hitpoint. If no hit was detected, the point will be invalid.
        /// This can easily be checked using Vector3D's <see cref="Vector3D.IsValid()" />
        /// </returns>
        public static Vector3D IntersectForPoint(
            EmbreeBVH bvh,
            float x,
            float y,
            float z,
            float dx,
            float dy,
            float dz,
            float max_distance = -1
        ) => NativeMethods.C_IntesectPoint(bvh.Pointer, x, y, z, dx, dy, dz, max_distance);

        /// <summary>
        /// Fire a single ray from origin in direction, and get the hitpoint if it intersects.
        /// </summary>
        /// <param name="bvh"> An embree bvh containing the geometry to intersect with </param>
        /// <param name="origin"> x,y,z location where the ray is fired from </param>
        /// <param name="direction"> x,y,z direction for the ray to be fired in </param>
        /// <param name="max_distance"> Maximum distance to consider for intersection </param>
        /// <returns>
        /// <returns> A vector of <see cref="Vector3D" /> for the hitpoint of each ray fired. If a
        /// ray didn't hit, its point will be invalid, checkable using <see
        /// cref="Vector3D.IsValid()" />. </returns>
        /// </returns>
        public static Vector3D IntersectForPoint(EmbreeBVH bvh, Vector3D origin, Vector3D direction, float max_distance = -1)
            => IntersectForPoint(bvh, origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, max_distance);

        /// <summary> Fire multiple rays and recieve hitpoints in return. </summary>
        /// <param name="bvh"> Geometry to intersect with. </param>
        /// <param name="origins"> A list of x,y,z coordinates to fire rays from. </param>
        /// <param name="directions"> A list of x,y,z directions to fire in. </param>
        /// <param name="max_distance"> Maximum distance to consider for intersection. </param>
        /// <returns>
        /// A vector of <see cref="Vector3D" /> for the hitpoint of each ray fired. If a ray didn't
        /// hit, its point will be invalid, checkable using <see cref="Vector3D.IsValid()" />.
        /// </returns>
        /// <remarks>
        /// <para> Can be fired in 3 configurations: </para>
        /// <list type="bullet">
        /// <item>
        /// <font color="#2a2a2a"> Equal amount of directions/origins: Fire a ray for every pair of
        /// origin/direction in order. i.e. (origin[0], direction[0]), (origin[1], direction[1]) </font>
        /// </item>
        /// <item>
        /// <font color="#2a2a2a"> One direction, multiple origins: Fire a ray in the given
        /// direction from each origin point in origins. </font>
        /// </item>
        /// <item>
        /// <font color="#2a2a2a"> One origin, multiple directions: Fire a ray from the origin point
        /// in each direction in directions </font>
        /// </item>
        /// </list>
        /// </remarks>
        /// <exception cref="System.ArgumentException">
        /// Length of <paramref name="directions" /> and <paramref name="origins" /> did not match
        /// any of the valid cases.
        /// </exception>
        public static Vector3D[] IntersectForPoints(
            EmbreeBVH bvh,
            IEnumerable<Vector3D> origins,
            IEnumerable<Vector3D> directions,
            float max_distance
        ) => NativeMethods.C_IntersectPoints(bvh.Pointer, origins, directions, max_distance);

        /// <summary>
        /// Fire a single ray and get the distance to its hit and the meshID if it hit anything. If
        /// it missed, then distance will be -1.
        /// </summary>
        /// <param name="bvh"> An embree bvh containing the geometry to intersect with </param>
        /// <param name="origin"> x,y,z locations where is fired from </param>
        /// <param name="direction"> x,y,z direction for the ray to be fired in </param>
        /// <param name="max_distance"> Maximum distance to consider for intersection </param>
        /// <returns>
        /// A <see cref="RayResult" /> with a distance to the hitpoint and meshid it hit. If the
        /// distance is equal to -1, then the ray missed
        /// </returns>
        public static RayResult IntersectForDistance(
            EmbreeBVH bvh,
            Vector3D origin,
            Vector3D direction,
            float max_distance
        ) => NativeMethods.C_IntersectRay(bvh.Pointer, origin, direction, max_distance);

        /// <summary> Fire multiple rays for distance and meshid. </summary>
        /// <param name="bvh">
        /// An <see cref="EmbreeBVH" /> containing the geometry to intersect with.
        /// </param>
        /// <param name="origins"> x,y,z locations where rays are fired from. </param>
        /// <param name="directions"> x,y,z direction for the ray to be fired in. </param>
        /// <param name="max_distance"> Maximum distance to consider for intersection. </param>
        /// <returns>
        /// <see cref="RayResults" /> for every ray fired in order. Rays that missed will have a
        /// distance and meshid of -1.
        /// </returns>
        /// <remarks>
        /// <para> Can be fired in 3 configurations: </para>
        /// <list type="bullet">
        /// <item>
        /// <font color="#2a2a2a"> Equal amount of directions/origins: Fire a ray for every pair of
        /// origin/direction in order. i.e. (origin[0], direction[0]), (origin[1], direction[1]). </font>
        /// </item>
        /// <item>
        /// <font color="#2a2a2a"> One direction, multiple origins: Fire a ray in the given
        /// direction from each origin point in origins. </font>
        /// </item>
        /// <item>
        /// <font color="#2a2a2a"> One origin, multiple directions: Fire a ray from the origin point
        /// in each direction in directions. </font>
        /// </item>
        /// </list>
        /// </remarks>
        public static RayResults IntersectForDistances(
            EmbreeBVH bvh,
            IEnumerable<Vector3D> origins,
            IEnumerable<Vector3D> directions,
            float max_distance
        ) => new RayResults(NativeMethods.C_IntersectRays(bvh.Pointer, origins, directions, max_distance));

        /// <summary>
        /// Fire one or more occlusion rays in C++. Occlusion rays are faster than standard rays,
        /// however are only capable of returning or not they hit anything.
        /// </summary>
        /// <param name="bvh"> A valid Embree BVH. </param>
        /// <param name="origin"> One or more origins. </param>
        /// <param name="direction"> One or more directions. </param>
        /// <param name="max_distance">
        /// Maximum distance that a ray can travel. Any hits beyond this point are not counted.
        /// </param>
        /// <returns>
        /// An array of <c> true </c> or <c> false </c> values indicating hits or misses respectively.
        /// </returns>
        public static bool[] IntersectOccluded(
            EmbreeBVH bvh,
            IEnumerable<Vector3D> origin,
            IEnumerable<Vector3D> direction,
            float max_distance
        ) => NativeMethods.C_FireOcclusionRays(bvh.Pointer, origin, direction, max_distance);
    }
}