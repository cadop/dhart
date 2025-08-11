///
///	\file		meshinfo.cpp
/// \brief		Contains implementation for the <see cref = "HF::Geometry::MeshInfo">MeshInfo</see> class
///
///	\author		TBA
///	\date		16 Jun 2020

#include <meshinfo.h>
#include <Geometry>
#include <HFExceptions.h>
#include <math.h>
#ifdef _WIN32
#include <corecrt_math_defines.h>
#endif
#include <iostream>
#include <robin_hood.h>

#define _USE_MATH_DEFINES

using std::array;
using std::vector;
namespace HF::Geometry {
	template <typename T>
	MeshInfo<T>::MeshInfo(const vector<array<T, 3>>& vertices, int id, std::string name)
	{
		// Throw if the input array has no values in it. 
		const size_t n = vertices.size();
		if (vertices.size() < 1) throw HF::Exceptions::InvalidOBJ(); // Doesn't have any valid geometry

		//AddVerts(vertices);
		VectorsToBuffers(vertices);

		// Throw if a NAN was placed into the mesh. 
		if (verts.hasNaN()) throw HF::Exceptions::InvalidOBJ();
	
		meshid = id;
		this->name = name;
	}

	template <typename T>
	inline void MeshInfo<T>::SetVert(int index, T x, T y, T z)
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


		\pre mapped_indexes and mapped_vertices are empty vectors of integers and Ts respectively.

		\post 1) mapped_indexes contains an array of indexes for mapped_vertices where
		every 3 indices represents a different triangle on the mesh.
		\post 2) mapped_vertices contains an array of Ts with each 3 vertices
		represent the x,y, and z coordinates for a different vertex in the mesh with 
		no repeats.
	*/
	template <typename T>
	void IndexRawVertices(
		const vector<array<T, 3>>& vertices,
		vector<int>& mapped_indexes,
		vector<T>& mapped_vertices
	) {
		// Create a hashmap to map vertices to indices.
		robin_hood::unordered_map <array<T, 3>, int> index_map;
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

	template <typename T>
	void MeshInfo<T>::VectorsToBuffers(const vector<array<T, 3>>& vertices)
	{
		// Create and fill vectors
		vector<int> mapped_indexes; vector<T> mapped_vertices;
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

	template <typename T>
	MeshInfo<T>::MeshInfo(
		const vector<T>& in_vertices,
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
		this->name = name;
	}

	template <typename T>
	void MeshInfo<T>::AddVerts(const vector<array<T, 3>>& in_vertices)
	{
		if (in_vertices.size() % 3 != 0) throw HF::Exceptions::InvalidOBJ(); // Incomplete triangle

		verts.resize(3, (static_cast<size_t>(verts.cols()) + in_vertices.size()));

		for (int i = 0; i < in_vertices.size(); i++) {
			auto& vertex = in_vertices[i];
			verts(0, i) = vertex[0];
			verts(1, i) = vertex[1];
			verts(2, i) = vertex[2];
		}
		if (verts.hasNaN()) throw std::runtime_error("Creation of mesh info failed");
	}

	template <typename T>
	int MeshInfo<T>::NumVerts() const { return static_cast<int>(verts.cols()); }

	template <typename T>
	int MeshInfo<T>::NumTris() const { return static_cast<int>(indices.cols()); }

	template <typename T>
	void MeshInfo<T>::ConvertToRhinoCoordinates()
	{
		Eigen::AngleAxis<T> yrot(0.5f * static_cast<T>(M_PI), Eigen::Vector3<T>::UnitX());
		Eigen::Quaternion<T> quat;
		quat = yrot;
		quat.normalize();
		verts = yrot.toRotationMatrix() * verts;
		//auto newer_matrix = new_matrix.eval();
		if (!verts.allFinite()) throw std::runtime_error("Verts has NAN");
	}

	template <typename T>
	void MeshInfo<T>::ConvertToOBJCoordinates()
	{
		Eigen::AngleAxis<T> yrot(-0.5f * static_cast<T>(M_PI), Eigen::Vector3<T>::UnitX());
		Eigen::Quaternion<T> quat;
		quat = yrot;
		quat.normalize();
		Eigen::Matrix3<T> rotation_matrix = yrot.toRotationMatrix();
		assert(!rotation_matrix.hasNaN());
		verts = (rotation_matrix * verts);
		assert(!verts.hasNaN());
	}

	template <typename T>
	void MeshInfo<T>::PerformRotation(T rx, T ry, T rz)
	{
		// Convert to radians
		T radian_ratio = static_cast<T>(M_PI) / 180.00f;
		rx *= radian_ratio; ry *= radian_ratio; rz *= radian_ratio;

		// Implementation based on 
		// https://stackoverflow.com/questions/21412169/creating-a-rotation-matrix-with-pitch-yaw-roll-using-eigen
		Eigen::AngleAxis<T> rollAngle(rz, Eigen::Vector3<T>::UnitZ());
		Eigen::AngleAxis<T> yawAngle(ry, Eigen::Vector3<T>::UnitY());
		Eigen::AngleAxis<T> pitchAngle(rx, Eigen::Vector3<T>::UnitX());


		// Create a quaternion from the angles, normalize it, then
		// convert it to a rotation matrix.
		Eigen::Quaternion<T> q = (rollAngle * yawAngle * pitchAngle);
		q.normalize();
		Eigen::Matrix3<T> rotation_matrix = q.toRotationMatrix();

		//! [snippet_objloader_assert]
		// Assert that we didn't create any NANS or infinite values
		assert(rotation_matrix.allFinite());

		// Apply the rotation matrix to verts
		verts = (rotation_matrix * verts);

		// Once again assert that we didn't create any nans or infinite numbers.
		assert(verts.allFinite());
		//! [snippet_objloader_assert]
	}

	template <typename T>
	int MeshInfo<T>::GetMeshID() const { return meshid; }

	template <typename T>
	vector<T> MeshInfo<T>::GetIndexedVertices() const
	{
		// Preallocate space for all vertices
		vector<T> out_array(verts.size());
		
		// Copy verts into it
		std::copy(verts.data(), verts.data() + verts.size(), out_array.begin());
		
		return out_array;
	}

	template <typename T>
	vector<int> MeshInfo<T>::getRawIndices() const
	{
		vector<int> out_array(indices.size());
		std::copy(indices.data(), indices.data() + indices.size(), out_array.begin());
		return out_array;
	}

	template <typename T>
	vector<std::array<T,3>> MeshInfo<T>::GetUnindexedVertices() const
	{
		// Preallocate an array 3x the size of the triangle matrix
		int tri_count = NumTris();
		vector<array<T, 3>>out_array(tri_count * 3);

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

	template <typename T>
	void MeshInfo<T>::SetMeshID(int new_id) { meshid = new_id; }

	template <typename T>
	array<T, 3> MeshInfo<T>::operator[](int i) const
	{
		// Throw if going beyond the array bounds
		if (i < 0 || i > NumVerts()) throw std::runtime_error("Out of range on index");
	
		// Create and return out array.
		array<T, 3> out_array;
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
	template <typename T>
	T arrayDist(const array<T, 3> from, const array<T, 3>& to) {
		return sqrtf(powf(from[0] - to[0], 2) + powf(from[1] - to[1], 2) + powf(from[2] - to[2], 2));
	}
	template <typename T>
	bool MeshInfo<T>::operator==(const MeshInfo& M2) const
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

	template <typename T>
	const array_and_size<int> MeshInfo<T>::GetIndexPointer() const {
		array_and_size<int> ret_array;

		ret_array.size = indices.size();
		ret_array.data = const_cast<int*>(indices.data());
		
		return ret_array;
	}

	template <typename T>
	const array_and_size<T> MeshInfo<T>::GetVertexPointer() const {
		array_and_size<T> ret_array;

		ret_array.size = verts.size();
		ret_array.data = const_cast<T*>(verts.data());

		return ret_array;
	}
}
template class HF::Geometry::MeshInfo<double>;

template class HF::Geometry::MeshInfo<float>;