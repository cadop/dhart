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
	bool HitStructD<double>::DidHit() const { return meshid != RTC_INVALID_GEOMETRY_ID; };

	/// <summary>
	/// Check an embree device for errors.
	/// </summary>
	/// <param name="device">Device to check for errors</param>
	/// <exception cref="std::exception">Some error is found while casting rays.</exception>
	RTCError CheckState(RTCDevice& device) {
		return rtcGetDeviceError(device);
	}

	using std::vector;
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

	EmbreeRayTracer::EmbreeRayTracer(bool use_precise)
	{
		this->use_precise = false;
		SetupScene();
	}

	EmbreeRayTracer::EmbreeRayTracer(std::vector<HF::Geometry::MeshInfo>& MI, bool use_precise) {
		// Throw if MI's size is less than 0
		this->use_precise = true;

		if (MI.empty())
			throw std::logic_error("Embree Ray Tracer was passed an empty vector of mesh info!");

		SetupScene();

		InsertNewMesh(MI, true);
	}

	EmbreeRayTracer::EmbreeRayTracer(HF::Geometry::MeshInfo& MI, bool use_precise) {
		SetupScene();
		this->use_precise = use_precise;
		InsertNewMesh(MI, true);
	}

	void EmbreeRayTracer::SetupScene() {
		device = rtcNewDevice("");
		scene = rtcNewScene(device);
		rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
		rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);

		rtcInitIntersectContext(&context);
	}

	EmbreeRayTracer::EmbreeRayTracer(const EmbreeRayTracer& ERT2)
	{
		// Copy over pointers to relevant embree objects
		device = ERT2.device;
		context = ERT2.context;
		scene = ERT2.scene;
		geometry = ERT2.geometry;

		// Increment embree's internal refrence counter.
		rtcRetainScene(scene);
		rtcRetainDevice(device);
	}

	int EmbreeRayTracer::TryToAddByID(RTCGeometry& geom, int id)
	{
		if (id >= 0) {
			rtcAttachGeometryByID(scene, geom, id);

			// If we're attaching by ID, then we must check to see if the ID already exists.
			// If it does already exist, then we'll have to just add it without specifying any ID
			RTCError error = CheckState(device);

			// Don't know the specific error that will be raised here (Documentation just states some error code)
			if (error != RTCError::RTC_ERROR_NONE)
				return TryToAddByID(geom);
		}
		else
			return rtcAttachGeometry(scene, geom);
	}

	EmbreeRayTracer::EmbreeRayTracer(const std::vector<std::array<float, 3>>& geometry) {
		SetupScene();

		for (auto geom : geometry) {
			// Set Setup buffers
			std::vector<Triangle> tris;
			std::vector<Vertex> verts;
			vectorsToBuffers(geometry, tris, verts);

			RTCGeometry geom = InsertGeometryFromBuffers(tris, verts);
			// Cast to triangle/vertex structs so we can operate on them.  This trick is from the embree tutorial
			TryToAddByID(geom);
		}

		rtcCommitScene(scene);
	}

	bool EmbreeRayTracer::InsertNewMesh(std::vector<std::array<float, 3>>& Mesh, int ID, bool Commit)
	{
		// Set Setup buffers
		std::vector<Triangle> tris;	std::vector<Vertex> verts;
		vectorsToBuffers(Mesh, tris, verts);

		// Add the geometry to embree as a geometry object
		auto geom = InsertGeometryFromBuffers(tris, verts);

		// Add the geometry by ID
		int added_id = TryToAddByID(geom);

		// Commit the scene if specified
		if (Commit)
			rtcCommitScene(scene);

		// Return False if it's id didn't need to be changed, true otherwise.
		return (added_id == ID);
	}

	RTCGeometry EmbreeRayTracer::InsertGeometryFromBuffers(vector<Triangle>& tris, vector<Vertex>& verts) {
		// Create new geometry object
		RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

		// Allocate it's triangle and index buffers in embree
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

		// Move data from input tris/verts into buffers
		std::move(tris.begin(), tris.end(), triangles);
		std::move(verts.begin(), verts.end(), Vertices);

		// Add a reference to this geometry to internal array of geometry.
		geometry.push_back(geom);
		
		// Commit this geometry to finalize the process then return
		rtcCommitGeometry(geom);

		return geom;
	}

	bool EmbreeRayTracer::InsertNewMesh(HF::Geometry::MeshInfo& Mesh, bool Commit) {

		if (Mesh.NumTris() < 1 || Mesh.NumVerts() < 1) 
			throw HF::Exceptions::InvalidOBJ();
		// Get vertex and triangle data from the mesh

		std::vector<Triangle> tris;	std::vector<Vertex> verts;
		auto indices = Mesh.getRawIndices(); auto vertices = Mesh.GetIndexedVertices();
		buffersToStructs(vertices, indices, verts, tris);

		// Construct geometry using embree
		auto geom = InsertGeometryFromBuffers(tris, verts);

		// Add the Mesh to the scene and update it's ID
		Mesh.meshid = TryToAddByID(geom, Mesh.meshid);

		// commit if specified
		if (Commit)
			rtcCommitScene(scene);

		return true;
	}

	bool EmbreeRayTracer::InsertNewMesh(std::vector<HF::Geometry::MeshInfo>& Meshes, bool Commit)
	{
		// Add every mesh in a loop
		for (auto& mesh : Meshes)
			InsertNewMesh(mesh, false);

		// Commit at the end to save performance
		if (Commit)
			rtcCommitScene(scene);

		return true;
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

		hit.ray.tnear = 0.00000001f; // The start of the ray segment
		hit.ray.tfar = INFINITY; // The end of the ray segment
		hit.ray.time = 0.0f; // Time of ray for motion blur, unrelated to our package

		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.primID = -1;

		rtcIntersect1(scene, &context, &hit);

		CheckState(device);
		// If valid geometry was hit, and the geometry matches the caller's desired mesh
		// (if specified) then update the hitpoint and return
		if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID || (mesh_id > -1 && hit.hit.geomID != mesh_id)) return false;

		auto distance_to_hit = hit.ray.tfar;

		// Use precise intersection if this is specified
		if (use_precise) {
			unsigned int geom_id = hit.hit.geomID;
			auto geometry = this->geometry[geom_id];

			// Construct a Vector3D of the triangle
			auto triangle = this->GetTriangle(geom_id, hit.hit.primID);

			distance_to_hit = RayTriangleIntersection(
				Vector3D{ x,y,z },
				Vector3D{ dx,dy,dz },
				triangle[0],
				triangle[1],
				triangle[2]
			);
		}

		// If the ray did hit, update the node position by translating the distance along the directions
		// This REQUIRES a normalized vector
		// Translate the point along the direction vector 
		x = x + (dx * distance_to_hit);
		y = y + (dy * distance_to_hit);
		z = z + (dz * distance_to_hit);

		return true;
	}

	inline Vector3D GetPointFromBuffer(int index, Vertex* buffer) {
		return	Vector3D{buffer[index].x, buffer[index].y,buffer[index].z};
	}

	std::array<Vector3D, 3> EmbreeRayTracer::GetTriangle(int geomID, int primID)
	{
		Triangle* index_buffer = reinterpret_cast<Triangle*>(rtcGetGeometryBufferData(
			rtcGetGeometry(this->scene, geomID),
			RTCBufferType::RTC_BUFFER_TYPE_INDEX,
			0
		));

		int x_index = index_buffer[primID].v0;
		int y_index = index_buffer[primID].v1;
		int z_index = index_buffer[primID].v2;

		Vertex * vertex_buffer = reinterpret_cast<Vertex *>(rtcGetGeometryBufferData(
			rtcGetGeometry(this->scene, geomID),
			RTCBufferType::RTC_BUFFER_TYPE_VERTEX,
			0
		));

		CheckState(this->device);

		return std::array<Vector3D, 3>{
				GetPointFromBuffer(x_index, vertex_buffer),
				GetPointFromBuffer(y_index, vertex_buffer),
				GetPointFromBuffer(z_index, vertex_buffer)
		};
	}

	HitStruct EmbreeRayTracer::FirePreciseRay(
		double x, double y, double z,
		double dx, double dy, double dz,
		double distance,	int mesh_id)
	{
		// Define an Embree hit data type to store results
		RTCRayHit hit;

		// Use the referenced values of the x,y,z position as the ray origin
		hit.ray.org_x = x; hit.ray.org_y = y; hit.ray.org_z = z;
		// Define the directions 
		hit.ray.dir_x = dx; hit.ray.dir_y = dy; hit.ray.dir_z = dz;

		hit.ray.tnear = 0.00000001f; // The start of the ray segment
		hit.ray.tfar = INFINITY; // The end of the ray segment
		hit.ray.time = 0.0f; // Time of ray for motion blur, unrelated to our package

		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.primID = -1;

		// Cast the ray and update the hitstruct
		rtcIntersect1(scene, &context, &hit);

		// If valid geometry was hit, and the geometry matches the caller's desired mesh
		// (if specified) then update the hitpoint and return
		if (hit.hit.geomID == RTC_INVALID_GEOMETRY_ID || (mesh_id > -1 && hit.hit.geomID != mesh_id)) return HitStruct();

		unsigned int geom_id = hit.hit.geomID;
		auto geometry = this->geometry[geom_id];

		// Construct a Vector3D of the triangle
		auto triangle = this->GetTriangle(geom_id, hit.hit.primID);
		
		double ray_distanceD = RayTriangleIntersection(
			Vector3D{ x,y,z },
			Vector3D{ dx,dy,dz },
			triangle[0],
			triangle[1],
			triangle[2]
		);
		float ray_distance = static_cast<float>(ray_distanceD);
		return HitStruct{ ray_distance, geom_id};
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

		hit.ray.tnear = 0.0f;
		hit.ray.tfar = (max_distance > 0) ? max_distance : INFINITY;
		hit.ray.time = 0.0f;

		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.primID = -1;

		rtcIntersect1(scene, &context, &hit);

		return HitStruct { hit.ray.tfar, hit.hit.geomID };
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

	// Increment reference counters to prevent destruction when this thing goes out of scope
	void EmbreeRayTracer::operator=(const EmbreeRayTracer& ERT2) {

		this->use_precise = ERT2.use_precise;
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