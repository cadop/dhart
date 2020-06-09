#pragma once
#ifndef EMBREE_RAY_TRACER
#define EMBREE_RAY_TRACER

#include <rpc.h>
#include <rtcore.h>

#include <corecrt_math_defines.h>
#include <vector>
#include <array>
#define _USE_MATH_DEFINES

namespace HF {
	namespace Geometry {
		class MeshInfo;
	}

	namespace RayTracer {
		/// <summary> A simple hit struct to carry all relevant information about hits. </summary>
		struct HitStruct {
			float distance = -1.0f;  ///<Distance from the origin point to the hit point. Set to -1 if no hit was recorded.
			unsigned int meshid = -1; ///< The ID of the hit mesh. Set to -1 if no hit was recorded

			/// <summary> Determine whether or not this hitstruct contains a hit. </summary>
			/// <returns> True if the point hit, false if it did not </returns>
			bool DidHit() const;
		};

		struct FullRayRequest;
		struct Vertex;
		struct Triangle;

		//TODO: This may be better served in another class

		/// <summary> Generate a vector of directions to distribute rays in 360 degrees. </summary>
		/// <remarks> This algorithm is from the old python codebase. </remarks>
		/// <param name="step">
		/// The stepsize to use for distributing directions. Lower step sizes yield better coverage,
		/// but more rays.
		/// </param>
		/// <returns> An array of directions distributed in a sphere. </returns>
		[[deprecated]]
		std::vector<std::array<float, 3>> genSphereRays(int step);

		/// <summary> Generate a set of equally distributed directions in sphere. </summary>
		/// <remarks>
		/// The directions from this function are better distributed than those of <see
		/// cref="genSphereRays" />. The implementation of this function is based 
		/// on https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere.
		/// </remarks>
		/// <param name="numrays"> The number of directions to generate. </param>
		/// <returns> An array of equally spaced directions. </returns>
		[[deprecated]]
		std::vector<std::array<float, 3>> genFibbonacciRays(int numrays);

		/// <summary>
		/// A wrapper for Intel's Embree Library.
		/// </summary>
		/// <remarks> Provides several functions to quickly and simply perform ray intersections using Embree. This class
		/// will automatically dispose of embree buffers when destroyed, incrementing Embree's reference counters on copy, and decrementing
		/// them on deletion.  </remarks>
		class EmbreeRayTracer {
			RTCDevice device;
			RTCScene scene;
			RTCIntersectContext context;
			Triangle* triangles;
			Vertex* Vertices;

			std::vector<RTCGeometry> geometry;

		public:
			EmbreeRayTracer() {}
			EmbreeRayTracer(std::vector<HF::Geometry::MeshInfo>& MI);

			/// <summary> Create a new Raytracer and generate its BVH from a flat array of vertices. </summary>
			/// <param name="geometry">
			/// A vector of float arrays representing geometry. Every 3 arrays should form a single
			/// triangle of the mesh
			/// </param>
			/// <remarks>
			/// Use of this function is discouraged since it is slower and less memory efficent than
			/// building from an array of triangle indices and an array of vertices. Internally a
			/// hashmap is used to assign an ID to every vertex in order to generate the index array.
			/// </remarks>
			EmbreeRayTracer(const std::vector<std::array<float, 3>>& geometry);

			/// <summary>
			/// Fire a single ray and get the point it hits.
			/// </summary>
			/// <param name="dir"> Direction to fire the ray in</param>
			/// <param name="origin"> Start point of the ray. Updated to contain the hitpoint if successful.</param>
			/// <param name="distance"> Maximum distance of the ray. -1 for infinite</param>
			/// <param name="mesh_id"> The id of the only mesh for this ray to collide with. -1 for all</param>
			/// <returns> True if the ray hit and the hitpoint is updated, false otherwise.</returns>
			bool FireRay(
				std::array<float, 3>& origin,
				const std::array<float, 3>& dir,
				float distance = -1,
				int mesh_id = -1
			);

			/// <summary>
			/// Fire a single ray and get the hitpoint. <paramref name="x" />, <paramref name="y"
			/// />,and <paramref name="z" /> are overridden with the hitpoint on a successful hit.
			/// </summary>
			/// <param name="x"> x component of the ray's origin </param>
			/// <param name="y"> y component of the ray's origin </param>
			/// <param name="z"> z component of the ray's origin </param>
			/// <param name="dx"> x component of the ray's direction </param>
			/// <param name="dy"> y component of the ray's direction </param>
			/// <param name="dz"> z component of the ray's direction </param>
			/// <param name="distance">
			/// Any intersections beyond this distance are ignored. Set
			/// to -1 count any hit regardless of distance.
			/// </param>
			/// <param name="mesh_id">
			/// the id of the only mesh for this ray to collide with. Any geometry wihtout this ID
			/// is ignored
			/// </param>
			/// <returns> true if the ray hit, false otherwise </returns>
			bool FireRay(
				float& x,
				float& y,
				float& z,
				float dx,
				float dy,
				float dz,
				float distance = -1,
				int mesh_id = -1
			);

			std::vector<bool> FireRays(
				std::vector<std::array<float, 3>>& origins,
				std::vector<std::array<float, 3>>& directions,
				bool use_parallel = true,
				float max_distance = -1,
				int mesh_id = -1
			);

			/// <summary> The most basic intersection possible. </summary>
			/// <param name="x"> x component of the ray's origin </param>
			/// <param name="y"> y component of the ray's origin </param>
			/// <param name="z"> z component of the ray's origin </param>
			/// <param name="dx"> x component of the ray's direction </param>
			/// <param name="dy"> y component of the ray's direction </param>
			/// <param name="dz"> z component of the ray's direction </param>
			/// <param name="mesh_id">
			/// (UNIMPLEMENTED) the id of the only mesh for this ray to collide with. Any geometry
			/// wihtout this ID is ignored
			/// </param>
			/// <returns>
			/// A hit struct containing information about the hit, or lack there of.
			/// </returns>
			HitStruct Intersect(
				float x,
				float y,
				float z,
				float dx,
				float dy,
				float dz,
				float max_distance = -1,
				int mesh_id = -1
			);

			/// <summary>
			/// Fire an occlusion ray. Only returns true if hit, can't return hitpoint but is faster than firing a standard ray.
			/// </summary>
			/// <param name="origin"> Start point of the ray</param>
			/// <param name="direction"> Direction of the ray</param>
			/// <param name="max_dist"> Maximum distance of the ray. If negative, use infinite distance </param>
			/// <returns>True if hit, false otherwise</returns>
			bool FireOcclusionRay(
				const std::array<float, 3>& origin,
				const std::array<float, 3>& direction,
				float max_dist = -1
			);

			/// <summary> Fire multiple occlusion rays </summary>
			/// <param name="origins">
			/// A list of origins. If only one is supplied then it will be fired for every direction
			/// in directions
			/// </param>
			/// <param name="directions">
			/// A list of directions. If only one is supplied then it will eb fired for every origin
			/// in origins
			/// </param>
			/// <param name="max_distance"> Maximum distance the ray can travel </param>
			/// <param name="parallel"> Whether or not to fire the rays in parallel. </param>
			/// <returns>
			/// An ordered array of bools where every true indicates a hit and every false indicates
			/// a miss.
			/// </returns>
			/// <remarks>
			/// <para> Can be fired in 3 configurations: </para>
			/// <list type="bullet">
			/// <item>
			/// <font color="#2a2a2a"> Equal amount of directions/origins: Fire a ray for every pair
			/// of origin/direction in order. i.e. (origin[0], direction[0]), (origin[1],
			/// direction[1]) </font>
			/// </item>
			/// <item>
			/// <font color="#2a2a2a"> One direction, multiple origins: Fire a ray in the given
			/// direction from each origin point in origins. </font>
			/// </item>
			/// <item>
			/// <font color="#2a2a2a"> One origin, multiple directions: Fire a ray from the origin
			/// point in each direction in directions </font>
			/// </item>
			/// </list>
			/// </remarks>
			std::vector<bool> FireOcclusionRays(
				const std::vector<std::array<float, 3>>& origins,
				const std::vector<std::array<float, 3>>& directions,
				float max_distance = -1
				, bool use_parallel = true
			);

			/// <summary>
			/// Fire a single occlusion ray. 
			/// </summary>
			/// <param name="x"> x component of the ray's origin. </param>
			/// <param name="y"> y component of the ray's origin. </param>
			/// <param name="z"> z component of the ray's origin. </param>
			/// <param name="dx"> x component of the ray's direction. </param>
			/// <param name="dy"> y component of the ray's direction. </param>
			/// <param name="dz"> z component of the ray's direction. </param>
			/// <param name="distance"> maximum distance of the ray. -1 for infinite. </param>
			/// <param name="mesh_id"> (NOT IMPLEMENTED) The id of the only mesh for this ray to collide with. -1 for all. </param>
			/// <returns> True if the ray hit anything, false otherwise</returns>
			bool FireOcclusionRay(
				float x,
				float y,
				float z,
				float dx,
				float dy,
				float dz,
				float distance = -1,
				int mesh_id = -1
			);

			/// <summary> Fire a vector of ray requests. </summary>
			/// <param name="requests">
			/// A list of requests to fire. These requests will be updated with the results of the shots
			/// </param>
			/// <param name="parallel"> If true will fire rays in parallel </param>
			void FireRequests(
				std::vector<FullRayRequest>& requests,
				bool parallel = true
			);

			/// <summary> Fire a vector of occlusion ray requests. </summary>
			/// <param name="requests">
			/// A list of requests to fire. Each request will be updated with hitpoints if successful.
			/// </param>
			/// <param name="parallel"> If true will fire rays in parallel </param>
			[[deprecated]]
			void FireOcclusionRequests(
				std::vector<FullRayRequest>& requests,
				bool parallel = true
			);

			/// <summary>
			/// Add a new mesh to the Scene with the specified ID. If False, then the addition failed, or the ID was already Taken
			/// </summary>
			/// <param name="Mesh">A vector of 3d points composing the mesh</param>
			/// <param name="ID">the id of the mesh</param>
			/// <param name="Commit">Whether or not to commit changes yet. This is slow, so only do this when you're done adding meshes</param>
			/// <returns>True if successful</returns>
			bool InsertNewMesh(std::vector<std::array<float, 3>>& Mesh, int ID, bool Commit = false);

			/// <summary>
			///	Insert a single new mesh to the scene.
			/// If the mesh has an ID it will be added to the scene with that ID.
			/// If not, then a new ID will be assigned and the mesh will be updated.
			/// </summary>
			/// <param name="Meshes"> A vector of meshinfo to each be added as a seperate mesh</param>
			/// <param name="ID">the id of the mesh</param>
			/// <param name="Commit">Whether or not to commit changes yet. This is slow, so only do this when you're done adding meshes</param>
			/// <returns>True if successful</returns>
			bool EmbreeRayTracer::InsertNewMesh(HF::Geometry::MeshInfo& Mesh, bool Commit);

			/// <summary>
			/// Add several new meshes to the Scene. If False, then the addition failed, or the ID was already taken
			/// </summary>
			/// <param name="Meshes"> A vector of meshinfo to each be added as a seperate mesh</param>
			/// <param name="ID">the id of the mesh</param>
			/// <param name="Commit">Whether or not to commit changes yet. This is slow, so only do this when you're done adding meshes</param>
			/// <returns>True if successful</returns>
			bool InsertNewMesh(std::vector<HF::Geometry::MeshInfo>& Meshes, bool Commit = true);

			/// <summary>
			/// Fire a ray from node in direction
			/// </summary>
			/// <param name="node"> A point in space. Must atleast have [0], [1], and [2] defined</param>
			/// <param name="direction"> Direction to fire the ray in. Same constraints as node, but can be a different type</param>
			/// <param name="out_distance"> distance from the ray to the hit (if any) </param>
			/// <param name="out_meshid"> ID of the mesh hit(if any) </param>
			/// <param name="max_distance"> Maximum distance the ray can travel. </param>
			/// <returns> true if the ray connected with anything, false otherwise.</returns>
			template <typename N, typename V>
			bool FireAnyRay(
				const N& node,
				const V& direction,
				float& out_distance,
				int& out_meshid,
				float max_distance = -1.0f
			) {
				auto result = Intersect(
					node[0], node[1], node[2],
					direction[0], direction[1], direction[2], max_distance
				);

				if (!result.DidHit()) return false;
				else {
					out_distance = result.distance;
					out_meshid = result.meshid;
					return true;
				}
			}

			template <typename N, typename V>
			bool FireAnyOcclusionRay(
				const N& origin,
				const V& direction,
				float max_distance = -1.0f,
				int mesh_id = -1
			) {
				return FireOcclusionRay(
					origin[0], origin[1], origin[2],
					direction[0], direction[1], direction[2],
					max_distance, mesh_id
				);
			}

			// Increment reference counters to prevent destruction when this thing goes out of scope
			void operator=(const EmbreeRayTracer& ERT2);

			/// /// <summary>
			/// Custom destructor to ensure cleanup of embree resources
			/// </summary>
			~EmbreeRayTracer();
		};
	}
}
#endif
