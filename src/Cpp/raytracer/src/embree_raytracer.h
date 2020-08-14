///
/// \file		embree_raytracer.h
/// \brief		Contains definitions for the <see cref="HF::RayTracer::EmbreeRayTracer">EmbreeRayTracer</see>
///
///	\author		TBA
///	\date		26 Jun 2020

#pragma once
#ifndef EMBREE_RAY_TRACER
#define EMBREE_RAY_TRACER

#include <rpc.h>
#include <rtcore.h>

#include <corecrt_math_defines.h>
#include <vector>
#include <array>
#define _USE_MATH_DEFINES

namespace HF::Geometry {
	class MeshInfo;
}

/*!
	\brief Cast rays to determine if and where they intersect geometry.

	\details
	The basics of raytracing entail casting a ray from an origin point in a specific direction and determining
	whether or not it intersects with a set of geometry. Certain functions can even determine where the intersection
	occured. Generally, every RayTracer will contain a set of geometry and provide functions for casting rays
	from an origin point in a specific direciton. Some generate an accelerated structure from geometry called
	a Bounding Volume Hierarchy, or BVH. A BVH can drastically decrease the time it takes to calculate ray
	intersections, but can be more complicated to manage compared to standard buffers of geometry. 

	\see EmbreeRayTracer For an implementation of a raytracer using Intel's Embree library as a backend.
*/
namespace HF::RayTracer {
	struct Vector3D {
		double x; double y; double z;

		inline Vector3D operator-(const Vector3D& v2) const {
			return Vector3D{
				x - v2.x,
				y - v2.y,
				z - v2.z
			};
		}

	};

	/// <summary> A simple hit struct to carry all relevant information about hits. </summary>
	struct HitStruct {
		float distance = -1.0f;  ///< Distance from the origin point to the hit point. Set to -1 if no hit was recorded.
		unsigned int meshid = -1; ///< The ID of the hit mesh. Set to -1 if no hit was recorded

		/// <summary> Determine whether or not this hitstruct contains a hit. </summary>
		/// <returns> True if the point hit, false if it did not </returns>
		bool DidHit() const;
	};

	struct FullRayRequest;
	struct Vertex;
	struct Triangle;

	/// <summary> A wrapper for Intel's Embree Library. </summary>
	/// <remarks>
	/// Provides several functions to quickly and simply perform ray intersections using Embree.
	/// This class will automatically dispose of Embree buffers when destroyed, incrementing
	/// Embree's reference counters on copy, and decrementing them on deletion. This class also
	/// provides methods for adding geometry to Embree's geometry buffers.
	/// </remarks>
	/// \todo Could use a constructor overload for a single instance of meshinfo for conveience. 
	/// \todo Many functions support MeshID but don't actually check for it. 
	class EmbreeRayTracer {
		/// All objects in Embree are created from this. https://www.embree.org/api.html#device-object
		RTCDevice device; 
		/// Container for a set of geometries, and the BVH. https://www.embree.org/api.html#scene-object
		RTCScene scene;
		/// Context to fire rays within.
		RTCIntersectContext context;
		/// Triangle buffer. Is used in multiple places, but contents are dumped.
		Triangle* triangles;
		/// Vertex buffer. Is used in multiple places but contents are dumped.
		Vertex* Vertices;

		bool use_precise = false; ///< If true, use custom triangle intersection intersection instead of embree's

		std::vector<RTCGeometry> geometry; //> A list of the geometry being used by RTCScene.

	public:
		/// <summary>Create an EmbreeRayTracer with no arguments</summary>

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// For brevity
				using HF::RayTracer::EmbreeRayTracer;

				// Create the EmbreeRayTracer, no arguments
				EmbreeRayTracer ert;
			\endcode
		*/
		EmbreeRayTracer() {}

		/// <summary> Create a new EmbreeRayTracer and add a single mesh to the scene. </summary>
		/// <param name="MI"> The mesh to use for scene construction. </param>
		/// <exception cref="Exception"> Thrown if MI contains no vertices. </exception>
		/// \todo This function calls
		/// <c> throw; </c>
		/// <a href="https://en.cppreference.com/w/cpp/language/throw"> </a>
		/// which is meant to only be used to rethrow previously thrown exceptions. This should
		/// be throwing
		/// <see cref="HF::Exceptions::InvalidOBJ" />

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// For brevity
				using HF::Geometry::MeshInfo;
				using HF::RayTracer::EmbreeRayTracer;

				// Prepare the obj file path
				std::string teapot_path = "teapot.obj";
				std::vector<MeshInfo> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

				// Create the EmbreeRayTracer
				auto ert = EmbreeRayTracer(geom);
			\endcode
		*/
		EmbreeRayTracer(std::vector<HF::Geometry::MeshInfo>& MI, bool use_precise_intersection = false);


		/*! \brief Construct a raytracer using another raytracer.
		
			\details Increments Embree's internal garbage collector to retain the scene and
					 context. 
		*/
		EmbreeRayTracer(const EmbreeRayTracer& ERT2);

		/// <summary>
		/// Create a new Raytracer and generate its BVH from a flat array of vertices.
		/// </summary>
		/// <param name="geometry">
		/// A vector of float arrays representing geometry. Every 3 arrays should form a single
		/// triangle of the mesh
		/// </param>
		/// <remarks>
		/// Use of this function is discouraged since it is slower and less memory efficent than
		/// building from an array of triangle indices and an array of vertices. Internally a
		/// hashmap is used to assign an ID to every vertex in order to generate the index array.
		/// </remarks>
		/// <exception cref="std::exception">
		/// Embree's geometry buffers could not be initialized.
		/// </exception>

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// Create a container of coordinates
				std::vector<std::array<float, 3>> directions = {
					{0, 0, 1},
					{0, 1, 0},
					{1, 0, 0},
					{-1, 0, 0},
					{0, -1, 0},
					{0, 0, -1},
				};

				// Create the EmbreeRayTracer
				auto ert = HF::RayTracer::EmbreeRayTracer(directions);
			\endcode
		*/
		EmbreeRayTracer(const std::vector<std::array<float, 3>>& geometry);

		/// <summary>
		/// Cast a ray and overwrite the origin with the hitpoint if it intersects any geometry.
		/// </summary>
		/// <param name="dir"> Direction to fire the ray in. </param>
		/// <param name="origin">
		/// Start point of the ray. Updated to contain the hitpoint if successful.
		/// </param>
		/// <param name="distance">
		/// Any hits beyond this distance are ignored. Set to -1 to count all intersections
		/// regardless of distance.
		/// </param>
		/// <param name="mesh_id">
		/// (UNUSED) The id of the only mesh for this ray to collide with. -1 for all
		/// </param>
		/// <returns>
		/// True if the ray intersected some geometry and the origin is updated with the hit
		/// point. False otherwise.
		/// </returns>
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const std::vector<float> plane_vertices{
							-10.0f, 10.0f, 0.0f,
							-10.0f, -10.0f, 0.0f,
							10.0f, 10.0f, 0.0f,
							10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Fire a ray straight down
				std::array<float, 3> origin{ 0,0,1 };
				bool res = ert.FireRay(
					origin,
					std::array<float, 3>{0, 0, -1}
				);

				// Print Results
				if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Fire a ray straight up
				origin = std::array<float, 3>{ 0, 0, 1 };
				res = ert.FireRay(
					origin,
					std::array<float, 3>{0, 0, 1}
				);

				// Print Results
				if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;
			\endcode

			` >>>(0,0,0)`\n
			` >>>Miss`
		*/
		bool FireRay(
			std::array<float, 3>& origin,
			const std::array<float, 3>& dir,
			float distance = -1,
			int mesh_id = -1
		);

		/// <summary>
		/// Cast a single ray and get the hitpoint. <paramref name="x" />, <paramref name="y"
		/// />,and <paramref name="z" /> are overridden with the hitpoint on a successful hit.
		/// </summary>
		/// <param name="x"> x component of the ray's origin. </param>
		/// <param name="y"> y component of the ray's origin. </param>
		/// <param name="z"> z component of the ray's origin. </param>
		/// <param name="dx"> x component of the ray's direction. </param>
		/// <param name="dy"> y component of the ray's direction. </param>
		/// <param name="dz"> z component of the ray's direction. </param>
		/// <param name="distance">
		/// Any intersections beyond this distance are ignored. Set to -1 count any hit
		/// regardless of distance.
		/// </param>
		/// <param name="mesh_id"> (UNUSED)
		/// The id of the only mesh for this ray to collide with. Any geometry wihtout this ID
		/// is ignored
		/// </param>
		/// \warning The ray direction must be a unit vector.
		/// <returns> true if the ray hit, false otherwise </returns>
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				bool res;

				// Fire a ray straight down directly at the plane
				float x = 0; float y = 0; float z = 1;
				res = ert.FireRay(x, y, z, 0, 0, -1);

				// Print output
				if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Fire a ray straight up
				x = 0; y = 0; z = 1;
				res = ert.FireRay(x, y, z, 0, 0, 1);

				// Print output
				if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;
			\endcode

			`>>>(0, 0, 0)`\n
			`>>>Miss`
		*/
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


		std::array<Vector3D, 3> GetTriangle(int geomID, int primID);

		HitStruct FirePreciseRay(float x, float y, float z, float dx, float dy, float dz, float distance, int mesh_id);

		

		/// <summary> Fire multiple rays and recieve hitpoints in return. </summary>
		/// <param name="origins"> An array of x,y,z coordinates to fire rays from. </param>
		/// <param name="directions"> An array of x,y,z directions to fire in. </param>
		/// <param name="use_parallel">
		/// Cast rays in parallel if true, if not fire in serial. All available cores will be used.
		/// </param>
		/// <param name="max_distance">
		/// Maximum distance the ray can travel. Any intersections beyond this distance will be
		/// ignored. If set to 1, all intersections will be counted regardless of distance.
		/// </param>
		/// <param name="mesh_id"> (UNUSED) Only intersect with the mesh of this ID </param>
		/// <returns>
		/// A vector of <see cref="Vector3D" /> for the hitpoint of each ray fired. If a ray
		/// didn't hit, its point will be invalid, checkable using <see
		/// cref="Vector3D.IsValid()" />.
		/// </returns>
		/// <remarks>
		/// <para> Can be fired in 3 configurations: </para>
		/// <list type="bullet">
		/// <item>
		/// Equal amount of directions/origins: Fire a ray for every pair of origin/direction in
		/// order. i.e. (origin[0], direction[0]), (origin[1], direction[1])
		/// </item>
		/// <item>
		/// One direction, multiple origins: Fire a ray in the given direction from each origin
		/// point in origins.
		/// </item>
		/// <item>
		/// One origin, multiple directions: Fire a ray from the origin point in each direction
		/// in directions.
		/// </item>
		/// </list>
		/// </remarks>
		/// <exception cref="System.ArgumentException">
		/// Length of <paramref name="directions" /> and <paramref name="origins" /> did not
		/// match any of the valid cases.
		/// </exception>
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Create an array of directions all containing {0,0,-1}
				std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{ 0, 0, -1 });

				// Create an array of origin points moving further to the left with each point
				std::vector<std::array<float, 3>> origins(10);
				for (int i = 0; i < 10; i++) origins[i] = std::array<float, 3>{static_cast<float>(2 * i), 0, 1};

				// Fire every ray.
				auto results = ert.FireRays(origins, directions);

				// Print results
				std::cout << "[";
				for (int i = 0; i < 10; i++) {
					if (results[i])
						std::cout << "(" << origins[i][0] << ", " << origins[i][1] << ", " << origins[i][2] << ")";
					else
						std::cout << "Miss";

					if (i != 9) std::cout << ", ";
				}
				std::cout << "]" << std::endl;
			\endcode

			` >>> [(0, 0, 0), (1.99, 0, 0), (3.98, 0, 0), (5.97, 0, 0), (7.96, 0, 0), (9.95, 0, 0), Miss, Miss, Miss, Miss] `
		*/
		std::vector<char> FireRays(
			std::vector<std::array<float, 3>>& origins,
			std::vector<std::array<float, 3>>& directions,
			bool use_parallel = true,
			float max_distance = -1,
			int mesh_id = -1
		);

		/// <summary> Cast a ray and store all relevant information in a HitStruct. </summary>
		/// <param name="x"> x component of the ray's origin. </param>
		/// <param name="y"> y component of the ray's origin. </param>
		/// <param name="z"> z component of the ray's origin. </param>
		/// <param name="dx"> x component of the ray's direction. </param>
		/// <param name="dy"> y component of the ray's direction. </param>
		/// <param name="dz"> z component of the ray's direction. </param>
		/// <param name="max_distance">
		/// Maximum distance the ray can travel. Any intersections beyond this distance will be
		/// ignored. If set to -1, all intersections will be counted regardless of distance.
		/// </param>
		/// <param name="mesh_id">
		/// (UNIMPLEMENTED) the id of the only mesh for this ray to collide with. Any geometry
		/// wihtout this ID is ignored.
		/// </param>
		/// <returns> A HitStruct containing information about the intersection if any occurred. </returns>
		/**
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				HitStruct res;

				// Fire a ray straight down
				res = ert.Intersect(0, 0, 1, 0, 0, -1);

				// Print distance if it connected
				if (res.DidHit()) std::cerr << res.distance << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Fire a ray straight up
				res = ert.Intersect(0, 0, 1, 0, 0, 1);

				//Print distance if it connected
				if (res.DidHit()) std::cerr << res.distance << std::endl;
				else std::cerr << "Miss" << std::endl;
			\endcode

			`>>>1`\n
			`>>>Miss`
		 */
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

		/// <summary> Cast an occlusion ray using arrays as input. </summary>
		/// <param name="origin"> Start point of the ray. </param>
		/// <param name="direction"> Direction of the ray. </param>
		/// <param name="max_dist">
		/// Maximum distance of the ray. Any hits beyond this distance will not be counted. If
		/// negative, count all hits regardless of distance.
		/// </param>
		/// <returns> True if The ray intersected any geometry, False otherwise. </returns>
		/// <remarks>
		/// Occulsion rays are faster than the other ray functions, but can only tell if a ray
		/// intersected any geometry or it didn't. use the other functions for information such
		/// as distance, hitpoint, or meshid.
		/// </remarks>
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Fire a ray straight down
				bool res = ert.FireOcclusionRay(
					std::array<float, 3>{ 0, 0, 1 },
					std::array<float, 3>{ 0, 0, -1 }
				);

				// Print Result
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;

				// Fire a ray straight up
				res = ert.FireOcclusionRay(
					std::array<float, 3>{ 0, 0, 1 },
					std::array<float, 3>{ 0, 0, 1 }
				);

				// Print Result.
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;
			\endcode
			
			`>>> True`\n
			`>>> False`
		*/
		bool FireOcclusionRay(
			const std::array<float, 3>& origin,
			const std::array<float, 3>& direction,
			float max_dist = -1
		);

		/// <summary> Cast multiple occlusion rays in parallel. </summary>
		/// <param name="origins">
		/// A list of origins. If only one is supplied then it will be fired for every direction
		/// in directions.
		/// </param>
		/// <param name="directions">
		/// A list of directions. If only one is supplied then it will be fired for every origin
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
		/// Equal amount of directions/origins: Fire a ray for every pair of origin/direction in
		/// order. i.e. (origin[0], direction[0]), (origin[1], direction[1])
		/// </item>
		/// <item>
		/// One direction, multiple origins: Fire a ray in the given direction from each origin
		/// point in origins.
		/// </item>
		/// <item>
		/// One origin, multiple directions: Fire a ray from the origin point in each direction
		/// in directions.
		/// </item>
		/// </list>
		/// </remarks>
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Create an array of directions all containing {0,0,-1}
				std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{0, 0, -1});

				// Create an array of origins with the first 5 values being above the plane and
				// the last five values being under it.
				std::vector<std::array<float, 3>> origins(10);
				for (int i = 0; i < 5; i++) origins[i] = std::array<float, 3>{ 0.0f, 0.0f, 1.0f };
				for (int i = 5; i < 10; i++) origins[i] = std::array<float, 3>{ 0.0f, 0.0f, -1.0f };

				// Cast every occlusion ray
				std::vector<char> results = ert.FireOcclusionRays(origins, directions);

				// Iterate through all results to print them
				std::cout << "[";
				for (int i = 0; i < 10; i++) {
					// Print true if the ray intersected, false otherwise
					if (results[i]) std::cout << "True";
					else std::cout << "False";

					// Add a comma if it's not the last member
					if (i != 9) std::cout << ", ";
				}
				std::cout << "]" << std::endl;
			\endcode

			`>>> [True, True, True, True, True, False, False, False, False, False]`
		*/
		std::vector<char> FireOcclusionRays(
			const std::vector<std::array<float, 3>>& origins,
			const std::vector<std::array<float, 3>>& directions,
			float max_distance = -1
			, bool use_parallel = true
		);

		/// <summary> Cast a single occlusion ray. </summary>
		/// <param name="x"> x component of the ray's origin. </param>
		/// <param name="y"> y component of the ray's origin. </param>
		/// <param name="z"> z component of the ray's origin. </param>
		/// <param name="dx"> x component of the ray's direction. </param>
		/// <param name="dy"> y component of the ray's direction. </param>
		/// <param name="dz"> z component of the ray's direction. </param>
		/// <param name="distance">
		/// Maximum distance of the ray. Any hits beyond this distance will not be counted. If
		/// negative, count all hits regardless of distance.
		/// </param>
		/// <param name="mesh_id">
		/// (NOT IMPLEMENTED) The id of the only mesh for this ray to collide with. -1 for all.
		/// </param>
		/// <returns> True if the ray intersected any geometry. False otherwise. </returns>
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Fire a ray straight down
				bool res = ert.FireOcclusionRay(0, 0, 1, 0, 0, -1);

				// Print Results
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;

				// Fire a ray straight up
				res = ert.FireOcclusionRay(0, 0, 1, 0, 0, 1);

				// Print results
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;
			\endcode

			`>>> True`\n
			`>>> False`
		*/
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

		/// <summary> Add a new mesh to this raytracer's BVH with the specified ID. </summary>
		/// <param name="Mesh"> A vector of 3d points composing the mesh </param>
		/// <param name="ID"> the id of the mesh </param>
		/// <param name="Commit">
		/// Whether or not to commit changes yet. This is slow, so only do this when you're done
		/// adding meshes.
		/// </param>
		/// <returns>
		/// True if the mesh was added successfully, false if the addition failed or the ID was
		/// already taken.
		/// </returns>
		/// <exception cref="std::exception">
		/// Embree failed to allocate vertex and or index buffers.
		/// </exception>

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// Create a container of coordinates
				std::vector<std::array<float, 3>> directions = {
					{0, 0, 1},
					{0, 1, 0},
					{1, 0, 0},
					{-1, 0, 0},
					{0, -1, 0},
					{0, 0, -1},
				};

				// Create the EmbreeRayTracer
				auto ert = HF::RayTracer::EmbreeRayTracer(directions);

				// Prepare the mesh ID
				const int id = 214;

				// Insert the mesh, Commit parameter defaults to false
				bool status = ert.InsertNewMesh(directions, id);

				// Retrieve status
				std::string result = status ? "status okay" : "status not okay";
				std::cout << result << std::endl;
			\endcode

			`>>>status okay`\n
		*/
		bool InsertNewMesh(std::vector<std::array<float, 3>>& Mesh, int ID, bool Commit = false);

		/// <summary>
		/// Add a new mesh to the BVH with the specified ID. If False, then the addition
		/// failed, or the ID was already taken.
		/// </summary>
		/// <param name="Mesh"> A vector of 3d points composing the mesh </param>
		/// <param name="ID"> the id of the mesh </param>
		/// <param name="Commit">
		/// Whether or not to commit changes yet. This is slow, so only do this when you're done
		/// adding meshes.
		/// </param>
		/// <returns> True if successful </returns>
		/// <exception cref="std::exception">
		/// RTC failed to allocate vertex and or index buffers.
		/// </exception>

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// Create a container of coordinates
				std::vector<std::array<float, 3>> directions = {
					{0, 0, 1},
					{0, 1, 0},
					{1, 0, 0}
				};


				// Create the EmbreeRayTracer
				auto ert = HF::RayTracer::EmbreeRayTracer(directions);

				// Prepare coordinates to create a mesh
				std::vector<std::array<float, 3>> mesh_coords = { 
					{-1, 0, 0},
					{0, -1, 0},
					{0, 0, -1} 
				};

				// Create a mesh
				const int id = 325;
				const std::string mesh_name = "my mesh";
				HF::Geometry::MeshInfo mesh(mesh_coords, id, mesh_name);

				// Determine if mesh insertion successful
				if (ert.InsertNewMesh(mesh, false)) {
					std::cout << "Mesh insertion okay" << std::endl;
				}
				else {
					std::cout << "Mesh insertion error" << std::endl;
				}
			\endcode

			`>>>Mesh insertion okay`\n
		*/
		bool EmbreeRayTracer::InsertNewMesh(HF::Geometry::MeshInfo& Mesh, bool Commit);

		/// <summary> Add several new meshes to the BVH. </summary>
		/// <param name="Meshes"> A vector of meshinfo to each be added as a seperate mesh. </param>
		/// <param name="Commit">
		/// Whether or not to commit changes to the scene after all meshes in Meshes have been added.
		/// </param>
		/// <returns> True. </returns>

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// For brevity
				using HF::Geometry::MeshInfo;
				using HF::RayTracer::EmbreeRayTracer;

				// Prepare the obj file path
				std::string teapot_path = "teapot.obj";
				std::vector<MeshInfo> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

				// Create the EmbreeRayTracer
				auto ert = EmbreeRayTracer(geom);

				// Prepare coordinates to create a mesh
				std::vector<std::array<float, 3>> mesh_coords_0 = {
					{0, 0, 1},
					{0, 1, 0},
					{1, 0, 0}
				};

				std::vector<std::array<float, 3>> mesh_coords_1 = {
					{-1, 0, 0},
					{0, -1, 0},
					{0, 0, -1}
				};

				// Prepare mesh IDs and names
				const int mesh_id_0 = 241;
				const int mesh_id_1 = 363;
				const std::string mesh_name_0 = "this mesh";
				const std::string mesh_name_1 = "that mesh";

				// Create each MeshInfo
				MeshInfo mesh_0(mesh_coords_0, mesh_id_0, mesh_name_0);
				MeshInfo mesh_1(mesh_coords_1, mesh_id_1, mesh_name_1);

				// Create a container of MeshInfo
				std::vector<MeshInfo> mesh_vec = { mesh_0, mesh_1 };

				// Determine if mesh insertion successful
				if (ert.InsertNewMesh(mesh_vec, false)) {
					std::cout << "Mesh insertion okay" << std::endl;
				}
				else {
					std::cout << "Mesh insertion error" << std::endl;
				}
			\endcode
		*/
		bool InsertNewMesh(std::vector<HF::Geometry::MeshInfo>& Meshes, bool Commit = true);

		/// <summary>
		/// Template for firing rays using array-like containers for the direction and origin.
		/// </summary>
		/// <param name="node"> A point in space. Must atleast have [0], [1], and [2] defined </param>
		/// <param name="direction">
		/// Direction to fire the ray in. Same constraints as node, but can be a different type
		/// </param>
		/// <param name="out_distance"> distance from the ray to the hit (if any) </param>
		/// <param name="out_meshid"> ID of the mesh hit(if any) </param>
		/// <param name="max_distance">
		/// Maximum distance the ray can travel. And intersections beyond this distance will be
		/// ignored. Will consider all intersections regardless of distance if set to -1.
		/// </param>
		/// <returns> True if the ray intersected any geometry, false otherwise. </returns>
		/// <remarks>
		/// This is preferrable to use over the other ray functions in many circumstances since
		/// the use of templates ensures no unnecessary conversions are performed.
		/// </remarks>
		/// \note C++ 2020's Concepts would be a good way to explain how to use this whenever
		/// they get implemented.
		/*!
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create a new raytracer from a basic 10x10 plane centered on the origin.
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Create origin/direction arrays
				std::array<float, 3> origin{ 0, 0, 1 };
				std::array<float, 3> direction{ 0, 0, -1 };

				bool res = false; float out_dist = -1; int out_id = -1;

				// Fire a ray straight down
				res = ert.FireAnyRay(origin, direction, out_dist, out_id);

				// Print its distance if it connected
				if (res) std::cerr << out_dist << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Fire a ray straight up
				res = ert.FireAnyRay(origin, origin, out_dist, out_id);

				// Print its distance if it connected
				if (res) std::cerr << out_dist << std::endl;
				else std::cerr << "Miss" << std::endl;
			\endcode

			`>>>(0, 0, 0)`\n
			`>>>Miss`

		*/
		template <typename N, typename V>
		bool FireAnyRay(
			const N& node,
			const V& direction,
			float& out_distance,
			int& out_meshid,
			float max_distance = -1.0f)
		{
			HitStruct result;
			
			// Use custom triangle intesection if required
			if (use_precise)
				result = FirePreciseRay(
					node[0], node[1], node[2],
					direction[0], direction[1], direction[2], max_distance, -1
				);
			else
				result = Intersect(
					node[0], node[1], node[2],
					direction[0], direction[1], direction[2], max_distance, -1
				);

			// If no intersection was found, return false
			if (!result.DidHit()) return false;
			else {
				// otherwise update outputs then return true
				out_distance = result.distance;
				out_meshid = result.meshid;
				return true;
			}
		}

		template <typename N, typename V>
		inline std::vector<HitStruct> FireAnyRayParallel(
			const N & nodes,
			const V & directions,
			float max_distance = -1.0f,
			bool force_precise = false)
		{
			const int n= nodes.size();
			std::vector<HitStruct> results (nodes.size());

			#pragma omp parallel for schedule(dynamic)
			for (int i = 0; i < n; i++) {// Use custom triangle intesection if required
				const auto& node = nodes[i];
				const auto& direction = directions[i];
				if (force_precise) {
					results[i] = FirePreciseRay(
						node[0], node[1], node[2],
						direction[0], direction[1], direction[2], max_distance, -1
					);
				}

				else
					results[i] = Intersect(
						node[0], node[1], node[2],
						direction[0], direction[1], direction[2], max_distance, -1
					);
			}
			return results;
		}


		/// <summary>
		/// Template for firing rays using array-like containers for the direction and origin.
		/// Similar to <see cref="FireAnyRay" />.
		/// </summary>
		/// <param name="node"> A point in space. Must atleast have [0], [1], and [2] defined. </param>
		/// <param name="direction">
		/// Direction to fire the ray in. Same constraints as node, but can be a different type.
		/// </param>
		/// <param name="max_distance"> Maximum distance the ray can travel. </param>
		/// <returns> true if the ray connected with anything, false otherwise. </returns>
		/// <remarks>
		/// Like the other occlusion functions, this is much faster than its counterparts at the
		/// cost of only being able to return true if the ray intersects any geometry, and false
		/// if it doesn't.
		/// </remarks>
		/// \note C++ 2020's Concepts would be a good way to explain how to use this whenever
		/// they get implemented.
		/*!	
			\par Example
			\code
				// Requires #include "embree_raytracer.h", #include "meshinfo.h"

				// for brevity
				using HF::RayTracer::EmbreeRayTracer;
				using HF::Geometry::MeshInfo;

				// Create Plane
				const vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

				// Fire a ray straight down
				bool res = ert.FireAnyOcclusionRay(
					std::array<float, 3>{0, 0, 1},
					std::array<float, 3>{0, 0, -1}
				);

				// Print Results
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;

				// Fire a ray straight up
				res = ert.FireAnyOcclusionRay(
					std::array<float, 3>{0, 0, 1},
					std::array<float, 3>{0, 0, 1}
				);

				// Print Results
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;
			\endcode
		
			`>>> True`\n
			`>>> False`
		*/
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

		/// <summary> Increment reference counters to prevent destruction when a copy is made. <summary>
		/// <param name="ERT2">Reference to EmbreeRayTracer, the right-hand side of the = statement</param>

		/*!
			\code
				// Requires #include "embree_raytracer.h"

				// Create a container of coordinates
				std::vector<std::array<float, 3>> directions = {
					{0, 0, 1},
					{0, 1, 0},
					{1, 0, 0},
					{-1, 0, 0},
					{0, -1, 0},
					{0, 0, -1},
				};

				// Create the EmbreeRayTracer
				HF::RayTracer::EmbreeRayTracer ert_0(directions);

				// Create an EmbreeRayTracer, no arguments
				HF::RayTracer::EmbreeRayTracer ert_1;

				// If and when ert_0 goes out of scope,
				// data within ert_0 will be retained inside of ert_1.
				ert_1 = ert_0;
			\endcode
		*/
		void operator=(const EmbreeRayTracer& ERT2);

		/// <summary> Custom destructor to ensure cleanup of embree resources. </summary>

		/*!
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// For brevity
				using HF::Geometry::MeshInfo;
				using HF::RayTracer::EmbreeRayTracer;

				// Prepare the obj file path
				std::string teapot_path = "teapot.obj";
				std::vector<MeshInfo> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

				// Begin scope
				{
					// Create the EmbreeRayTracer
					auto ert = EmbreeRayTracer(geom);

					// Use ert within this scope
				}
				// end scope - destructor called at end of scope
			\endcode
		*/
		~EmbreeRayTracer();
	};
}
#endif
