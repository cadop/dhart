///
/// \file		embree_raytracer.cpp
/// \brief		Contains implementation for the <see cref="HF::RayTracer::EmbreeRayTracer">EmbreeRayTracer</see>
///
///	\author		TBA
///	\date		26 Jun 2020

#include <embree_raytracer.h>
#include <corecrt_math_defines.h>
#include <functional>
#include <iostream>
#include <thread>

#include <robin_hood.h>

#include <meshinfo.h>
#include <RayRequest.h>
#include <HFExceptions.h>

namespace HF::RayTracer {

	bool HitStruct::DidHit() const { return meshid != RTC_INVALID_GEOMETRY_ID; };

	/// <summary>
	/// Check an embree device for errors.
	/// </summary>
	/// <param name="device">Device to check for errors</param>
	/// <exception cref="std::exception">Some error is found while casting rays.</exception>
	void CheckState(RTCDevice& device) {
		auto err = rtcGetDeviceError(device);

		if (err != RTCError::RTC_ERROR_NONE)
			throw std::exception("RTC DEVICE ERROR");
	}
	/// <summary>
	/// A vertex. Used internally in Embree.
	/// </summary>
	struct Vertex { float x, y, z; };

	/// <summary>
	/// A triangle. Used internally in Embree.
	/// </summary>
	struct Triangle { int v0, v1, v2; };
	
	/// <summary>
	/// Index a list of verticies and place them into a triangle and vertex buffer.
	/// </summary>
	/// <param name="vertices">Mesh vertices.</param>
	/// <param name="Tribuffer">Output triangle buffer.</param>
	/// <param name="Vbuffer">Output Vertex Buffer.</param>
	/// <remarks>Internally, this uses a hashmap to create an index map of vertices.</remarks>
	/// \todo Performance here can be improved by using indexing.
	inline void vectorsToBuffers(
		const std::vector<std::array<float, 3>>& vertices,
		std::vector<Triangle>& Tribuffer,
		std::vector<Vertex>& Vbuffer
	) {
		robin_hood::unordered_map <std::array<float, 3>, int> index_map;
		
		int next_id = 0;
		int vertsize = vertices.size();

		for (int i = 0; i < vertsize; i += 3) {
			std::array<int, 3> ids;

			//  Get the ids for the next 3 vertices
			for (int k = 0; k < 3; k++)
			{
				auto vert = vertices[i + k];
				int current_id;

				// Get ID from index map, or create a new ID if one doesn't exist
				if (index_map.count(vert) > 0)
					current_id = index_map[vert];
				else {
					index_map[vert] = next_id;
					current_id = next_id;

					// Store new vertex in the vertex buffer
					Vbuffer.emplace_back(Vertex{ vert[0], vert[1], vert[2] });
					next_id++;
				}
				ids[k] = current_id;
			}

			// Store new triangle in the triangle buffer.
			Tribuffer.emplace_back(Triangle{ ids[0], ids[1], ids[2] });
		}
	}

	/// <summary>
	/// Package raw arrays of floats and indices in buffers to the required Embree types.
	/// </summary>
	/// <param name="in_vertices">
	/// An array of floats in which every 3 floats represents a vertex.
	/// </param>
	/// <param name="in_indices">
	/// An array of integers in which every 3 integers represents a triangle.
	/// </param>
	/// <param name="out_vertices"> Output array for vertexes.</param>
	/// <param name="out_triangles"> Output array for triangles.</param>
	/// \todo May be faster as a std::move or std::copy.
	inline void buffersToStructs(
		std::vector<float>& in_vertices,
		std::vector<int>& in_indices,
		std::vector<Vertex>& out_vertices,
		std::vector<Triangle>& out_triangles
	) {
		out_vertices.resize(in_vertices.size() / 3);
		for (int i = 0; i < in_vertices.size(); i += 3) {
			out_vertices[i / 3] = Vertex{
				in_vertices[i],
				in_vertices[i + 1],
				in_vertices[i + 2]
			};
		}

		out_triangles.resize(in_indices.size() / 3);
		for (int i = 0; i < in_indices.size(); i += 3) {
			out_triangles[i / 3] = Triangle{
				in_indices[i],
				in_indices[i + 1],
				in_indices[i + 2]
			};
		}
	}

	EmbreeRayTracer::EmbreeRayTracer(const std::vector<std::array<float, 3>>& geometry) {
		device = rtcNewDevice("start_threads=1,set_affinity=1");
		scene = rtcNewScene(device);
		rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
		rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
		RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
		Triangle* triangles;
		Vertex* Vertices;

		// Set Setup buffers
		std::vector<Triangle> tris;
		std::vector<Vertex> verts;
		vectorsToBuffers(geometry, tris, verts);

		// Cast to triangle/vertex structs so we can operate on them.  This trick is from the embree tutorial
		triangles = (Triangle*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), tris.size() + 1);
		Vertices = (Vertex*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), verts.size() + 1);

		// If either of these turns up null, this can't function
		if (!triangles || !Vertices)
			throw std::exception("RTC DEVICE ERROR");
		
		for (int i = 0; i < tris.size(); i++)
			triangles[i] = tris[i];

		for (int i = 0; i < verts.size(); i++)
			Vertices[i] = verts[i];

		// Add geometry and commit the scene
		rtcCommitGeometry(geom);
		rtcAttachGeometryByID(scene, geom, 0);
		rtcCommitScene(scene);
		rtcInitIntersectContext(&context);
	}

	bool EmbreeRayTracer::FireRay(
		std::array<float, 3>& origin,
		const std::array<float, 3>& dir,
		float distance,
		int mesh_id
	) {
		return FireRay(origin[0], origin[1], origin[2], dir[0], dir[1], dir[2], distance, mesh_id);
	}

	bool EmbreeRayTracer::FireRay(
			float& x,
			float& y,
			float& z,
			float dx,
			float dy,
			float dz,
			float distance,
			int mesh_id) 
	{
		// Define an Embree hit data type to store results
		RTCRayHit hit;

		// Use the referenced values of the x,y,z position as the ray origin
		hit.ray.org_x = x; hit.ray.org_y = y; hit.ray.org_z = z;
		// Define the directions 
		hit.ray.dir_x = dx; hit.ray.dir_y = dy; hit.ray.dir_z = dz;

		hit.ray.tnear = 0.0f; // The start of the ray segment
		hit.ray.tfar = INFINITY; // The end of the ray segment
		hit.ray.time = 0.0f; // Time of ray for motion blur, unrelated to our package

		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.primID = -1;

		rtcIntersect1(scene, &context, &hit);

		CheckState(device);
		// If valid geometry was hit, and the geometry matches the caller's desired mesh
		// (if specified) then update the hitpoint and return
		if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID || (mesh_id > -1 && hit.hit.geomID != mesh_id)) return false;
		
		// If the ray did hit, update the node position by translating the distance along the directions
		// This REQUIRES a normalized vector
		else 
		{
			// Translate the point along the direction vector 
			x = x + (dx * hit.ray.tfar);
			y = y + (dy * hit.ray.tfar);
			z = z + (dz * hit.ray.tfar);

			return true;
		}
	}

	std::vector<char> EmbreeRayTracer::FireRays(
		std::vector<std::array<float, 3>>& origins,
		std::vector<std::array<float, 3>>& directions,
		bool use_parallel,
		float max_distance,
		int mesh_id
	) {
		std::vector<char> out_results(origins.size());

		// Allow users to shoot multiple rays with a single direction or origin
		if (origins.size() > 1 && directions.size() > 1) {
#pragma omp parallel for if(use_parallel) schedule(dynamic)
			for (int i = 0; i < origins.size(); i++) {
				auto& org = origins[i];
				auto& dir = directions[i];
				out_results[i] = FireRay(
					org[0], org[1], org[2],
					dir[0], dir[1], dir[2],
					max_distance, mesh_id
				);
			}
		}

		else if (origins.size() > 1 && directions.size() == 1) {
			const auto& dir = directions[0];
#pragma omp parallel for if(use_parallel) schedule(dynamic)
			for (int i = 0; i < origins.size(); i++) {
				auto& org = origins[i];
				out_results[i] = FireRay(
					org[0], org[1], org[2],
					dir[0], dir[1], dir[2],
					max_distance, mesh_id
				);
			}
		}

		else if (origins.size() == 1 && directions.size() > 1) {
			out_results.resize(directions.size());
#pragma omp parallel for if(use_parallel) schedule(dynamic)
			for (int i = 0; i < directions.size(); i++) {
				auto org = origins[0];
				auto& dir = directions[i];
				bool did_hit = FireRay(
					org[0], org[1], org[2],
					dir[0], dir[1], dir[2],
					max_distance, mesh_id
				);
				if (did_hit) // Directions is the output array in this case
				{
					directions[i][0] = org[0];
					directions[i][1] = org[1];
					directions[i][2] = org[2];
				}
				out_results[i] = did_hit;
			}
		}
		else {
			throw std::exception("Incorrect usage of firerays");
		}

		return out_results;
	}

	HitStruct EmbreeRayTracer::Intersect(
		float x, float y, float z,
		float dx, float dy, float dz,
		float max_distance, int mesh_id
	) {
		RTCRayHit hit;
		hit.ray.org_x = x; hit.ray.org_y = y; hit.ray.org_z = z;
		hit.ray.dir_x = dx; hit.ray.dir_y = dy; hit.ray.dir_z = dz;

		hit.ray.tnear = 0.00001f;
		hit.ray.tfar = (max_distance > 0) ? max_distance : INFINITY;
		hit.ray.time = 0.0f;

		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.primID = -1;

		rtcIntersect1(scene, &context, &hit);

		return HitStruct{ hit.ray.tfar, hit.hit.geomID };
	}

	bool EmbreeRayTracer::FireOcclusionRay(
		const std::array<float, 3>& origin,
		const std::array<float, 3>& direction,
		float max_dist
	) {
		return FireOcclusionRay(origin[0], origin[1], origin[2], direction[0], direction[1], direction[2]);
	}

	std::vector<char> EmbreeRayTracer::FireOcclusionRays(
		const std::vector<std::array<float, 3>>& origins,
		const std::vector<std::array<float, 3>>& directions,
		float max_distance, bool use_parallel)
	{
		std::vector<char> out_array;
		int cores = std::thread::hardware_concurrency();
		if (origins.size() < cores || directions.size() < cores)
			// Don't use more cores than there are rays. This caused a hard to find bug earlier.
			// Doesn't seem to happen with the other ray types. (race condition?)
			omp_set_num_threads(min(max(origins.size(), directions.size()), cores));

		if (directions.size() > 1 && origins.size() > 1) {
			out_array.resize(origins.size());
#pragma omp parallel for if(use_parallel) schedule(dynamic)
			for (int i = 0; i < origins.size(); i++) {
				//const std::array<float, 3> origin = { origins[i][0], origins[i][1], origins[i][2] };
				//const std::array<float, 3> direction = directions[i];
				//printf("Origin: %f, %f, %f\n", origin[0], origin[1]);
				//printf("Direction: %f, %f, %f\n", direction[0], direction[1], direction[2]);
				out_array[i] = FireOcclusionRay(
					origins[i][0], origins[i][1], origins[i][2],
					directions[i][0], directions[i][1], directions[i][2],
					max_distance, -1
				);
			}
		}
		else if (directions.size() > 1 && origins.size() == 1) {
			out_array.resize(directions.size());
			const auto& origin = origins[0];
			printf("Using multidirection, single origin\n");
#pragma omp parallel for if(use_parallel) schedule(dynamic)
			for (int i = 0; i < directions.size(); i++) {
				const auto& direction = directions[i];
				out_array[i] = FireOcclusionRay(
					origin[0], origin[1], origin[2],
					direction[0], direction[1], direction[2],
					max_distance, -1
				);
			}
		}
		else if (directions.size() == 1 && origins.size() > 1)
		{
			out_array.resize(origins.size());
			const auto& direction = directions[0];
#pragma omp parallel for if(use_parallel) schedule(dynamic)
			for (int i = 0; i < origins.size(); i++) {
				const auto& origin = origins[i];
				out_array[i] = FireOcclusionRay(
					origin[0], origin[1], origin[2],
					direction[0], direction[1], direction[2],
					max_distance, -1
				);
			}
		}
		else if (directions.size() == 1 && origins.size() == 1) {
			out_array = { FireOcclusionRay(origins[0], directions[0], max_distance) };
		}
		return out_array;
	}

	EmbreeRayTracer::EmbreeRayTracer(std::vector<HF::Geometry::MeshInfo>& MI) {
		// Throw if MI's size is less than 0
		if (MI.empty())
		{
			std::cerr << "Embree Ray Tracer was passed an empty vector of mesh info!" << std::endl;
			throw std::logic_error("Embree Ray Tracer was passed an empty vector of mesh info!");
		}

		device = rtcNewDevice("start_threads=1,set_affinity=1");
		scene = rtcNewScene(device);
		rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
		rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);

		InsertNewMesh(MI, true);
		rtcInitIntersectContext(&context);
	}

	EmbreeRayTracer::EmbreeRayTracer(HF::Geometry::MeshInfo& MI) {
		device = rtcNewDevice("start_threads=1,set_affinity=1");
		scene = rtcNewScene(device);
		rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
		rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);

		InsertNewMesh(MI, true);
		rtcInitIntersectContext(&context);
	}

	EmbreeRayTracer::EmbreeRayTracer(const EmbreeRayTracer& ERT2)
	{
		device = ERT2.device;
		context = ERT2.context;
		scene = ERT2.scene;
		geometry = ERT2.geometry;

		rtcRetainScene(scene);
		rtcRetainDevice(device);
	}

	bool EmbreeRayTracer::FireOcclusionRay(float x, float y, float z, float dx, float dy, float dz, float distance, int mesh_id)
	{
		RTCRay ray;
		ray.org_x = x; ray.org_y = y; ray.org_z = z;
		ray.dir_x = dx; ray.dir_y = dy; ray.dir_z = dz;

		ray.tnear = 0.0001f;
		ray.tfar = distance > 0 ? distance : INFINITY;
		ray.time = 0.0f;
		ray.flags = 0;

		rtcOccluded1(scene, &context, &ray);

		return ray.tfar == -INFINITY;
	}


	bool EmbreeRayTracer::InsertNewMesh(std::vector<std::array<float, 3>>& Mesh, int ID, bool Commit)
	{
		// Set Setup buffers
		std::vector<Triangle> tris;	std::vector<Vertex> verts;
		vectorsToBuffers(Mesh, tris, verts);
		
		RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

		// Create Geometry/Triangle buffers
		triangles = static_cast<Triangle*>(
			rtcSetNewGeometryBuffer(
				geom,
				RTC_BUFFER_TYPE_INDEX,
				0,
				RTC_FORMAT_UINT3,
				sizeof(Triangle),
				tris.size() + 1
			)
			);
		Vertices = static_cast<Vertex*>(
			rtcSetNewGeometryBuffer(
				geom,
				RTC_BUFFER_TYPE_VERTEX,
				0,
				RTC_FORMAT_FLOAT3,
				sizeof(Vertex),
				verts.size() + 1
			)
			);

		// Use the move from tris/verts into embree's buffers
		std::move(tris.begin(), tris.end(), triangles);
		std::move(verts.begin(), verts.end(), Vertices);

		// Even if the scene isn't going to be committed, we need to commit the 
		// geometry.
		rtcCommitGeometry(geom);
		rtcAttachGeometryByID(scene, geom, ID);
		geometry.push_back(geom);

		// Commit the scene if specified
		if (Commit)
			rtcCommitScene(scene);

		return ID;
	}

	bool EmbreeRayTracer::InsertNewMesh(HF::Geometry::MeshInfo& Mesh, bool Commit) {
		// Get vertex and triangle data from the mesh
		std::vector<Triangle> tris;	std::vector<Vertex> verts;
		auto indices = Mesh.getRawIndices(); auto vertices = Mesh.GetIndexedVertices();
		buffersToStructs(vertices, indices, verts, tris);

		// Create embree buffers
		RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
		triangles = static_cast<Triangle*>(
			rtcSetNewGeometryBuffer(
				geom,
				RTC_BUFFER_TYPE_INDEX,
				0,
				RTC_FORMAT_UINT3,
				sizeof(Triangle),
				tris.size() + 1
			)
			);
		Vertices = static_cast<Vertex*>(
			rtcSetNewGeometryBuffer(
				geom,
				RTC_BUFFER_TYPE_VERTEX,
				0,
				RTC_FORMAT_FLOAT3,
				sizeof(Vertex),
				verts.size() + 1
			)
			);
		if (!triangles || !Vertices) //? When would this occur
			throw std::exception("RTC DEVICE ERROR");

		// Move data into embree buffers
		std::move(tris.begin(), tris.end(), triangles);
		std::move(verts.begin(), verts.end(), Vertices);
		geometry.push_back(geom);
		rtcCommitGeometry(geom);

		// If the mesh has an ID, then use that ID. Otherwise get an id from embree and assign it to the mesh.
		const auto id = Mesh.GetMeshID();
		if (id > 0) rtcAttachGeometryByID(scene, geom, id);
		else Mesh.SetMeshID(rtcAttachGeometry(scene, geom));

		// commit if specified
		if (Commit)
			rtcCommitScene(scene);

		return true;
	}

	bool EmbreeRayTracer::InsertNewMesh(std::vector<HF::Geometry::MeshInfo>& Meshes, bool Commit)
	{
		for (auto& mesh : Meshes)
			InsertNewMesh(mesh, false);

		if (Commit) // Only commit at the end to save performance.
			rtcCommitScene(scene);

		return true;
	}

	// Increment reference counters to prevent destruction when this thing goes out of scope
	void EmbreeRayTracer::operator=(const EmbreeRayTracer& ERT2) {
		device = ERT2.device;
		context = ERT2.context;
		scene = ERT2.scene;
		geometry = ERT2.geometry;

		rtcRetainScene(scene);
		rtcRetainDevice(device);
		//for (auto g : geometry) rtcRetainGeometry(g);
	}

	EmbreeRayTracer::~EmbreeRayTracer() {
		//for (auto& g : geometry) rtcReleaseGeometry(g);
		rtcReleaseScene(scene);
		rtcReleaseDevice(device);
	}
}