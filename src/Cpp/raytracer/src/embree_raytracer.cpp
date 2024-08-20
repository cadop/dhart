///
/// \file		embree_raytracer.cpp
/// \brief		Contains implementation for the <see cref="HF::RayTracer::EmbreeRayTracer">EmbreeRayTracer</see>
///
///	\author		TBA
///	\date		26 Jun 2020

#include <embree_raytracer.h>
#ifdef _WIN32
#include <corecrt_math_defines.h>
#endif
#include <functional>
#include <iostream>
#include <thread>
#include <robin_hood.h>

#include <meshinfo.h>
#include <RayRequest.h>
#include <HFExceptions.h>

using std::vector;

namespace HF::RayTracer {

	template <typename numeric1, typename numeric2, typename dist_type = float>
	inline RTCRayHit ConstructHit(
		numeric1 x, numeric1 y, numeric1 z,
		numeric2 dx, numeric2 dy, numeric2 dz,
		dist_type distance = -1.0f, int mesh_id = -1) 
	{
		// Define an Embree hit data type to store results
		RTCRayHit hit;

		// Use the referenced values of the x,y,z position as the ray origin
		hit.ray.org_x = x; hit.ray.org_y = y; hit.ray.org_z = z;
		// Define the directions 
		hit.ray.dir_x = dx; hit.ray.dir_y = dy; hit.ray.dir_z = dz;

		hit.ray.tnear = 0.00000001f; // The start of the ray segment
		hit.ray.tfar = distance > 0 ? distance : INFINITY; // The end of the ray segment
		hit.ray.time = 0.0f; // Time of ray for motion blur, unrelated to our package

		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.primID = -1;

		return hit;
	}

	template <typename numeric1, typename numeric2, typename dist_type = float>
	inline RTCRay ConstructRay(
		numeric1 x, numeric1 y, numeric1 z,
		numeric2 dx, numeric2 dy, numeric2 dz,
		dist_type distance = -1.0f, int mesh_id = -1)
	{
		RTCRay ray;
		ray.org_x = x; ray.org_y = y; ray.org_z = z;
		ray.dir_x = dx; ray.dir_y = dy; ray.dir_z = dz;

		ray.tnear = 0.0000001f;
		ray.tfar = (distance > 0) ? distance : INFINITY;
		ray.time = 0.0f;
		ray.flags = 0;
			
		return ray;
	}
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

	EmbreeRayTracer::EmbreeRayTracer(std::vector<HF::Geometry::MeshInfo<float>>& MI, bool use_precise) {
		// Throw if MI's size is less than 0
		this->use_precise = true;

		if (MI.empty())
			throw std::logic_error("Embree Ray Tracer was passed an empty vector of mesh info!");

		SetupScene();

		AddMesh(MI, true);
	}

	EmbreeRayTracer::EmbreeRayTracer(HF::Geometry::MeshInfo<float>& MI, bool use_precise) {
		SetupScene();
		this->use_precise = use_precise;
		AddMesh(MI, true);
	}

	void EmbreeRayTracer::SetupScene() {
		device = rtcNewDevice("");
		scene = rtcNewScene(device);
		rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
		rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
		// Initialize the intersect context, which should later allow RTC_INTERSECT_CONTEXT_FLAG_COHERENT
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

	int EmbreeRayTracer::InsertGeom(RTCGeometry& geom, int id)
	{
		if (id >= 0) {
			rtcAttachGeometryByID(scene, geom, id);

			// If we're attaching by ID, then we must check to see if the ID already exists.
			// If it does already exist, then we'll have to just add it without specifying any ID
			RTCError error = CheckState(device);

			// Don't know the specific error that will be raised here (Documentation just states some error code)
			if (error != RTCError::RTC_ERROR_NONE)
				return InsertGeom(geom);
		}
	
		return static_cast<int>(rtcAttachGeometry(scene, geom));
	}

	inline Vector3D cross(const Vector3D& x, const Vector3D& y) {
		return Vector3D{
			x.y * y.z - y.y * x.z,
			x.z * y.x - y.z * x.x,
			x.x * y.y - y.x * x.y
		};
	}

	inline double dot(const Vector3D& v1, const Vector3D& v2) {
		return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
	}

	inline Vector3D InvertVector(const Vector3D& V) {
		return Vector3D{ -V.x, -V.y, -V.z };
	}


	double RayTriangleIntersection(
		const Vector3D& origin,
		const Vector3D& direction,
		const Vector3D& v1,
		const Vector3D& v2,
		const Vector3D& v3)
	{

		const double EPSILON = 0.0000001;
		const Vector3D inverted_direction = direction;//InvertVector(direction);

		const Vector3D edge1 = v2 - v1;
		const Vector3D edge2 = v3 - v1;
		const Vector3D h = cross(inverted_direction, edge2);
		const double a = dot(edge1, h);

		// This ray is parallel to this triangle.
		if (a > -EPSILON && a < EPSILON)
			return -1;

		const double f = 1.0 / a;
		const Vector3D s = origin - v1;
		const double u = f * dot(s, h);
		if (u < 0.0 || u > 1.0)
			return -1;

		const Vector3D q = cross(s, edge1);
		const double v = f * dot(direction, q);
		if (v < 0.0 || u + v > 1.0)
			return -1;

		// At this stage we can compute t to find out where the intersection point is on the line.
		double t_hit = f * dot(edge2, q);
		return t_hit;
	}

	double EmbreeRayTracer::CalculatePreciseDistance(
		unsigned int geom_id,
		unsigned int prim_id, 
		const Vector3D & origin, 
		const Vector3D & direction) const
	{
		// Get the triangle intersected by this hit
		auto triangle = this->GetTriangle(geom_id, prim_id);

		// Perform the raytriangle intersection
		return RayTriangleIntersection(
			origin,
			direction,
			triangle[0],
			triangle[1],
			triangle[2]
		);
	}

	EmbreeRayTracer::EmbreeRayTracer(const std::vector<std::array<float, 3>>& geometry) {
		SetupScene();

		// Set Setup buffers
		std::vector<Triangle> tris;
		std::vector<Vertex> verts;
		vectorsToBuffers(geometry, tris, verts);

		RTCGeometry inserted_geom = ConstructGeometryFromBuffers(tris, verts);
		// Cast to triangle/vertex structs so we can operate on them.  This trick is from the embree tutorial
		InsertGeom(inserted_geom);

		rtcCommitScene(scene);
	}

	bool EmbreeRayTracer::AddMesh(std::vector<std::array<float, 3>>& Mesh, int ID, bool Commit)
	{
		// Set Setup buffers
		std::vector<Triangle> tris;	std::vector<Vertex> verts;
		vectorsToBuffers(Mesh, tris, verts);

		// Add the geometry to embree as a geometry object
		auto geom = ConstructGeometryFromBuffers(tris, verts);

		// Add the geometry by ID
		int added_id = InsertGeom(geom);

		// Commit the scene if specified
		if (Commit)
			rtcCommitScene(scene);

		// Return False if it's id didn't need to be changed, true otherwise.
		return (added_id == ID);
	}

	RTCGeometry EmbreeRayTracer::ConstructGeometryFromBuffers(vector<Triangle>& tris, vector<Vertex>& verts) {
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

	bool EmbreeRayTracer::AddMesh(HF::Geometry::MeshInfo<float>& Mesh, bool Commit) {

		if (Mesh.NumTris() < 1 || Mesh.NumVerts() < 1) 
			throw HF::Exceptions::InvalidOBJ();
		// Get vertex and triangle data from the mesh

		std::vector<Triangle> tris;	std::vector<Vertex> verts;
		auto indices = Mesh.getRawIndices(); auto vertices = Mesh.GetIndexedVertices();
		buffersToStructs(vertices, indices, verts, tris);

		// Construct geometry using embree
		auto geom = ConstructGeometryFromBuffers(tris, verts);

		// Add the Mesh to the scene and update it's ID
		Mesh.meshid = InsertGeom(geom, Mesh.meshid);

		// commit if specified
		if (Commit)
			rtcCommitScene(scene);

		return true;
	}

	bool EmbreeRayTracer::AddMesh(std::vector<HF::Geometry::MeshInfo<float>>& Meshes, bool Commit)
	{
		// Add every mesh in a loop
		for (auto& mesh : Meshes)
			AddMesh(mesh, false);

		// Commit at the end to save performance
		if (Commit)
			rtcCommitScene(scene);

		return true;
	}

	bool EmbreeRayTracer::PointIntersection(
		std::array<float, 3>& origin,
		const std::array<float, 3>& dir,
		float distance,
		int mesh_id
	) {
		return PointIntersection(origin[0], origin[1], origin[2], dir[0], dir[1], dir[2], distance, mesh_id);
	}

	bool EmbreeRayTracer::PointIntersection(
			float& x,
			float& y,
			float& z,
			float dx,
			float dy,
			float dz,
			float distance,
			int mesh_id) 
	{
		HitStruct<float> res = Intersect<float>(x, y, z, dx, dy, dz, distance, mesh_id);

		// If the ray did hit, update the node position by translating the distance along the directions
		// This REQUIRES a normalized vector
		// Translate the point along the direction vector 
		if (res.DidHit()) {
			x = x + (dx * res.distance);
			y = y + (dy * res.distance);
			z = z + (dz * res.distance);

			return true;
		}
		else
			return false;
	}

	inline Vector3D GetPointFromBuffer(int index, Vertex* buffer) {
		return	Vector3D{buffer[index].x, buffer[index].y,buffer[index].z};
	}

	std::array<Vector3D, 3> EmbreeRayTracer::GetTriangle(unsigned int geomID, unsigned int primID) const
	{
		// Get the index buffer for the geometry at geomID
		Triangle* index_buffer = reinterpret_cast<Triangle*>(rtcGetGeometryBufferData(
			rtcGetGeometry(this->scene, geomID),
			RTCBufferType::RTC_BUFFER_TYPE_INDEX,
			0
		));

		// Get the indices of the 3 vertices that form this triangle from thig
		// geometry's index buffer
		int v1_index = index_buffer[primID].v0;
		int v2_index = index_buffer[primID].v1;
		int v3_index = index_buffer[primID].v2;

		// Get a pointer to this geometry's vertex buffer
		Vertex * vertex_buffer = reinterpret_cast<Vertex *>(rtcGetGeometryBufferData(
			rtcGetGeometry(this->scene, geomID),
			RTCBufferType::RTC_BUFFER_TYPE_VERTEX,
			0
		));

		// Get the vertices at these indices from the vertex buffer.
		return std::array<Vector3D, 3>{
				GetPointFromBuffer(v1_index, vertex_buffer),
				GetPointFromBuffer(v2_index, vertex_buffer),
				GetPointFromBuffer(v3_index, vertex_buffer)
		};
	}

	std::vector<char> EmbreeRayTracer::PointIntersections(
		std::vector<std::array<float, 3>>& origins,
		std::vector<std::array<float, 3>>& directions,
		bool use_parallel,
		float max_distance,
		int mesh_id
	) {
		std::vector<char> out_results(origins.size());

		// Allow users to shoot multiple rays with a single direction or origin
		if (origins.size() > 1 && directions.size() > 1) {
#pragma omp parallel for if(use_parallel) schedule(dynamic, 128)
			for (int i = 0; i < origins.size(); i++) {
				auto& org = origins[i];
				auto& dir = directions[i];
				out_results[i] = PointIntersection(
					org[0], org[1], org[2],
					dir[0], dir[1], dir[2],
					max_distance, mesh_id
				);
			}
		}

		else if (origins.size() > 1 && directions.size() == 1) {
			const auto& dir = directions[0];
#pragma omp parallel for if(use_parallel) schedule(dynamic, 128)
			for (int i = 0; i < origins.size(); i++) {
				auto& org = origins[i];
				out_results[i] = PointIntersection(
					org[0], org[1], org[2],
					dir[0], dir[1], dir[2],
					max_distance, mesh_id
				);
			}
		}

		else if (origins.size() == 1 && directions.size() > 1) {
			out_results.resize(directions.size());
#pragma omp parallel for if(use_parallel) schedule(dynamic, 128)
			for (int i = 0; i < directions.size(); i++) {
				auto org = origins[0];
				auto& dir = directions[i];
				bool did_hit = PointIntersection(
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
			throw std::runtime_error("Incorrect usage of castrays");
		}

		return out_results;
	}

	RTCRayHit EmbreeRayTracer::Intersect_IMPL(
		float x, float y, float z,
		float dx, float dy, float dz,
		float max_distance, int mesh_id)
	{
		RTCRayHit hit = ConstructHit(x, y, z, dx, dy, dz);

		rtcIntersect1(scene, &context, &hit);

		return hit;
	}

	bool EmbreeRayTracer::Occluded_IMPL(
		const std::array<float, 3>& origin,
		const std::array<float, 3>& direction,
		float max_dist
	) {
		return Occluded_IMPL(origin[0], origin[1], origin[2], direction[0], direction[1], direction[2]);
	}

	std::vector<char> EmbreeRayTracer::Occlusions(
		const std::vector<std::array<float, 3>>& origins,
		const std::vector<std::array<float, 3>>& directions,
		float max_distance, bool use_parallel)
	{
		std::vector<char> out_array;
		int cores = static_cast<int> (std::thread::hardware_concurrency());
		if (origins.size() < cores || directions.size() < cores)
			// Don't use more cores than there are rays. This caused a hard to find bug earlier.
			// Doesn't seem to happen with the other ray types. (race condition?)
			omp_set_num_threads(std::min(std::max(origins.size(), directions.size()), static_cast<size_t>(cores)));

		if (directions.size() > 1 && origins.size() > 1) {
			out_array.resize(origins.size());
#pragma omp parallel for if(use_parallel) schedule(dynamic, 128)
			for (int i = 0; i < origins.size(); i++) {
				out_array[i] = Occluded_IMPL(
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
#pragma omp parallel for if(use_parallel) schedule(dynamic, 128)
			for (int i = 0; i < directions.size(); i++) {
				const auto& direction = directions[i];
				out_array[i] = Occluded_IMPL(
					origin[0], origin[1], origin[2],
					direction[0], direction[1], direction[2],
					max_distance, -1
				);
			}
		}

		// This is might seem to be an application of coherent rays with streaming, 
		// but coherent rays also require the origins to be very similar, so it doesn't help
		else if (directions.size() == 1 && origins.size() > 1)
		{
			out_array.resize(origins.size());
			const auto& direction = directions[0];

			// Use chunk size of 256 for reducing parallel overhead
		#pragma omp parallel for if(use_parallel) schedule(dynamic, 256)
			for (int i = 0; i < origins.size(); i++) {
				const auto& origin = origins[i];
				out_array[i] = Occluded_IMPL(origin[0], origin[1], origin[2], 
											direction[0], direction[1], direction[2], 
											max_distance, -1);
			}
		}

		else if (directions.size() == 1 && origins.size() == 1) {
			out_array = { Occluded_IMPL(origins[0], directions[0], max_distance) };
		}
		return out_array;
	}

	bool EmbreeRayTracer::Occluded_IMPL(float x, float y, float z, float dx, float dy, float dz, float distance, int mesh_id)
	{
		auto ray = ConstructRay(x, y, z, dx, dy, dz, distance);
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
	}

	EmbreeRayTracer::~EmbreeRayTracer() {
		rtcReleaseScene(scene);
		rtcReleaseDevice(device);
	}

}