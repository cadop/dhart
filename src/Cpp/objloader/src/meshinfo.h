
#include <Dense>
#include <array>
//#define EIGEN_DONT_ALIGN_STATICALLY
//#define EIGEN_DONT_VECTORIZE

namespace std {
	template <typename SizeT>
	inline void array_hash_combine_impl(SizeT& seed, SizeT value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <>
	struct hash<std::array<float, 3>>
	{
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
			int meshid; // Unique ID of this mesh in the Db
			Eigen::Matrix3X<float> verts; // A Matrix of vertices for this mesh
			Eigen::Matrix3X<int> indices; // A Matrix of triangle indexes for this mesh
			std::string name = ""; // A name to help identify the mesh from other meshes

		
		/// <summary>
		/// Set the specific coordinates at index to x, y, and z. 
		/// </summary>
		/// <param name="index">Place in the array to set the coordinate</param>
		/// <param name="x"> X coordinate</param>
		/// <param name="y"> Y coordinate</param>
		/// <param name="z"> Z coordinate</param>
		void SetVert(int index, float x, float y, float z);

		/// <summary>
		/// Insert an unindexed list of vertices into the triangle and vertex buffers using hashing. 
		/// </summary>
		/// <param name="vertices">An ordered list of vertices, Every 3 vertices form a triangle in the mesh</param>
		void VectorsToBuffers(const std::vector<std::array<float,3>> & vertices);
		public:

			MeshInfo() { meshid = 0; verts.resize(3, 0); name = "INVALID"; };
			/// <summary>
			/// Construct a new MeshInfo object from an unindexed vector of vertices, an ID, and a name
			/// </summary>
			/// <param name="vertices">An ordered list of vertices, Every 3 vertices form a triangle in the mesh</param>
			/// <param name="id">The id of this mesh</param>
			/// <param name="name">The name of this mesh</param>
			MeshInfo(const std::vector<std::array<float, 3>>& vertices, int id, std::string name = "");
			
			/// <summary>
			/// Construct a new MeshInfo object from an indexed vector of vertices
			/// </summary>
			/// <param name="vertices">An ordered list of floats with each 3 floats representing a single vertex.</param>
			/// <param name="indexes">A vector of indexes with each index matching up with a vertex in vertices.</param>
			/// <param name="id">The id of this mesh</param>
			/// <param name="name">The name of this mesh</param>
			MeshInfo(const std::vector<float>& in_vertices, const std::vector<int>& in_indexes,  int id, std::string name = "");


			/// <summary>
			/// Append a list of verts onto the end of the mesh. This will cause the array to be resized.
			/// </summary>
			void AddVerts(const std::vector<std::array<float, 3>>& verts);
			
			/// <summary>
			/// Populate the mesh using only an array of floats and vertices corresponding to mesh
			/// x,y,z values and triangle indexes respectively.
			/// </summary>
			/// <param name="verts"></param>
			/// <param name="indices"></param>
			void AddVerts(const std::vector<float> & verts, std::vector<int> & indices);

			/// <summary>
			/// Get the number of vertices stored in this mesh info
			/// </summary>
			/// <returns>Total number of stored vertices</returns>
			int NumVerts() const;

			/// <summary>
			/// Get the total number of triangles in the given mesh
			/// </summary>
			/// <returns>numVerts/3</returns>
			int NumTris() const;

			/// <summary>
			/// Convert the internal coordinates of this mesh to rhino coordinates
			/// </summary>
			void ConvertToRhinoCoordinates();

			/// <summary>
			/// Convert the internal coordinates of this mesh to OBJ coordinates
			/// </summary>
			void ConvertToOBJCoordinates();

			/// <summary>
			/// Rotate this mesh by x,y,z rotations in degrees.
			/// </summary>
			void PerformRotation(float rx, float ry, float rz);

			/// <summary>
			/// Return a copy of meshID
			/// </summary>
			int GetMeshID() const;


			/// <summary>
			/// Get a copy of the underlying vertex buffer. Helpful for filling other bufferss
			/// </summary>
			std::vector<float> getRawVertices() const;
			
			/// /// <summary>
			/// Get a copy of the underlying index buffer. Useful for copying into other buffers
			/// </summary>
			std::vector<int> getRawIndices() const;

			


			//TODO: Find a better solution for this.
			/// <summary>
			///  Get verticies as a vector of floats. 
			/// </summary>
			/// <returns></returns>
			std::vector<std::array<float, 3>> GetVertsAsArrays() const;
			void SetMeshID(int new_id);

			/// <summary>
			/// Compare the vertices of two mesh info objects
			/// </summary>
			/// <param name="M2">Mesh info to compare with</param>
			/// <returns>True if all vertices are equal within a certain tolerance</returns>
			bool operator==(const MeshInfo& M2) const;
			
			/// /// <summary>
			/// Get a vertice at a specific vertex in the mesh
			/// </summary>
			/// <param name="i">Index of the desired vertice</param>
			/// <returns>An array of floats for the x,y, and z coordinates</returns>
			std::array<float, 3> operator[](int i) const;

		};
	}
}
