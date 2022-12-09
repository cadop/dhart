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
#include <HitStruct.h>
#define _USE_MATH_DEFINES

namespace HF::Geometry {
	template <typename T> class MeshInfo;
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

		inline Vector3D::Vector3D(double x, double y, double z) {
			this->x = x;
			this->y = y;
			this->z = z;

		}
		inline Vector3D operator-(const Vector3D& v2) const {
			return Vector3D{
				x - v2.x,
				y - v2.y,
				z - v2.z
			};
		}
		// Scalar multiplication overload 
		inline Vector3D operator*(const double&a) const {
			return Vector3D{
				a * x,
				a * y,
				a * z
			};
		}
	};

	//*! \brief Determine whether this mesh did or did not intersect */
	bool DidIntersect(int mesh_id);

	struct RayRequest;
	struct Vertex;
	struct Triangle;

	/// <summary> A wrapper for Intel's Embree Library. </summary>
	/// <remarks>
	/// Provides several functions to quickly and simply perform ray intersections using Embree.
	/// This class will automatically dispose of Embree buffers when destroyed, incrementing
	/// Embree's reference counters on copy, and decrementing them on deletion. This class also
	/// provides methods for adding geometry to Embree's geometry buffers.
	/// </remarks>
	/// \todo Many functions support MeshID but don't actually check for it. 
	class EmbreeRayTracer {
		/// All objects in Embree are created from this. https://www.embree.org/api.html#device-object
		RTCDevice device; 
		/// Container for a set of geometries, and the BVH. https://www.embree.org/api.html#scene-object
		RTCScene scene;
		/// Context to cast rays within.
		RTCIntersectContext context;
		/// Triangle buffer. Is used in multiple places, but contents are dumped.
		Triangle* triangles;
		/// Vertex buffer. Is used in multiple places but contents are dumped.
		Vertex* Vertices;

		bool use_precise = false; ///< If true, use custom triangle intersection intersection instead of embree's

		std::vector<RTCGeometry> geometry; //> A list of the geometry being used by RTCScene.

	private:
		/*! \brief Performs all the necessary operations to set up the scene.

			\details
			1) Creates device
			2) Creates the scene using device
			3) Sets the build quality of the scene
			4) Sets scene flags
			5) Inits Intersect_IMPL context


			\remarks
			Any changes to the internal settings of embree should be handled here. I.E. Enforcing
			That all bvh's used be of robust quality, assigning a custom context, etc.

		*/
		void SetupScene();

		/*! 
			\brief Get the vertices for a specific triangle in a mesh.
		
			\param geomID ID of the geometry the triangle belongs to
			\param primID Id of the triangle to retrieve

			\returns The 3 vertices that comprise the triangle with ID `primID` in the
					 geometry with id `geomID`

			\pre 1) `geomID` must be the ID of geometry that already exists within this raytracer's BVH
			\pre 2) `primID` must be the ID of a triangle within the bounds of the geometry at `geomID`'s buffers
		
		*/
		std::array<Vector3D, 3> GetTriangle(unsigned int geomID, unsigned int primID) const;

		/*!\brief Attach geometry to the current scene.
			
			\param geom Geometry to attach
			\param id Id to attempt to attach with. If this ID is already taken, the next available
					  ID will be assigned, and the new ID will be returned

			\returns the New ID if a new ID needed to be given to the mesh or the ID given by ID.
		
		*/
		int InsertGeom(RTCGeometry& geom, int id = -1);

		/*! \brief Calculate the distance from origin to the point of intersection using an algorithm with higher precision
			
			\param geom_id ID of the geometry the ray intersected
			\param prim_id ID of the primitive in geometry the ray intersected
			\param origin The origin point of the ray
			\param direction the direction the ray was casted in

			\returns The distance between origin and the triangle it intersected

			\details
			Uses `geom_id` to get the buffers of the intersected geometry, then uses `prim_id` to get the 3 vertices
			comprising the intersected triangle. Once these are obtained, this function calls RayTriangleIntersection
			to calculate the precise point of intersection and returns the result. When use_precise is set to true, 
			Intersect will call this function to calculate its distance value instead of returning the distance calculated
			by embree.

			\remarks
			This algorithm was implemented due to the relatively low precision of the distances returned
			from Embree which causes issues with certain analysis methods. 

			\see GetTriangle for details on getting the geometry and triangle from embree 
			\see RayTriangleIntersection for details on the intersection algorithm itself
		*/
	double CalculatePreciseDistance(
			unsigned int geom_id,
			unsigned int prim_id,
			const Vector3D& origin,
			const Vector3D& direction) const;

		/// <summary> Implementation for fundamental ray intersection. </summary>
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
				using HF::Geometry::MeshInfo<float>;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

				HitStruct res;

				// Cast a ray straight down
				res = ert.Intersect_IMPL(0, 0, 1, 0, 0, -1);

				// Print distance if it connected
				if (res.DidHit()) std::cerr << res.distance << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Cast a ray straight up
				res = ert.Intersect_IMPL(0, 0, 1, 0, 0, 1);

				//Print distance if it connected
				if (res.DidHit()) std::cerr << res.distance << std::endl;
				else std::cerr << "Miss" << std::endl;
			\endcode

			`>>>1`\n
			`>>>Miss`
		 */
		RTCRayHit Intersect_IMPL(
			float x,
			float y,
			float z,
			float dx,
			float dy,
			float dz,
			float max_distance = -1,
			int mesh_id = -1
		);

		/// <summary> Implementation for fundamental occlusion ray intersection. </summary>
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
				using HF::Geometry::MeshInfo<float>;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

				// Cast a ray straight down
				bool res = ert.Occluded_IMPL(0, 0, 1, 0, 0, -1);

				// Print Results
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;

				// Cast a ray straight up
				res = ert.Occluded_IMPL(0, 0, 1, 0, 0, 1);

				// Print results
				if (res) std::cerr << "True" << std::endl;
				else std::cerr << "False" << std::endl;
			\endcode

			`>>> True`\n
			`>>> False`
		*/
		bool Occluded_IMPL(
			float x,
			float y,
			float z,
			float dx,
			float dy,
			float dz,
			float distance = -1,
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
		using HF::Geometry::MeshInfo<float>;

		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};

		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create RayTracer
		EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

		// Cast a ray straight down
		bool res = ert.Occluded_IMPL(
			std::array<float, 3>{ 0, 0, 1 },
			std::array<float, 3>{ 0, 0, -1 }
		);

		// Print Result
		if (res) std::cerr << "True" << std::endl;
		else std::cerr << "False" << std::endl;

		// Cast a ray straight up
		res = ert.Occluded_IMPL(
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
		bool Occluded_IMPL(
			const std::array<float, 3>& origin,
			const std::array<float, 3>& direction,
			float max_dist = -1
		);

		/*!
			\brief Create a new instance of RTCGeometry from a triangle and vertex buffer

			\param tris Triangle buffer to construct new geometry with
			\param verts Vertex buffer to construct geometry with

			\returns Committed Geometry containing the specified triangles and vertices.

		*/
		RTCGeometry ConstructGeometryFromBuffers(std::vector<Triangle>& tris, std::vector<Vertex>& verts);

	public:
		/*!
			\brief Construct an empty EmbreeRayTracer;
		
			\param use_precise If set to true, use a more precise intesection algorithm to determine
				   the distance between rays origin points and their points of intesection
			\code
				// Requires #include "embree_raytracer.h", #include "objloader.h"

				// For brevity
				using HF::RayTracer::EmbreeRayTracer;

				// Create the EmbreeRayTracer, no arguments
				EmbreeRayTracer ert;
			\endcode
		*/
		EmbreeRayTracer(bool use_precise = false);

		/// <summary> Create a new EmbreeRayTracer and add a single mesh to the scene. </summary>
		/// <param name="MI"> The mesh to use for scene construction. </param>
		/// <param name="use_precise">If set to true, use a more precise intesection algorithm to determine
		///	the distance between rays origin pointsand their points of intesection </param>
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
				using HF::Geometry::MeshInfo<float>;
				using HF::RayTracer::EmbreeRayTracer;

				// Prepare the obj file path
				std::string teapot_path = "teapot.obj";
				std::vector<MeshInfo<float>> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

				// Create the EmbreeRayTracer
				auto ert = EmbreeRayTracer(geom);
			\endcode
		*/
		EmbreeRayTracer(std::vector<HF::Geometry::MeshInfo<float>>& MI, bool use_precise_intersection = false);

		/*! 
			\brief Construct the raytracer using only a single mesh.
			
			\param MI MeshInfo<float> instance to create the BVH with.
			\param use_precise If set to true, use a more precise intesection algorithm to determine
							   the distance between rays origin points and their points of intesection
		*/
		EmbreeRayTracer(HF::Geometry::MeshInfo<float>& MI, bool use_precise = false);


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
		bool status = ert.AddMesh(directions, id);

		// Retrieve status
		std::string result = status ? "status okay" : "status not okay";
		std::cout << result << std::endl;
	\endcode

	`>>>status okay`\n
*/
		bool AddMesh(std::vector<std::array<float, 3>>& Mesh, int ID, bool Commit = false);

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
				HF::Geometry::MeshInfo<float> mesh(mesh_coords, id, mesh_name);

				// Determine if mesh insertion successful
				if (ert.AddMesh(mesh, false)) {
					std::cout << "Mesh insertion okay" << std::endl;
				}
				else {
					std::cout << "Mesh insertion error" << std::endl;
				}
			\endcode

			`>>>Mesh insertion okay`\n
		*/
		bool AddMesh(HF::Geometry::MeshInfo<float>& Mesh, bool Commit);

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
		using HF::Geometry::MeshInfo<float>;
		using HF::RayTracer::EmbreeRayTracer;

		// Prepare the obj file path
		std::string teapot_path = "teapot.obj";
		std::vector<MeshInfo<float>> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

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

		// Create each MeshInfo<float>
		MeshInfo<float> mesh_0(mesh_coords_0, mesh_id_0, mesh_name_0);
		MeshInfo<float> mesh_1(mesh_coords_1, mesh_id_1, mesh_name_1);

		// Create a container of MeshInfo<float>
		std::vector<MeshInfo<float>> mesh_vec = { mesh_0, mesh_1 };

		// Determine if mesh insertion successful
		if (ert.AddMesh(mesh_vec, false)) {
			std::cout << "Mesh insertion okay" << std::endl;
		}
		else {
			std::cout << "Mesh insertion error" << std::endl;
		}
	\endcode
*/
		bool AddMesh(std::vector<HF::Geometry::MeshInfo<float>>& Meshes, bool Commit = true);

		/// <summary>
		/// Cast a ray and overwrite the origin with the hitpoint if it intersects any geometry.
		/// </summary>
		/// <param name="dir"> Direction to cast the ray in. </param>
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
				using HF::Geometry::MeshInfo<float>;

				// Create Plane
				const std::vector<float> plane_vertices{
							-10.0f, 10.0f, 0.0f,
							-10.0f, -10.0f, 0.0f,
							10.0f, 10.0f, 0.0f,
							10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

				// Cast a ray straight down
				std::array<float, 3> origin{ 0,0,1 };
				bool res = ert.PointIntersection(
					origin,
					std::array<float, 3>{0, 0, -1}
				);

				// Print Results
				if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Cast a ray straight up
				origin = std::array<float, 3>{ 0, 0, 1 };
				res = ert.PointIntersection(
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
		bool PointIntersection(
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
				using HF::Geometry::MeshInfo<float>;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

				bool res;

				// Cast a ray straight down directly at the plane
				float x = 0; float y = 0; float z = 1;
				res = ert.PointIntersection(x, y, z, 0, 0, -1);

				// Print output
				if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;

				// Cast a ray straight up
				x = 0; y = 0; z = 1;
				res = ert.PointIntersection(x, y, z, 0, 0, 1);

				// Print output
				if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
				else std::cerr << "Miss" << std::endl;
			\endcode

			`>>>(0, 0, 0)`\n
			`>>>Miss`
		*/
		bool PointIntersection(
			float& x,
			float& y,
			float& z,
			float dx,
			float dy,
			float dz,
			float distance = -1,
			int mesh_id = -1
		);

		/// <summary> Cast multiple rays and recieve hitpoints in return. </summary>
		/// <param name="origins"> An array of x,y,z coordinates to cast rays from. </param>
		/// <param name="directions"> An array of x,y,z directions to cast in. </param>
		/// <param name="use_parallel">
		/// Cast rays in parallel if true, if not cast in serial. All available cores will be used.
		/// </param>
		/// <param name="max_distance">
		/// Maximum distance the ray can travel. Any intersections beyond this distance will be
		/// ignored. If set to 1, all intersections will be counted regardless of distance.
		/// </param>
		/// <param name="mesh_id"> (UNUSED) Only intersect with the mesh of this ID </param>
		/// <returns>
		/// A vector of <see cref="Vector3D" /> for the hitpoint of each ray cast. If a ray
		/// didn't hit, its point will be invalid, checkable using <see
		/// cref="Vector3D.IsValid()" />.
		/// </returns>
		/// <remarks>
		/// <para> Can be cast in 3 configurations: </para>
		/// <list type="bullet">
		/// <item>
		/// Equal amount of directions/origins: Cast a ray for every pair of origin/direction in
		/// order. i.e. (origin[0], direction[0]), (origin[1], direction[1])
		/// </item>
		/// <item>
		/// One direction, multiple origins: Cast a ray in the given direction from each origin
		/// point in origins.
		/// </item>
		/// <item>
		/// One origin, multiple directions: Cast a ray from the origin point in each direction
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
				using HF::Geometry::MeshInfo<float>;

				// Create plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

				// Create an array of directions all containing {0,0,-1}
				std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{ 0, 0, -1 });

				// Create an array of origin points moving further to the left with each point
				std::vector<std::array<float, 3>> origins(10);
				for (int i = 0; i < 10; i++) origins[i] = std::array<float, 3>{static_cast<float>(2 * i), 0, 1};

				// Cast every ray.
				auto results = ert.PointIntersections(origins, directions);

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
		std::vector<char> PointIntersections(
			std::vector<std::array<float, 3>>& origins,
			std::vector<std::array<float, 3>>& directions,
			bool use_parallel = true,
			float max_distance = -1,
			int mesh_id = -1
		);



		/// <summary> Cast multiple occlusion rays in parallel. </summary>
		/// <param name="origins">
		/// A list of origins. If only one is supplied then it will be cast for every direction
		/// in directions.
		/// </param>
		/// <param name="directions">
		/// A list of directions. If only one is supplied then it will be cast for every origin
		/// in origins
		/// </param>
		/// <param name="max_distance"> Maximum distance the ray can travel </param>
		/// <param name="parallel"> Whether or not to cast the rays in parallel. </param>
		/// <returns>
		/// An ordered array of bools where every true indicates a hit and every false indicates
		/// a miss.
		/// </returns>
		/// <remarks>
		/// <para> Can be cast in 3 configurations: </para>
		/// <list type="bullet">
		/// <item>
		/// Equal amount of directions/origins: Cast a ray for every pair of origin/direction in
		/// order. i.e. (origin[0], direction[0]), (origin[1], direction[1])
		/// </item>
		/// <item>
		/// One direction, multiple origins: Cast a ray in the given direction from each origin
		/// point in origins.
		/// </item>
		/// <item>
		/// One origin, multiple directions: Cast a ray from the origin point in each direction
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
				using HF::Geometry::MeshInfo<float>;

				// Create Plane
				const std::vector<float> plane_vertices{
					-10.0f, 10.0f, 0.0f,
					-10.0f, -10.0f, 0.0f,
					10.0f, 10.0f, 0.0f,
					10.0f, -10.0f, 0.0f,
				};

				const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

				// Create RayTracer
				EmbreeRayTracer ert(std::vector<MeshInfo<float>>{MeshInfo<float>(plane_vertices, plane_indices, 0, " ")});

				// Create an array of directions all containing {0,0,-1}
				std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{0, 0, -1});

				// Create an array of origins with the first 5 values being above the plane and
				// the last five values being under it.
				std::vector<std::array<float, 3>> origins(10);
				for (int i = 0; i < 5; i++) origins[i] = std::array<float, 3>{ 0.0f, 0.0f, 1.0f };
				for (int i = 5; i < 10; i++) origins[i] = std::array<float, 3>{ 0.0f, 0.0f, -1.0f };

				// Cast every occlusion ray
				std::vector<char> results = ert.Occlusions(origins, directions);

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
		std::vector<char> Occlusions(
			const std::vector<std::array<float, 3>>& origins,
			const std::vector<std::array<float, 3>>& directions,
			float max_distance = -1
			, bool use_parallel = true
		);


		/*! \brief Cast a ray from origin in direction. 
		
			\tparam return_type Numeric type for the returned distance value i.e. double, long double, float, etc.
			\tparam N X,Y,Z coordinates representing a point in space
			\tparam V X,Y,Z Coordinates representing direction vector

			\param node A point in space. Must atleast have [0], [1], and [2] defined.
			\param direction Direction to cast the ray in. Same constraints as node, but can be a different type.
			\param max_distance Maximum distance a ray can travel before intersections are ignored. Set to -1
								for infinite distance.
			\param mesh_id Only consider intersections with the mesh at this ID. Set to -1 to consider
							intersections with all meshes. 

			\returns The distance from origin to the ray's point of intersection. If no intersection was found
					 the returned hitstruct's meshID will be -1. This is checkable with the hitstruct's DidHit()
					 function.

			\remarks
			If `use_precise` is set to true then this will use a more precise algorithm to calculate returned 
			distance value.  This is one of the most basic forms of intersection for the raytracer and many
			other functions will call this internally. 

			\see Intersections for a parallel version of this function
		*/
		template <typename return_type = double, class N, class V>
		HitStruct<return_type> Intersect(
			const N& node,
			const V& direction,
			float max_distance = -1.0f, int mesh_id = -0.1f)
		{
			return Intersect<return_type>(node[0], node[1], node[2], direction[0], direction[1], direction[2], max_distance, mesh_id);
		}

		/*! \brief Cast a ray from origin in direction.
		
			\tparam return_type Numeric type used for the output distance value.
			\tparam numeric1 Numeric type used for the x,y,z components of the origin
			\tparam numeric2 Numeric type used for the x,y,z components of the direction
			
			\param x X component of the ray's origin.
			\param y Y component of the ray's origin.
			\param z Z component of the ray's origin.
			\param dx X component of the ray's direction.
			\param dy Y component of the ray's direction.
			\param dz Z component of the ray's direction.
			\param max_distance Maximum distance a ray can travel before intersections are ignored. Set to -1
								for infinite distance.
			\param mesh_id Ignore intersections with any mesh other than the mesh with this ID. set to -1 to
					        consider intersections with any geometry

			\returns The distance from origin to the ray's point of intersection. If no intersection was found
					 the returned hitstruct's meshID will be -1. This is checkable with the hitstruct's DidHit()
					 function.

			\remarks
			If `use_precise` is set to true then this will use a more precise algorithm to calculate returned
			distance value.This is one of the most basic forms of intersection for the raytracerand many
			other functions will call this internally.
			
			\see Intersections for a parallel version of this function
			
			\par Example
			\snippet tests\src\embree_raytracer.cpp EX_Intersect
		*/
		template <typename return_type = double, typename numeric1 = double, typename numeric2 = double>
		HitStruct<return_type> Intersect(
				numeric1 x, numeric1 y, numeric1 z,
				numeric2 dx, numeric2 dy, numeric2 dz,
				float distance = -1.0f, int mesh_id = -1)
		{
			// create output value
			HitStruct<return_type> out_struct;

			// Cast the ray
			auto result = Intersect_IMPL(
				x,y,z,
				dx,dy,dz, distance, mesh_id
			);

			// If an intersection occured, update the struct. 
			if (DidIntersect(result.hit.geomID))
			{
				// Use a precise ray intersection if required
				if (!(this->use_precise))
					out_struct.distance = result.ray.tfar;
				else
					out_struct.distance = CalculatePreciseDistance(
						result.hit.geomID,
						result.hit.primID,
						Vector3D(x,y,z),
						Vector3D(dx,dy,dz)
					);
				out_struct.meshid = result.hit.geomID;
			}

			return out_struct;
		}


		/*! \brief Cast a ray from origin in direction and update the parameters instead of returning a hitstruct.

			\tparam return_type Numeric type for the returned distance value i.e. double, long double, float, etc.
			\tparam N X,Y,Z coordinates representing a point in space
			\tparam V X,Y,Z Coordinates representing direction vector

			\param node Origin point of the ray.
			\param direction Direction to cast the ray in. 
			\param max_distance Maximum distance a ray can travel before intersections are ignored. Set to -1
								for infinite distance.
			\param out_distance On intersection will be updated to contain the distance from origin to the point
								of intersection.

			\param out_meshid updated to contain the ID of the intersected mesh.
			
			\returns true if the ray did intersect any geometry and the outputs were updated, false otherwise.		

			\remarks
			It's suggested to use Intersect instead of this function. 

			\see Intersections for a parallel version of this function
		*/
		template <typename N, typename V, typename return_type>
		bool IntersectOutputArguments(
			const N& node,
			const V& direction,
			return_type& out_distance,
			int& out_meshid,
			float max_distance = -1.0f)
		{
			HitStruct<return_type> result = Intersect<return_type>(node, direction, max_distance);
			if (result.DidHit()) {
				out_distance = result.distance;
				out_meshid = result.meshid;
				return true;
			}
			else
				return false;
		}
		
		/*! \brief Cast multiple rays in parallel.
		
			\tparam return_type Numeric type for the returned distance value i.e. double, long double, float, etc.
			\tparam N X,Y,Z A container of objects holding x,y,z coordinates for the origin points of every ray
			\tparam V X,Y,Z A container of objects holding x,y,z coordinates for the direction points of every ray

			\param node Origin points to cast rays from.
			\param direction Directions to cast rays in.
			\param max_distance Maximum distance a ray can travel before intersections are ignored. Set to -1
								for infinite distance.
			\param mesh_id Only consider intersections with the mesh at this ID. Set to -1 to consider
							intersections with all meshes.

			\returns An ordered array of results from casting a ray for every origin in origins in the direciton
			in directions with a matching index. Rays that do not result in an intersection will have hitstructs
			with meshids of -1. This can be checked by calling the HitStruct's .DidHit function.  

			\remarks
			If `use_precise` is set to true then this will use a more precise algorithm to calculate returned
			distance value.  This is one of the most basic forms of intersection for the raytracer and many
			other functions will call this internally.

			\pre The length of nodes must match the length of directions.

		*/
		template <typename return_type, typename N, typename V>
		inline std::vector<HitStruct<return_type>> Intersections(
			const N & nodes,
			const V & directions,
			float max_distance = -1.0f,
			const bool use_parallel = false)
		{
			const int n = nodes.size();

			std::vector<HitStruct<return_type>> results (nodes.size());

			#pragma omp parallel for schedule(dynamic, 256) if (use_parallel)
			for (int i = 0; i < n; i++) {// Use custom triangle intesection if required
				const auto& node = nodes[i];
				const auto& direction = directions[i];
				results[i] = Intersect<return_type>(node, direction);
			}
			return results;
		}

		/*!
			\brief Determine if there is an intersection with any geometry 
			
			\tparam N X,Y,Z coordinates representing a point in space
			\tparam V X,Y,Z Coordinates representing direction vector

			\param node A point in space. Must atleast have [0], [1], and [2] defined.
			\param direction Direction to cast the ray in. Same constraints as node, but can be a different type.
			\param max_distance Maximum distance a ray can travel before intersections are ignored. Set to -1 
								for infinite distance.

			\returns `true` if the ray intersected with any geometry within max_distance, `false` otherwise
		
			\remarks
			Occlusion rays are much faster than other intersection functions however they are only able
			tell whether they intersected with anything or not. These rays are very useful for quick line
			of sight checks utilizing the `max_distance` parameter.

			\par Example

			\snippet tests\src\embree_raytracer.cpp EX_Occluded_Array
		
			`>>> True`\n
			`>>> False`
		*/
		template <typename N, typename V>
		bool Occluded(
			const N& origin,
			const V& direction,
			float max_distance = -1.0f,
			int mesh_id = -1
		) {
			return Occluded_IMPL(
				origin[0], origin[1], origin[2],
				direction[0], direction[1], direction[2],
				max_distance, mesh_id
			);
		}


		/*!
			\brief Determine if there is an intersection with any geometry

			\tparam numeric1 Numeric type used for the x,y,z components of the origin
			\tparam numeric2 Numeric type used for the x,y,z components of the direction
			\tparam dist_type Numeric type used for the distance parameter. 

			\param x X component of the ray's origin.
			\param y Y component of the ray's origin.
			\param z Z component of the ray's origin.
			\param dx X component of the ray's direction.
			\param dy Y component of the ray's direction.
			\param dz Z component of the ray's direction.
			\param max_distance Maximum distance a ray can travel before intersections are ignored. Set to -1
								for infinite distance.
			\param mesh_id Ignore intersections with any mesh other than the mesh with this ID. set to -1 to
					        consider intersections with any geometry

			\returns `true` if the ray intersected with any geometry within max_distance, `false` otherwise

			\remarks
			Occlusion rays are much faster than other intersection functions however they are only able
			tell whether they intersected with anything or not. These rays are very useful for quick line
			of sight checks utilizing the `max_distance` parameter.

			\par Example

			\snippet tests\src\embree_raytracer.cpp EX_Occluded

			`>>> True`\n
			`>>> False`
		*/
		template <typename numeric1, typename numeric2, typename dist_type = float>
		bool Occluded(
			numeric1 x, numeric1 y, numeric1 z,
			numeric2 dx, numeric2 dy, numeric2 dz,
			dist_type max_distance = -1.0,
			int mesh_id = -1
		) {
			return Occluded_IMPL(
				x,y,z,
				dx,dy,dz,
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
				using HF::Geometry::MeshInfo<float>;
				using HF::RayTracer::EmbreeRayTracer;

				// Prepare the obj file path
				std::string teapot_path = "teapot.obj";
				std::vector<MeshInfo<float>> geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

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

	/*!
	\brief Determine the distance between a ray's origin and it's point of intersection with a triangle.

	\param origin Origin point of the ray.
	\param direction Direction the ray was casted in
	\param v1 First vertex of the triangle
	\param v2 Second vertex of a triangle
	\param v3 Third vertex of a triangle


	\returns The distance between the ray's origin point and the point of intersection with the triangle defined
			 by v1, v2, and v3 OR -1 if there was no intersection between the ray and the triangle.


	\remarks
	This algorithm is based on an implementation of the Möller–Trumbore intersection algorithm written
	on the wikipedia page https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm.

*/
	double RayTriangleIntersection(
		const Vector3D& origin,
		const Vector3D& direction,
		const Vector3D& v1,
		const Vector3D& v2,
		const Vector3D& v3);
}
#endif
