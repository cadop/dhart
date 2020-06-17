///
///	\file		meshinfo.h
///	\brief		Header file for MeshInfo
///
///	\author		TBA
///	\date		16 Jun 2020
///
#include <Dense>
#include <array>

//#define EIGEN_DONT_ALIGN_STATICALLY
//#define EIGEN_DONT_VECTORIZE

namespace std {
	/// <summary>
	/// Used to create a seed
	/// </summary>
	/// @tparam SizeT Integral type, such as size_t (implementation defined)

	/// \code{.cpp}
	///	std::size_t seed_in = 0;
	/// std::size_t value_in = VALUE_USER_DEFINED;	// user-defined value for seed
	///
	///	// seed_in is passed by reference
	///	std::array_hash_combine_impl<size_t>(seed_in, value_in);
	///
	///	// seed_in has now been given a value by array_hash_combine_impl
	/// \endcode
	template <typename SizeT>
	inline void array_hash_combine_impl(SizeT& seed, SizeT value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	/// <summary>
	/// Template specialization of std::hash, using std::array<float, 3> as type.
	/// </summary>
	template <>
	struct hash<std::array<float, 3>>
	{
		/// <summary>
		/// Returns a seed
		/// </summary>
		/// <param name="k">Reference to an array of coordinates (x, y, z)</param>
		/// <returns>A seed, using std::array_has_combine_impl</returns>

		/// \code{.cpp}
		/// // Create coordinates, (x, y, z)
		/// std::array<float, 3> arr = { 123.0, 456.1, 789.2 };
		///
		/// // Using the template specialization for std::hash (which takes a std::array<float, 3>),
		/// // we retrieve a seed
		/// std::size_t arr_hash = std::hash<std::array<float, 3>>(arr);
		/// \endcode
		inline std::size_t operator()(const std::array<float, 3>& k) const
		{
			size_t seed = std::hash<float>()(k[0]);
			array_hash_combine_impl(seed, std::hash<float>()(k[1]));
			array_hash_combine_impl(seed, std::hash<float>()(k[2]));
			return seed;
		}
	};
}

namespace HF {
	namespace Geometry {

		/// <summary>
		/// Holds vertices and some other information about a specific mesh
		/// </summary>
		class MeshInfo {
		private:
			int meshid;						///< Unique ID of this mesh in the Db ///<
			Eigen::Matrix3X<float> verts;	///< A Matrix of vertices for this mesh ///<
			Eigen::Matrix3X<int> indices;	///< A Matrix of triangle indices for this mesh ///<
			std::string name = "";			///< A name to help identify the mesh from other meshes ///<


		/// <summary>
		/// Set the specific coordinates at index to x, y, and z.
		/// </summary>
		/// <param name="index">Position in the array to assign the coordinate</param>
		/// <param name="x">X coordinate</param>
		/// <param name="y">Y coordinate</param>
		/// <param name="z">Z coordinate</param>

		/// \code{.cpp}
		/// // TODO: this is a private member function of MeshInfo, but it is not used by any other member function within MeshInfo.
		/// \endcode
			void SetVert(int index, float x, float y, float z);

			/// <summary>
			/// Insert an unindexed list of vertices into the triangle and vertex buffers using hashing. 
			/// </summary>
			/// <param name="vertices">An ordered list of vertices; every 3 vertices form a triangle in the mesh</param>

			/// \code{.cpp}
			/// // TODO: this is a private member function of MeshInfo, but it is not used by any other member function with MeshInfo.
			/// \endcode
			void VectorsToBuffers(const std::vector<std::array<float, 3>>& vertices);
		public:

			/// <summary>
			/// Default constructor, all fields initialized except for member indices
			/// </summary>

			/// \code{.cpp}
			/// MeshInfo mesh();	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"
			/// \endcode
			MeshInfo() { meshid = 0; verts.resize(3, 0); name = "INVALID"; };

			/// <summary>
			/// Construct a new MeshInfo object from an unindexed vector of vertices, an ID, and a name
			/// </summary>
			/// <param name="vertices">An ordered list of vertices, Every 3 vertices form a triangle in the mesh</param>
			/// <param name="id">The id of this mesh</param>
			/// <param name="name">The name of this mesh</param>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			/// // This mesh contains a triangle (due to having 3 vertices), id == 3451, name == "My Mesh"
			/// // Note that vertices is passed by reference.
			/// // Also, passing a std::vector<std::array<float, 3>> with (size < 1) will cause
			///	// HF::Exceptions::InvalidOBJ to be thrown.
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			/// \endcode
			MeshInfo(const std::vector<std::array<float, 3>>& vertices, int id, std::string name = "");

			/// <summary>
			/// Construct a new MeshInfo object from an indexed vector of vertices
			/// </summary>
			/// <param name="vertices">An ordered list of floats with each 3 floats representing a single vertex.</param>
			/// <param name="indexes">A vector of indexes with each index matching up with a vertex in vertices.</param>
			/// <param name="id">The id of this mesh</param>
			/// <param name="name">The name of this mesh</param>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
			///								//    0     1     2     3     4     5     6     7     8
			///
			///	// Prepare indices - each member in indices must be the index representing the initial coordinate within a vertex.
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices = { 0, 3, 6 };	
			///
			/// // Note that vertices and indices are passed by reference.
			/// // Also, passing a std::vector<float> with a size that is not a multiple of 3,
			///	// or passing a std::vector<int> with a size that is not a multiple of 3 --
			///	// will cause HF::Exceptions::InvalidOBJ to be thrown.
			/// // Also, indices.size() == (vertices.size() / 3), because all members of indices
			/// // must correspond to the index of the first member representing the initial coordinate within a vertex.
			/// MeshInfo mesh(vertices, indices, 5901, "This Mesh");
			/// \endcode
			MeshInfo(const std::vector<float>& in_vertices, const std::vector<int>& in_indexes, int id, std::string name = "");

			/// <summary>
			/// Append a list of verts onto the end of the mesh. This will cause the array to be resized.
			/// </summary>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create a vector of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			/// 
			/// // Create the MeshInfo instance - this example uses the no-arg constructor
			/// MeshInfo mesh();	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"
			///
			/// // Append the vertices to the mesh
			/// mesh.AddVerts(vertices);
			/// \endcode
			void AddVerts(const std::vector<std::array<float, 3>>& verts);

			/// <summary>
			/// Populate the mesh using only an array of floats and vertices corresponding to mesh
			/// x,y,z values and triangle indexes respectively.
			/// </summary>
			/// <param name="verts"></param>
			/// <param name="indices"></param>

			/// \code{.cpp}
			/// // TODO: needs implementation
			///
			/// // Prepare the vertices. Every three floats represents one vertex.
			///	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
			///								//    0     1     2     3     4     5     6     7     8
			///
			///	// Prepare indices - each member in indices must be the index representing the initial coordinate within a vertex.
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices = { 0, 3, 6 };	
			///
			/// // Create the MeshInfo instance - this example uses the no-arg constructor
			/// MeshInfo mesh();	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"
			///
			/// // Append the vertices to the mesh.
			/// mesh.AddVerts(vertices, indices);
			/// \endcode
			void AddVerts(const std::vector<float>& verts, std::vector<int>& indices);

			/// <summary>
			/// Get the number of vertices stored in this mesh info
			/// </summary>
			/// <returns>Total number of stored vertices</returns>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			///	// Create the MeshInfo
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			///
			/// int vert_count = 0;
			/// if ((vert_count = mesh.NumVerts()) == 0) {
			///		std::cout << "This mesh has no vertices." << std::endl;
			/// } else {
			///		std::cout << "Vertex count: " << std::endl;
			/// }
			///
			/// // Output is: 'Vertex count: 3'
			/// \endcode
			int NumVerts() const;

			/// <summary>
			/// Get the total number of triangles in the given mesh
			/// </summary>
			/// <returns>numVerts/3</returns>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			/// // Create the MeshInfo
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			///
			/// int tri_count = 0;
			/// if ((tri_count = mesh.NumTri()) == 0) {
			///		std::cout << "This mesh has no triangles." << std::endl;
			/// } else {
			///		std::cout << "Triangle count: " << tri_count << std::endl;
			/// }
			///
			/// // Output is: 'Triangle count: 1'
			/// // Note that (mesh.NumVerts() / 3 && mesh.NumTris()) will always be true,
			/// // since three vertices compose a single triangle.
			/// \endcode
			int NumTris() const;

			/// <summary>
			/// Convert the internal coordinates of this mesh to rhino coordinates
			/// </summary>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			/// // Create the MeshInfo
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			///
			/// // Convert the coordinates
			/// mesh.ConvertToRhinoCoordinates();
			/// \endcode
			void ConvertToRhinoCoordinates();

			/// <summary>
			/// Convert the internal coordinates of this mesh to OBJ coordinates
			/// </summary>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			/// // Create the MeshInfo
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			///
			/// // Convert the coordinates
			/// mesh.ConvertToOBJCoordinates();
			/// \endcode
			void ConvertToOBJCoordinates();

			/// <summary>
			/// Rotate this mesh by x, y, z rotations in degrees (pitch, yaw, roll)
			/// </summary>
			/// <param name="rx">Determines the pitch angle</param>
			/// <param name="ry">Determines the yaw angle</param>
			/// <param name="rz">Determines the roll angle</param>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			/// // Create the MeshInfo
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			///
			/// // Perform rotation
			/// float rot_x = 10.0;
			/// float rot_y = 20.0;
			/// float rot_z = 30.0;
			/// mesh.PerformRotation(rot_x, rot_y, rot_z);
			/// \endcode
			void PerformRotation(float rx, float ry, float rz);

			/// <summary>
			/// Return a copy of meshid
			/// </summary>
			/// <returns>The member field meshid, by value</returns>

			/// \code{.cpp}
			/// // Prepare the vertices
			/// std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
			/// std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
			/// std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };
			///
			/// // Create an array of vertices
			/// std::vector<std::array<float, 3>> vertices(vertex_0, vertex_1, vertex_2);
			///
			/// // Create the MeshInfo
			/// MeshInfo mesh(vertices, 3451, "My Mesh");
			///
			/// // Use GetMeshID to do an ID match
			/// int mesh_id = -1;
			/// if ((mesh_id = mesh.GetMeshID()) == 3451) {
			///		std::cout << "Retrieved 'My Mesh' << std::endl;
			/// }
			/// \endcode
			int GetMeshID() const;

			/// <summary>
			/// Retrieve a copy of the underlying vertex buffer. Helpful for filling other buffers
			/// </summary>
			/// <returns>A copy of the member field verts, the underlying vertex buffer (using std::copy)</returns>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
			///								//    0     1     2     3     4     5     6     7     8
			///
			///	// Prepare indices - each member in indices must be the index representing the initial coordinate within a vertex.
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices = { 0, 3, 6 };	
			///
			///	// Create the mesh.
			/// MeshInfo mesh(vertices, indices, 5901, "This Mesh");
			///
			///	// Retrieve copies of mesh's vertices.
			/// std::vector<float> vertices_copy_0 = mesh.getRawVertices();
			/// std::vector<float> vertices_copy_1 = mesh.getRawVertices();
			///
			///	// Uses std::vector<float>'s operator== to determine member equality
			/// if (vertices_copy_0 == vertices_copy_1) {
			///		std::cout << "vertices_copy_0 and vertices_copy_1 share the same elements/permutation." << std::endl;
			/// }
			///
			///	// This will demonstrate that vertices_copy_0 and vertices_copy_1 are different container instances
			/// if (&vertices_copy_0 != &vertices_copy_1) {
			///		std::cout << "vertices_copy_0 and vertices_copy_1 are different container instances." << std::endl;
			/// }
			///
			/// // Output:	vertices_copy_0 and vertices_copy_1 share the same elements/permutation.
			/// //			vertices_copy_0 and vertices_copy_1 are different container instances.
			/// \endcode
			std::vector<float> getRawVertices() const;
			
			/// <summary>
			/// Retrieve a copy of the underlying index buffer. Useful for copying into other buffers
			/// </summary>
			/// <returns>A copy of the member field indices, the underlying index buffer (using std::copy)</returns>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
			///								//    0     1     2     3     4     5     6     7     8
			///
			///	// Prepare indices - each member in indices must be the index representing the initial coordinate within a vertex.
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices = { 0, 3, 6 };	
			///
			///	// Create the mesh.
			/// MeshInfo mesh(vertices, indices, 5901, "This Mesh");
			///
			///	// Retrieve a copy of mesh's index vector
			/// std::vector<int> indices_copy_0 = mesh.getRawIndices();
			/// std::vector<int> indices_copy_1 = mesh.getRawIndices();
			///
			///	// Uses std::vector<int>'s operator== to determine member equality
			/// if (indices_copy_0 == indices_copy_1) {
			///		std::cout << "indices_copy_0 and indices_copy_1 share the same elements/permutation." << std::endl;
			/// }
			///
			///	// This will demonstrate that indices_copy_0 and indices_copy_1 are different container instances
			/// if (&indices_copy_0 != &indices_copy_1) {
			///		std::cout << "indices_copy_0 and indices_copy_1 are different container instances." << std::endl;
			/// }
			///
			/// // Output:	indices_copy_0 and indices_copy_1 share the same elements/permutation.
			/// //			indices_copy_0 and indices_copy_1 are different container instances.
			/// \endcode
			std::vector<int> getRawIndices() const;

			// TODO: Find a better solution for this.
			/// <summary>
			/// Retrieve vertices as a vector of arrays of float.
			/// </summary>
			/// <returns>A vector of arrays of float</returns>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
			///								//    0     1     2     3     4     5     6     7     8
			///
			///	// Prepare indices - each member in indices must be the index representing the initial coordinate within a vertex.
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices = { 0, 3, 6 };	
			///
			///	// Create the mesh.
			/// MeshInfo mesh(vertices, indices, 5901, "This Mesh");
			///
			/// // Retrieve vertices as a vector of coordinates (x, y, z)
			/// // Useful if your vertices were prepared from a one-dimensional container c, of float
			/// // (such that c.size() % 3 == 0)
			/// std::vector<std::array<float, 3>> vert_container = mesh.GetVertsAsArrays();
			/// \endcode
			std::vector<std::array<float, 3>> GetVertsAsArrays() const;

			/// <summary>
			/// Assigns the value of new_id to meshid
			/// </summary>
			/// <param name="new_id">The desired id to assign to member field meshid</param>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
			///								//    0     1     2     3     4     5     6     7     8
			///
			///	// Prepare indices - each member in indices must be the index representing the initial coordinate within a vertex.
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices = { 0, 3, 6 };	
			///
			///	// Create the mesh.
			/// MeshInfo mesh(vertices, indices, 5901, "This Mesh");
			///
			/// // Prepare a new mesh ID.
			/// int new_mesh_id = 9999;
			///
			/// // Assign new_mesh_id to mesh.
			/// mesh.SetMeshID(new_mesh_id);
			///
			/// // Test for value equality. Will return true.
			/// if (new_mesh_id == mesh.GetMeshID()) {
			///		std::cout << "ID assignment successful." << std::endl;
			/// }
			/// \endcode
			void SetMeshID(int new_id);

			/// <summary>
			/// Compare the vertices of two MeshInfo objects
			/// </summary>
			/// <param name="M2">The desired MeshInfo to compare</param>
			/// <returns>True if all vertices are equal within a certain tolerance, false otherwise</returns>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices_0 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
			///								//      0     1     2     3     4     5     6     7     8
			///
			/// std::vector<float> vertices_1 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
			///								//      0     1     2     3     4     5     6     7     8
			///
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices_0 = { 0, 3, 6 };	
			/// std::vector<int> indices_1 = { 0, 3, 6 };
			///
			/// MeshInfo mesh_0(vertices_0, indices_0, 5901, "This Mesh");
			/// MeshInfo mesh_1(vertices_1, indices_0, 4790, "That Mesh");
			///
			/// bool equivalent = mesh_0 == mesh_1;		// returns true
			///
			/// // operator== will determine if two MeshInfo are equal if
			///	// the Euclidean distance between each matching element in mesh_0 and mesh_1
			///	// is within 0.001. This means that for all i between vertices_0 and vertices_1 --
			///	// each x, y, z within
			///	//		vertices_0[i] and vertices_1[i]
			///	//	must be within 0.001 to be considered equivalent.
			///
			/// // Of course, this also means that if
			///	//		mesh_0.NumVerts() != mesh_1.NumVerts(),
			///	//	mesh_0 and mesh_1 are not equivalent at all.
			/// \endcode
			bool operator==(const MeshInfo& M2) const;
			
			/// <summary>
			/// Get vertex at a specific index in the mesh
			/// </summary>
			/// <param name="i">Index of the desired vertex</param>
			/// <returns>An array of floats representing the x, y, and z coordinates</returns>

			/// \code{.cpp}
			/// // Prepare the vertices. Every three floats represents one vertex.
			/// std::vector<float> vertices_0 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
			///								//      0     1     2     3     4     5     6     7     8
			///								//		0				  1					2
			///
			/// // indices[0] refers to vertices[0] (beginning of vertex 0)
			///	// indices[1] refers to vertices[3] (beginning of vertex 1)
			/// // indices[2] refers to vertices[6] (beginning of vertex 2)
			/// std::vector<int> indices_0 = { 0, 3, 6 };	
			///
			///	// Create the mesh
			/// MeshInfo mesh(vertices_0, indices_0, 5901, "This Mesh");
			///
			/// // Retrieve the desired vertex
			///	int index = 1;
			///
			///	// vertex consists of { 44.0, 55.0, 66.0 },
			///	// which is the second triplet of coordinates within vertices_0.
			///	std::vector<std::array<float, 3>> vertex = mesh[index];
			/// \endcode
			std::array<float, 3> operator[](int i) const;
		};
	}
}
