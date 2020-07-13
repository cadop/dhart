///
///	\file		meshinfo.cpp
/// \brief		Contains implementation for the <see cref = "HF::SpatialStructures::MeshInfo">MeshInfo</cref> class
///
///	\author		TBA
///	\date		16 Jun 2020

#include <meshinfo.h>
#include <Geometry>
#include <HFExceptions.h>
#include <math.h>
#include <corecrt_math_defines.h>
#include <iostream>
#include <robin_hood.h>


#define _USE_MATH_DEFINES

using std::array;
using std::vector;
namespace HF::Geometry {
	MeshInfo::MeshInfo(const vector<array<float, 3>>& vertices, int id, std::string name)
	{
		// Throw if the input array has no values in it. 
		const size_t n = vertices.size();
		if (vertices.size() < 1) throw HF::Exceptions::InvalidOBJ(); // Doesn't have any valid geometry

		//AddVerts(vertices);
		VectorsToBuffers(vertices);

		// Throw if a NAN was placed into the mesh. 
		if (verts.hasNaN()) throw std::exception("NAN DETECTED ON INPUT");
	
		meshid = id;
		name = n;
	}

	inline void MeshInfo::SetVert(int index, float x, float y, float z)
	{
		// the () operator is overloaded for eigen to index the array
		// For example array(row, col) will index the value at row, col.
		verts(0, index) = x;
		verts(1, index) = y;
		verts(2, index) = z;
	}


	/*!
		\brief Index an array of vertices.
		\param vertices An array of vertices for a mesh organized so every 3 vertices
		represents a triangle on the mesh.
		\param mapped_indexes Output parameter for array of indicies
		\param mapped_vertices Output parameter for vertex array.

		\details 
		Iterate through every array in vertices. If the x,y,z location in the array
		hasn't been seen yet, add its coordinates to mapped_vertices, assign it a unique ID
		in a an index hashmap, then add its id to mapped_indexes. IF it has been seen
		add the existing ID in the index hashmap to mapped_indexes.


		\pre mapped_indexes and mapped_vertices are empty vectors of integers and floats respectively.

		\post 1) mapped_indexes contains an array of indexes for mapped_vertices where
		every 3 indices represents a different triangle on the mesh.
		\post 2) mapped_vertices contains an array of floats with each 3 vertices
		represent the x,y, and z coordinates for a different vertex in the mesh with 
		no repeats.
	*/
	void IndexRawVertices(
		const vector<array<float, 3>>& vertices,
		vector<int>& mapped_indexes,
		vector<float>& mapped_vertices
	) {
		// Create a hashmap to map vertices to indices.
		robin_hood::unordered_map <array<float, 3>, int> index_map;
		int next_id = 0;

		int vertsize = vertices.size();
		
		// Iterate through every 3 vertexes in vertices
		for (int i = 0; i < vertsize; i += 3) {
			array<int, 3> ids;

			// Iterate through every 3 vertexes in vertices. 
			for (int k = 0; k < 3; k++)
			{
				// Get the vertex at this position
				const auto& vert = vertices[i + k];
				int current_id;

				// Get ID from index map, or create a new ID if one doesn't exist
				if (index_map.count(vert) > 0)
					current_id = index_map[vert];
				else {

					// store the id in the hashmap for this vertex.
					index_map[vert] = next_id;
					current_id = next_id;

					// If this is a new vertex, add it the mapped_vertices vector
					mapped_vertices.push_back(vert[0]);
					mapped_vertices.push_back(vert[1]);
					mapped_vertices.push_back(vert[2]);
					next_id++;
				}
				// Store this new ID in the array.
				ids[k] = current_id;
			}

			// Push the indexes back into the triangle array
			mapped_indexes.push_back(ids[0]);
			mapped_indexes.push_back(ids[1]);
			mapped_indexes.push_back(ids[2]);
		}
	}

	void MeshInfo::VectorsToBuffers(const vector<array<float, 3>>& vertices)
	{
		// Create and fill vectors
		vector<int> mapped_indexes; vector<float> mapped_vertices;
		IndexRawVertices(vertices, mapped_indexes, mapped_vertices);

		// This OBJ isn't valid if the following doesn't hold
		if (!(mapped_indexes.size() % 3 == 0))
			throw HF::Exceptions::InvalidOBJ();

		// Copy contents into index and vertex vectors into matrices.
		verts.resize(3, mapped_vertices.size() / 3);
		indices.resize(3, mapped_indexes.size() / 3);
		std::move(mapped_vertices.begin(), mapped_vertices.end(), verts.data());
		std::move(mapped_indexes.begin(), mapped_indexes.end(), indices.data());
	}

	MeshInfo::MeshInfo(
		const vector<float>& in_vertices,
		const vector<int>& in_indexes,
		int id,
		std::string name
	) {
		// If the size of vertices or the size of indexes aren't multiples of 3
		// then we can easily rule this out as an invalid mesh
		if (in_vertices.size() % 3 != 0 || in_indexes.size() % 3 != 0)
			throw HF::Exceptions::InvalidOBJ();

		// Resize verts and indices, then std::move valuees into them.
		verts.resize(3, in_vertices.size() / 3);
		indices.resize(3, in_indexes.size() / 3);
		std::move(in_vertices.begin(), in_vertices.end(), verts.data());
		std::move(in_indexes.begin(), in_indexes.end(), indices.data());

		meshid = id;
		name = name;
	}

	void MeshInfo::AddVerts(const vector<array<float, 3>>& in_vertices)
	{
		if (in_vertices.size() % 3 != 0) throw HF::Exceptions::InvalidOBJ(); // Incomplete triangle

		verts.resize(3, (static_cast<size_t>(verts.cols()) + in_vertices.size()));

		for (int i = 0; i < in_vertices.size(); i++) {
			auto& vertex = in_vertices[i];
			verts(0, i) = vertex[0];
			verts(1, i) = vertex[1];
			verts(2, i) = vertex[2];
		}
		if (verts.hasNaN()) throw std::exception("Creation of mesh info failed");
	}

	int MeshInfo::NumVerts() const { return static_cast<int>(verts.cols()); }

	int MeshInfo::NumTris() const { return static_cast<int>(indices.cols()); }

	void MeshInfo::ConvertToRhinoCoordinates()
	{
		Eigen::AngleAxis<float> yrot(0.5f * static_cast<float>(M_PI), Eigen::Vector3f::UnitX());
		Eigen::Quaternion<float> quat;
		quat = yrot;
		quat.normalize();
		verts = yrot.toRotationMatrix() * verts;
		//auto newer_matrix = new_matrix.eval();
		if (!verts.allFinite()) throw std::exception("Verts has NAN");
	}

	void MeshInfo::ConvertToOBJCoordinates()
	{
		Eigen::AngleAxis<float> yrot(-0.5f * static_cast<float>(M_PI), Eigen::Vector3f::UnitX());
		Eigen::Quaternion<float> quat;
		quat = yrot;
		quat.normalize();
		Eigen::Matrix3f rotation_matrix = yrot.toRotationMatrix();
		assert(!rotation_matrix.hasNaN());
		verts = (rotation_matrix * verts);
		assert(!verts.hasNaN());
	}

	void MeshInfo::PerformRotation(float rx, float ry, float rz)
	{
		// Convert to radians
		float radian_ratio = static_cast<float>(M_PI) / 180.00f;
		rx *= radian_ratio; ry *= radian_ratio; rz *= radian_ratio;

		// Implementation based on 
		// https://stackoverflow.com/questions/21412169/creating-a-rotation-matrix-with-pitch-yaw-roll-using-eigen
		Eigen::AngleAxis<float> rollAngle(rz, Eigen::Vector3f::UnitZ());
		Eigen::AngleAxis<float> yawAngle(ry, Eigen::Vector3f::UnitY());
		Eigen::AngleAxis<float> pitchAngle(rx, Eigen::Vector3f::UnitX());


		// Create a quaternion from the angles, normalize it, then
		// convert it to a rotation matrix.
		Eigen::Quaternion<float> q = (rollAngle * yawAngle * pitchAngle);
		q.normalize();
		Eigen::Matrix3f rotation_matrix = q.toRotationMatrix();

		// Assert that we didn't create any NANS or infinite values
		assert(rotation_matrix.allFinite());

		// Apply the rotation matrix to verts
		verts = (rotation_matrix * verts);

		// Once again assert that we didn't create any nans or infinite numbers.
		assert(verts.allFinite());
	}

	int MeshInfo::GetMeshID() const { return meshid; }

	vector<float> MeshInfo::GetIndexedVertices() const
	{
		// Preallocate space for all vertices
		vector<float> out_array(verts.size());
		
		// Copy verts into it
		std::copy(verts.data(), verts.data() + verts.size(), out_array.begin());
		
		return out_array;
	}

	vector<int> MeshInfo::getRawIndices() const
	{
		vector<int> out_array(indices.size());
		std::copy(indices.data(), indices.data() + indices.size(), out_array.begin());
		return out_array;
	}

	vector<array<float, 3>> MeshInfo::GetUnindexedVertices() const
	{
		// Preallocate an array 3x the size of the triangle matrix
		int tri_count = NumTris();
		vector<array<float, 3>>out_array(tri_count * 3);

		// Fill arrays with values of vertices
		for (int i = 0; i < tri_count; i++)
		{
			const int offset = i * 3;
			out_array[offset] = (*this)[indices(0, i)];
			out_array[offset + 1] = (*this)[indices(1, i)];
			out_array[offset + 2] = (*this)[indices(2, i)];
		}
		return out_array;
	}

	void MeshInfo::SetMeshID(int new_id) { meshid = new_id; }

	array<float, 3> MeshInfo::operator[](int i) const
	{
		// Throw if going beyond the array bounds
		if (i < 0 || i > NumVerts()) throw std::exception("Out of range on index");
	
		// Create and return out array.
		array<float, 3> out_array;
		out_array[0] = verts(0, i);
		out_array[1] = verts(1, i);
		out_array[2] = verts(2, i);
		return out_array;
	}

	/*! \brief Calculate the distance between from and to
	
		\param from x,y,z position to calculate distance from.
		\param to x,y,z position to calculate distance to.
		
		\returns The distance between from and to. 
	*/
	float arrayDist(const array<float, 3> from, const array<float, 3>& to) {
		return sqrtf(powf(from[0] - to[0], 2) + powf(from[1] - to[1], 2) + powf(from[2] - to[2], 2));
	}
	bool MeshInfo::operator==(const MeshInfo& M2) const
	{
		// Get out quickly if the number of vertices do not match
		if (NumVerts() != M2.NumVerts()) return false;

		for (int i = 0; i < NumVerts(); i++) {
			auto this_array = (*this)[i];
			auto that_array = M2[i];

			if (!(arrayDist(this_array, that_array) < 0.001))
				return false;
		}
		return true;
	}
}