using DHARTAPI.Exceptions;
using DHARTAPI.NativeUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;

/*!
    \brief Cast rays to determine if and where they intersect geometry.

    \details
    The basics of raytracing entail casting a ray from an origin point in a
    specific direction and determining whether or not it intersects with a
    set of geometry. Certain functions can even determine where the intersection
    occured. Generally, every RayTracer will contain a set of geometry and provide
    functions for casting rays from an origin point in a specific direciton. Some
    generate an accelerated structure from geometry called a Bounding Volume
    Hierarchy, or BVH. A BVH can drastically decrease the time it takes to
    calculate ray intersections, but can be more complicated to manage compared
    to standard buffers of geometry.


    \see EmbreeRaytracer for an implementation of a raytracer using Intel's Embree Library as a backend.
 */

namespace DHARTAPI.RayTracing
{
	/*!
        \brief Cast rays with Intel's Embree raytracing library.

        \remarks
        All functions in this class first require the creation of an EmbreeBVH with valid mesh geometry.

        \see Geometry.MeshInfo for details on converting a mesh to a format that DHARTAPI can interpret
        \see EmbreeBVH for details on generating a BVH from geometry.
    */

	public static class EmbreeRaytracer
	{
		/*!
            \brief Cast a single ray, and get a point in return if it intersects any geometry.

            \param bvh BVH of geometry to intersect.
            \param x x component of the ray's origin point
            \param y y component of the ray's origin point
            \param z z component of the ray's origin point
            \param dx x component of the ray's direction
            \param dy y component of the ray's direction
            \param dz z component of the ray's direction
            \param max_distance Maximum distance to consider for intersections. Set to -1 for infinite

            \remarks
            Consider calling \link IntersectForPoints \endlink when casting mutliple rays at once since it can
            make use of parallel processing to drastically reduce the time it takes to cast each ray.

            \returns
            A Vector3D containing the hitpoint. If no hit was detected, the point will be invalid.
            This can easily be checked using Vector3D.IsValid.

            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
            \snippet raytracer\test_raytracer.cs EX_IntForPointSingle
            `(0, 0, 0)`
            \snippet raytracer\test_raytracer.cs EX_IntForPointSingle_2
            `(NaN, NaN, NaN)`

        */

		public static Vector3D IntersectForPoint(
			EmbreeBVH bvh,
			float x,
			float y,
			float z,
			float dx,
			float dy,
			float dz,
			float max_distance = -1
		) => NativeMethods.C_IntersectPoint(bvh.Pointer, x, y, z, dx, dy, dz, max_distance);

		/*!
            \brief Cast a single ray, and get a point in return if it intersects any geometry.

            \param bvh BVH of geometry to intersect.
            \param origin Origin point to cast the ray from.
            \param direction direction to cast the ray in.
            \param max_distance Maximum distance to consider for intersections.Set to -1 for infinite

            \returns
            A Vector3D containing the hitpoint. If no hit was detected, the point will be invalid.
            This can easily be checked using Vector3D.IsValid.

            \details
            This calls the other overload with the components of origin and direction.

            \remarks
            Consider calling \link IntersectForPoints \endlink when casting mutliple rays at once since it can
            make use of parallel processing to drastically reduce the time it takes to get the
            results of every ray.
            
            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
            \snippet raytracer\test_raytracer.cs EX_IntForPointSinglexyz
            `(0, 0, 0)`
            \snippet raytracer\test_raytracer.cs EX_IntForPointSinglexyz_2
            `(NaN, NaN, NaN)`

        */

		public static Vector3D IntersectForPoint(EmbreeBVH bvh, Vector3D origin, Vector3D direction, float max_distance = -1)
			=> IntersectForPoint(bvh, origin.x, origin.y, origin.z, direction.x, direction.y, direction.z, max_distance);

		/*!
            \brief Cast multiple rays and recieve hitpoints in return.

            \param bvh  A valid BVH containing geometry to intersect with.
            \param origins A list of x,y,z coordinates to cast rays in.
            \param directions A list of x,y,z directions to cast in.
            \param max_distance Maximum distance to consider for intersection.

            \returns
            An array of Vector3D for the hitpoint of each ray casted in order. If a ray didn't intersect any
            geometry, then its point will be invalid, checkable using Vector3D.IsValid()

            \details
            Can be casted in 3 configurations: </para>
            <list type="bullet">
            <item>
            Equal amount of directions/origins: Cast a ray for every pair of
            origin/direction in order. i.e. (origin[0], direction[0]), (origin[1], direction[1]), etc.
            </item>
            <item>
            One direction, multiple origins: Cast a ray in the given
            direction from each origin point in origins.
            </item>
            <item>
             One origin, multiple directions: Cast a ray from the origin point
            in each direction in directions.
            </item>
            </list>

            \throws System.ArgumentException
            Length of directions and origins did not match any of the valid cases.

            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
            \snippet raytracer\test_raytracer.cs EX_CastRayMultiplePoints
            ```
            Origin: (0, 0, 1), Intersection: (0, 0, 0)
			Origin: (0, 1, 1), Intersection: (0, 1, 0)
			Origin: (0, 2, 1), Intersection: (0, 2, 0)
			Origin: (0, 3, 1), Intersection: (0, 3, 0)
			Origin: (0, 4, 1), Intersection: (0, 4, 0)
            ```
        */

		public static Vector3D[] IntersectForPoints(
			EmbreeBVH bvh,
			IEnumerable<Vector3D> origins,
			IEnumerable<Vector3D> directions,
			float max_distance = -1
		) => NativeMethods.C_IntersectPoints(bvh.Pointer, origins, directions, max_distance);

		/*!
            \brief Cast a single ray and get the distance to its hit and the meshID if it hit anything

            \param bvh A valid BVH containing the geometry to intersect with.
            \param origin x,y,z coordinates for the ray's origin point.
            \param direction x,y,z coordinates for the direction for the ray to be casted in.
            \param max_distance Maximum distance to consider for intersection. Set to -1 for infinite.

            \remarks
            This can be faster than \link IntersectForPoints \endlink,  as the data returned is lighter. If you only need
            the distance or meshID, this function is recommended.

            \returns
            A RayResult containing the distance to the hitpoint and meshid it hit. If the
            distance is equal to -1, then the ray did not intersect any geometry.

            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
            \snippet raytracer\test_raytracer.cs EX_IntersectForDistance
            `[1,0]`
        */

		public static RayResult IntersectForDistance(
			EmbreeBVH bvh,
			Vector3D origin,
			Vector3D direction,
			float max_distance = -1
		) => NativeMethods.C_IntersectRay(bvh.Pointer, origin, direction, max_distance);

		/*!
            \brief Cast multiple rays and recieve the distance and meshid of geometry intersected by each in return.

            \param bvh  A valid BVH containing geometry to intersect with.
            \param origins A list of x, y, z coordinates to cast rays in.
            \param directions A list of x, y, z directions to cast in.
            \param max_distance Maximum distance to consider for intersection.

            \returns
            An array of RayResults for the hitpoint of each ray casted in order. If a ray didn't intersect any
            geometry, then its distance and meshid will be -1.

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
            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
            \snippet raytracer\test_raytracer.cs EX_IntersectForDistances
            ```
            Origin: (0, 0, 0), Result: [-1,-1]
			Origin: (0, 0, 1), Result: [1,0]
			Origin: (0, 0, 2), Result: [2,0]
			Origin: (0, 0, 3), Result: [3,0]
			Origin: (0, 0, 4), Result: [4,0]
			Origin: (0, 0, 5), Result: [5,0]
            ```
        */

		public static RayResults IntersectForDistances(
			EmbreeBVH bvh,
			IEnumerable<Vector3D> origins,
			IEnumerable<Vector3D> directions,
			float max_distance = -1
		) => new RayResults(NativeMethods.C_IntersectRays(bvh.Pointer, origins, directions, max_distance));

		/*!
            \brief Determine if any geometry occludes a point from a direction.

            \param bvh A valid Embree BVH.
            \param origin One or more origins.
            \param direction One or more directions.
            \param max_distance Maximum distance that a ray can travel. Any hits beyond this point are not counted.

            \returns An array of true or false values indicating hits or misses respectively.

            \details
            Can be casted in 3 configurations:
            </para>
            <list type = "bullet" >
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

            \remarks
            Occlusion rays are the fastest raycasting function, however are only capable of returning or
            not they hit anything. This can be useful for line of sight checks. Will execute in parallel
            if multiple origins/directions are supplied.
           
            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
            \snippet raytracer\test_raytracer.cs EX_Occlusion
            `Ray 1: True, Ray 2 : False`
        */

        public static bool[] IntersectOccluded(
            EmbreeBVH bvh,
            float[] origin,
            float[] direction,
            float max_distance = -1
)
        {
            return NativeMethods.C_CastOcclusionRays(bvh.Pointer, origin, direction, max_distance);
        }

        public static bool[] IntersectOccludedUnsafe(
            EmbreeBVH bvh,
            Vector3D[] origin,
            Vector3D[] direction,
            float max_distance = -1
)
        {
            float[] origin_array = HelperFunctions.FlattenVectorArrayUnsafe(origin);
            float[] direction_array = HelperFunctions.FlattenVectorArrayUnsafe(direction);

            return NativeMethods.C_CastOcclusionRays(bvh.Pointer, origin_array, direction_array, max_distance);
        }

        public static bool[] IntersectOccluded(
			EmbreeBVH bvh,
			IEnumerable<Vector3D> origin,
			IEnumerable<Vector3D> direction,
			float max_distance = -1
		) => NativeMethods.C_CastOcclusionRays(bvh.Pointer, origin, direction, max_distance);
	}
}