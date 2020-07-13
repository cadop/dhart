///
///	\file		meshinfo.h
/// \brief		Contains definitions for the <see cref = "HF::SpatialStructures::MeshInfo">MeshInfo</cref> class
///
///	\author		TBA
///	\date		16 Jun 2020

#include <Dense>
#include <array>

//#define EIGEN_DONT_ALIGN_STATICALLY
//#define EIGEN_DONT_VECTORIZE

namespace std {
	/// <summary> Combine the hash of value into seed. </summary>
	/// @tparam SizeT Integral type, such as size_t (implementation defined)

	/*!
		\code
			// be sure to #include "meshinfo.h"

			std::size_t seed_in = 0;
			std::size_t value_in = VALUE_USER_DEFINED;	// user-defined value for seed
	
			// seed_in is passed by reference
			std::array_hash_combine_impl<size_t>(seed_in, value_in);
	
			// seed_in has now been given a value by array_hash_combine_impl
		\endcode
	*/
	template <typename SizeT>
	inline void array_hash_combine_impl(SizeT& seed, SizeT value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	/// <summary> Template specialization of std::hash for using std::array<float, 3>. </summary>
	template <>
	struct hash<std::array<float, 3>>
	{
		/// <summary> Calculate a hash for k. </summary>
		/// <param name="k"> Reference to an array of coordinates (x, y, z) </param>
		/// <returns> The hashed value of k. </returns>
		/*!
			\details Combines the hash of x,y,and z.
			\code
				// be sure to #include "meshinfo.h"

				// Create coordinates, (x, y, z)
				std::array<float, 3> arr = { 123.0, 456.1, 789.2 };
		
				// Using the template specialization for std::hash (which takes a std::array<float,
				// 3>), we retrieve a seed
				std::size_t arr_hash = std::hash<std::array<float, 3>>(arr);
			\endcode
		*/
		inline std::size_t operator()(const std::array<float, 3>& k) const
		{
			size_t seed = std::hash<float>()(k[0]);
			array_hash_combine_impl(seed, std::hash<float>()(k[1]));
			array_hash_combine_impl(seed, std::hash<float>()(k[2]));
			return seed;
		}
	};
}

namespace HF::Geometry {
	/*!
		\brief A collection of vertices and indices representing geometry.
		
		\details 
		Internally stored as a 3 by X matrix of vertices and a 3 by X matrix for indices. 
		Eigen is used to manage all matricies and perform quick transformations such
		as RotateMesh. More details on Eigen are available here:
		https://eigen.tuxfamily.org/dox/group__Geometry__Module.html

		\invariant
			Will always hold a valid mesh with finite members.


		\image html https://upload.wikimedia.org/wikipedia/commons/2/2d/Mesh_fv.jpg "MeshInfo"
	*/
	class MeshInfo {
	private:
		int meshid;						///< Identifier for this mesh.
		Eigen::Matrix3X<float> verts;	///< 3 by X matrix of vertices
		Eigen::Matrix3X<int> indices;	///< 3 by X matrix of indices for triangles.
		std::string name = "";			///< A human-readable title. 

		/// <summary> Change the position of the vertex at index. </summary>
		/// <param name="index"> Index of the vertex to change, </param>
		/// <param name="x"> X coordinate </param>
		/// <param name="y"> Y coordinate </param>
		/// <param name="z"> Z coordinate </param>
		/*!
			\code
				// TODO: this is a private member function of MeshInfo, but it is not used by any other member function within MeshInfo
			\endcode
		*/
		void SetVert(int index, float x, float y, float z);

		/// <summary>
		/// Index vertices then insert them into verts and indices.
		/// </summary>
		/// <param name="vertices">
		/// An ordered list of vertices; every 3 vertices form a triangle in the mesh.
		/// </param>
		/*!
			\details Calls IndexRawVertices to index the input vertices then fill verts and indices.
			
			\post
			This class's verts and indices are filled with the indexed vertices from vertices.

			\exception HF::Exceptions::InvalidOBJ vertices did not contain the vertices of a valid mesh.

			\code
				// TODO: this is a private member function of MeshInfo, but it is not used by any other member function with MeshInfo.
			\endcode
		*/
		void VectorsToBuffers(const std::vector<std::array<float, 3>>& vertices);
	public:

		/// <summary> Construct an empty instance of MeshInfo. </summary>

		/*!
			\code
				// be sure to #include "meshinfo.h"

				HF::Geometry::MeshInfo mesh;	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"
			\endcode
		*/
		MeshInfo() { meshid = 0; verts.resize(3, 0); name = "INVALID"; };

		/// <summary>
		/// Construct a new MeshInfo object from an unindexed vector of vertices, an ID, and a name.
		/// </summary>
		/// <param name="vertices">
		/// An ordered list of vertices where every 3 vertices comprise single a triangle in the mesh.
		/// </param>
		/// <param name="id"> A unique identifier. </param>
		/// <param name="name"> A human-readable title. </param>
		/*!
			\exception HF::Exceptions::InvalidOBJ the input vertices don't represent a valid mesh.
			\exception std::exception the input array had one or more NAN values.
			
			\todo Change the std::exception to also be an InvalidOBJ exception. 
			\todo Check for a number of vertices that aren't a multiple of 3 like the other functions.
			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

				// This mesh contains a triangle (due to having 3 vertices), id == 3451, name ==
				// "My Mesh" Note that vertices is passed by reference. Also, passing a
				// std::vector<std::array<float, 3>> with (size < 1) will cause
				// HF::Exceptions::InvalidOBJ to be thrown.
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

				// Display the vertices for mesh
				std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
				for (auto vertex : mesh.GetVertsAsArrays()) {
					std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
				}				
			\endcode
		*/
		MeshInfo(
			const std::vector<std::array<float, 3>>& vertices,
			int id,
			std::string name = ""
		);

		/// <summary> Construct a new MeshInfo object from an indexed vector of vertices. </summary>
		/// <param name="vertices">
		/// An ordered list of floats with every 3 floats representing the x,y, and z coordinates for a single vertex.
		/// </param>
		/// <param name="indexes">
		/// A vector of indices with each index matching up with a vertex in vertices.
		/// </param>
		/// <param name="id"> A unique identifier. </param>
		/// <param name="name"> A human readable title. </param>
		/*!
			\exception HF::Exceptions::InvalidOBJ input indices and vertices don't
			represent a valid mesh.

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
											// 0 1 2

				// If indices.size() == 3, this means that vertices.size() == 9, and each member
				// of indices represents a std::array<float, 3> i.e. indices[0] represents the
				// 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5} indices[1] represents
				// the 1st set of (x, y, z) coordinates, {23.5, 85.7, 45.2} indices[2]
				// represents the 2nd set of (x, y, z) coordinates, {12.0, 24.6, 99.4}
				std::vector<int> indices = { 0, 1, 2 };

				// Note that vertices and indices are passed by reference. Also, passing a
				// std::vector<float> with a size that is not a multiple of 3, or passing a
				// std::vector<int> with a size that is not a multiple of 3 -- will cause
				// HF::Exceptions::InvalidOBJ to be thrown. Also, indices.size() ==
				// (vertices.size() / 3), because all members of indices must correspond to the
				// index of the first member representing the initial coordinate within a vertex.
				HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

				// Display the vertices for mesh
				std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
				for (auto vertex : mesh.GetVertsAsArrays()) {
					std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
				}
			\endcode
		*/
		MeshInfo(
			const std::vector<float>& in_vertices, 
			const std::vector<int>& in_indexes,
			int id, 
			std::string name = ""
		);

		/*!
			\brief Add more vertices to this mesh. 
			
			\param verts Vertices to add to this mesh, with each 3 belonging to a new triangle. 

			\details
				Index the vertices in verts, then insert the new indices and vertices into their respective arrays.
			
			\remarks
				This function can effectively be used to "Merge" another mesh into this one.
			
			\exception HF::Exceptions::InvalidOBJ in_vertices did not contain a valid set of triangles.

			\todo Change std::exception to HumanFactors::InvalidOBJ. 
			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

				// Create a vector of vertices
				std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

				// Create the MeshInfo instance - this example uses the no-arg constructor
				HF::Geometry::MeshInfo mesh;	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"

				// Append the vertices to the mesh
				mesh.AddVerts(vertices);

				std::cout << "size: " << mesh.getRawVertices().size() << std::endl;

				// Display the vertices for mesh
				std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
				for (auto vertex : mesh.GetVertsAsArrays()) {
					std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
				}
			\endcode
		*/
		void AddVerts(const std::vector<std::array<float, 3>>& verts);
	
		/// <summary> Determine how many vertices are in this mesh. </summary>
		/// <returns> Total number of vertices for this mesh. </returns>
		/*!
			\details The number of vertices in the mesh is equal to the number of
			columns in the verts matrix. 


			\remarks Useful for testing correctness. 

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

				// Create the MeshInfo
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

				int vert_count = 0;
				if ((vert_count = mesh.NumVerts()) == 0) {
					std::cout << "This mesh has no vertices." << std::endl;
				} else {
					std::cout << "Vertex count: " << vert_count << std::endl;
				}

				// Output is: 'Vertex count: 3'
			\endcode
		*/
		int NumVerts() const;

		/// <summary> Calculate the total number of triangles in this mesh.</summary>
		/// <returns> The number of triangles in this mesh. </returns>
		/*!
			\details The number of triangles in the mesh is equal to the number of
			columns in the indices matrix.

			\remarks 
			Used to equal num_verts/3, but now that does not since indexed buffers are used.

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
		
				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
		
				// Create the MeshInfo
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");
		
				int tri_count = 0;
				if ((tri_count = mesh.NumTris()) == 0) {
					std::cout << "This mesh has no triangles." << std::endl;
				} else {
					std::cout << "Triangle count: " << tri_count << std::endl;
				}
		
				// Output is: 'Triangle count: 1' Note that (mesh.NumVerts() / 3 &&
				// mesh.NumTris()) will always be true, since three vertices compose a single triangle.
			\endcode
		*/
		int NumTris() const;

		/*!
			\brief Convert a mesh from Y-Up to Z-Up

			\todo Rename this to ConvertToZUp.
			\todo Just use the RotateMesh function.
			\todo Change exception to an assertion since it shouldn't happen in production code. 

			\see ConvertToOBJCoordinates for the inverse operation.
			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

				// Create the MeshInfo
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

				// Convert the coordinates
				mesh.ConvertToRhinoCoordinates();

				// Note: If vertices contain NaN or +/- infinity values, std::exception is thrown

				// Display the vertices for mesh
				std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
				for (auto vertex : mesh.GetVertsAsArrays()) {
					std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
				}
			\endcode
		*/
		void ConvertToRhinoCoordinates();

		/*!
			\brief Convert a mesh from Z-Up to Y-Up

			\todo Rename this to ConvertToYup.
			\todo Just use the RotateMesh function.
			
			\see ConvertToRhinoCoordinates for the inverse operation.
			
			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

				// Create the MeshInfo
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

				// Convert the coordinates
				mesh.ConvertToOBJCoordinates();

				// Note: Program will abort if any coordinate is NaN

				// Display the vertices for mesh
				std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
				for (auto vertex : mesh.GetVertsAsArrays()) {
					std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
				}
			\endcode

		*/
		void ConvertToOBJCoordinates();

		/// <summary> Rotate this mesh by x, y, z rotations in degrees (pitch, yaw, roll). </summary>
		/// <param name="rx"> Pitch to rotate by in degrees. </param>
		/// <param name="ry"> Yaw to rotate by in degrees. </param>
		/// <param name="rz"> Roll to rotate by in degrees. </param>
		/*
			\details 
			Uses Eigen's geometry module to create a rotation matrix, then applies it to 
			this mesh's vertex matrix. 

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

				// Create the MeshInfo
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

				// Perform rotation
				float rot_x = 10.0;
				float rot_y = 20.0;
				float rot_z = 30.0;

				mesh.PerformRotation(rot_x, rot_y, rot_z);

				// Display the vertices for mesh
				std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
				for (auto vertex : mesh.GetVertsAsArrays()) {
					std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
				}
			\endcode
		*/
		void PerformRotation(float rx, float ry, float rz);

		/// <summary> Get the ID of this mesh. </summary>
		/// <returns> The member field meshid, by value. </returns>
		/*!
			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices
				std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
				std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
				std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
		
				// Create an array of vertices
				std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
		
				// Create the MeshInfo
				HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");
		
				// Use GetMeshID to do an ID match
				int mesh_id = -1;
				if ((mesh_id = mesh.GetMeshID()) == 3451) {
					std::cout << "Retrieved 'My Mesh'" << std::endl;
				}
			\endcode
		*/
		int GetMeshID() const;


		/// <returns>
		/// A copy of every vertex in this array.
		/// </returns>
		/*!
			
			\brief Copy this mesh's vertices as a 1D array.

			\details
			The vertices are copied directly from this mesh's underlying eigen matrix. The index of each vertex
			matches the index used for this mesh's index array. For Example the first element is the vertex at index 0, 
			the second is the vertex for index 1, etc.

			\todo
			Make a version of this function that just returns a const reference or pointer to the data. 
			There's not much reason	to do a copy here. 

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
											// 0 1 2

				// If indices.size() == 3, this means that vertices.size() == 9, and each member
				// of indices represents a std::array<float, 3> i.e. indices[0] represents the
				// 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5} indices[1] represents
				// the 1st set of (x, y, z) coordinates, {23.5, 85.7, 45.2} indices[2]
				// represents the 2nd set of (x, y, z) coordinates, {12.0, 24.6, 99.4}
				std::vector<int> indices = { 0, 1, 2 };

				// Create the mesh.
				HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

				// Retrieve copies of mesh's vertices.
				std::vector<float> vertices_copy_0 = mesh.getRawVertices();
				std::vector<float> vertices_copy_1 = mesh.getRawVertices();

				// Uses std::vector<float>'s operator== to determine member equality
				if (vertices_copy_0 == vertices_copy_1) {
					std::cout << "vertices_copy_0 and vertices_copy_1 share the same elements/permutation." << std::endl;
				}

				// This will demonstrate that vertices_copy_0 and vertices_copy_1 are different
				// container instances
				// Output:	vertices_copy_0 and vertices_copy_1 share the same elements/permutation.
				// vertices_copy_0 and vertices_copy_1 are different container instances.
				if (&vertices_copy_0 != &vertices_copy_1) {
					std::cout << "vertices_copy_0 and vertices_copy_1 are different container instances." << std::endl;
				}

				// Output all coordinates
				const int total = mesh.NumTris() * 3;
				int offset = 0;

				for (int i = 0; i < total; i++) {
					auto y_offset = static_cast<int64_t>(offset) + 1;
					auto z_offset = static_cast<int64_t>(offset) + 2;

					std::cout << "(" << vertices_copy_0[offset] << ", "
						<< vertices_copy_0[y_offset] << ", "
						<< vertices_copy_0[z_offset] << ")" << std::endl;
						offset += 3;
				}
			\endcode
		*/
		std::vector<float> GetIndexedVertices() const;
		
		/// <summary>
		/// Retrieve a copy of this mesh's index buffer as a 1D array.
		/// </summary>
		/// <returns>
		/// A copy of this mesh's index buffer containing every index in this mesh.
		/// </returns>
		/*!

			\todo
			Version of this function that just returns a const reference to the data.
			There's not much reason	to do a copy here.

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
										  // 0 1 2

				// If indices.size() == 3, this means that vertices.size() == 9, and each member
				// of indices represents a std::array<float, 3> i.e. indices[0] represents the
				// 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5} indices[1] represents
				// the 1st set of (x, y, z) coordinates, {23.5, 85.7, 45.2} indices[2]
				// represents the 2nd set of (x, y, z) coordinates, {12.0, 24.6, 99.4}
				std::vector<int> indices = { 0, 1, 2 };

				// Create the mesh.
				HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

				// Retrieve a copy of mesh's index vector
				std::vector<int> indices_copy_0 = mesh.getRawIndices();
				std::vector<int> indices_copy_1 = mesh.getRawIndices();

				// Uses std::vector<int>'s operator== to determine member equality
				if (indices_copy_0 == indices_copy_1) {
					std::cout << "indices_copy_0 and indices_copy_1 share the same elements/permutation." << std::endl;
				}

				// This will demonstrate that indices_copy_0 and indices_copy_1 are different
				// container instances
				// Output:	indices_copy_0 and indices_copy_1 share the same elements/permutation.
				// indices_copy_0 and indices_copy_1 are different container instances.
				if (&indices_copy_0 != &indices_copy_1) {
					std::cout << "indices_copy_0 and indices_copy_1 are different container instances." << std::endl;
				}

				// Output the indices
				for (auto i : indices_copy_0) {
					std::cout << i << std::endl;
				}
			\endcode
		*/
		std::vector<int> getRawIndices() const;

		/// <summary> Retrieve an unindexed array of this mesh's vertices. </summary>
		/*!

			\returns 
			An array of vertices to match this mesh's array of indices.

			\details
			For every index in this mesh's indices array, copy the matching vertex into the return array.
			This will result in vertexes being repeated multiple times. For example, the first element
			is the vertex pointed to by the first index in indices, the second element is the  vertex
			pointed to by the second index in indices etc.

			\remarks
			The output of this function should match the input for MeshInfo() to create this
			mesh. This function is not really useful outside of testing, since it's essentially
			uncompressing the mesh and repeating a lot of data. 

			\see getRawVertices() for a more efficent way of reading this mesh's vertices. 
			\see getRawIndices() for an ordered list of indices for each vertex in the return array.

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
										  // 0 1 2

				// If indices.size() == 3, this means that vertices.size() == 9, and each member
				// of indices represents a std::array<float, 3> i.e. indices[0] represents the
				// 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5} indices[1] represents
				// the 1st set of (x, y, z) coordinates, {23.5, 85.7, 45.2} indices[2]
				// represents the 2nd set of (x, y, z) coordinates, {12.0, 24.6, 99.4}
				std::vector<int> indices = { 0, 1, 2 };
		
				// Create the mesh.
				HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");
		
				// Retrieve vertices as a vector of coordinates (x, y, z) Useful if your
				// vertices were prepared from a one-dimensional container c, of float (such
				// that c.size() % 3 == 0)
				std::vector<std::array<float, 3>> vert_container = mesh.GetVertsAsArrays();
			\endcode
		*/
		std::vector<std::array<float, 3>> GetUnindexedVertices() const;

		/// <summary>Change the ID of this mesh.</summary>
		/// <param name="new_id"> New ID to assign to this mesh. </param>
		/*!
			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
										 // 0 1 2

				// If indices.size() == 3, this means that vertices.size() == 9, and each member
				// of indices represents a std::array<float, 3> i.e. indices[0] represents the
				// 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5} indices[1] represents
				// the 1st set of (x, y, z) coordinates, {23.5, 85.7, 45.2} indices[2]
				// represents the 2nd set of (x, y, z) coordinates, {12.0, 24.6, 99.4}
				std::vector<int> indices = { 0, 1, 2 };
		
				// Create the mesh.
				HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");
		
				// Prepare a new mesh ID.
				int new_mesh_id = 9999;
		
				// Assign new_mesh_id to mesh.
				mesh.SetMeshID(new_mesh_id);
		
				// Test for value equality. Will return true.
				if (new_mesh_id == mesh.GetMeshID()) {
					std::cout << "ID assignment successful." << std::endl;
				}
			\endcode
		*/
		void SetMeshID(int new_id);

		/// <summary> Compare the vertices of two MeshInfo objects. </summary>
		/// <param name="M2"> The desired MeshInfo to compare. </param>
		/// <returns> True if all vertices are equal within a certain tolerance, false otherwise. </returns>
		/*!
			\details
			First check that the size of both mesh's vertex arrays are the same size, then
			compare the distance between every vertex one by one. 

			\warning 
			This does not compare indices at all and will return false if both meshes
			contain the same vertices but in a different order. 

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices_0 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
											// 0 1 2
		
				std::vector<float> vertices_1 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
											// 0 1 2
		
				// If indices_0.size() == 3, this means that vertices_0.size() == 9, and each
				// member of indices represents a std::array<float, 3> i.e. indices[0]
				// represents the 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5}
				// indices[1] represents the 1st set of (x, y, z) coordinates, {23.5, 85.7,
				// 45.2} indices[2] represents the 2nd set of (x, y, z) coordinates, {12.0,
				// 24.6, 99.4}
				//
				// The same is true for vertices_1 and indices_1.
				std::vector<int> indices_0 = { 0, 1, 2 };
				std::vector<int> indices_1 = { 0, 1, 2 };

				HF::Geometry::MeshInfo mesh_0(vertices_0, indices_0, 5901, "This Mesh");
				HF::Geometry::MeshInfo mesh_1(vertices_1, indices_0, 4790, "That Mesh");
		
				bool equivalent = mesh_0 == mesh_1;		// returns true
		
				// operator== will determine if two MeshInfo are equal if the Euclidean distance
				// between each matching element in mesh_0 and mesh_1 is within 0.001. This
				// means that for all i between vertices_0 and vertices_1 -- each x, y, z within
				// vertices_0[i] and vertices_1[i] must be within 0.001 to be considered equivalent.
		
				// Of course, this also means that if mesh_0.NumVerts() != mesh_1.NumVerts(),
				// mesh_0 and mesh_1 are not equivalent at all.
			\endcode
		*/
		bool operator==(const MeshInfo& M2) const;
		
		/// <summary> Get vertex at a specific index in the mesh. </summary>
		/// <param name="i"> Index of the vertex to retrieve. </param>
		/// <returns> The x, y, and z coordinates for the vertex at index i. </returns>
		/*!

			\exception std::exception i is out of the bounds of the vertex array.

			\code
				// be sure to #include "meshinfo.h", and #include <vector>

				// Prepare the vertices. Every three floats represents one vertex.
				std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
										 // 0 1 2

				// If indices.size() == 3, this means that vertices.size() == 9, and each member
				// of indices represents a std::array<float, 3> i.e. indices[0] represents the
				// 0th set of (x, y, z) coordinates, {34.1, 63.9, 16.5} indices[1] represents
				// the 1st set of (x, y, z) coordinates, {23.5, 85.7, 45.2} indices[2]
				// represents the 2nd set of (x, y, z) coordinates, {12.0, 24.6, 99.4}
				std::vector<int> indices = { 0, 1, 2 };
		
				// Create the mesh
				HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");
		
				// Retrieve the desired vertex
				int index = 1;
		
				// vertex consists of { 44.0, 55.0, 66.0 }, which is the second triplet of
				// coordinates within vertices_0.
				auto vertex = mesh[index];

				std::cout << "Retrieved at index << " << index << ": (" << vertex[0] << ", " 
				<< vertex[1] << ", " 
				<< vertex[2] << ")" << std::endl;
			\endcode
		*/
		std::array<float, 3> operator[](int i) const;
	};
}
