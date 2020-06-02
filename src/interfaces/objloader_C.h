#include <vector>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace Geometry {
		class MeshInfo;
	}
}

/**
* @defgroup Geometry
* Read and manipulate meshes.
* @{
*/

/// <summary> Load an obj from the given path then rotate it by x,y, and z </summary>
/// <param name="obj_path"> Path to an obj file to load. </param>
/// <param name="length"> The number of characters int he path string </param>
/// <param name="xrot"> Degrees to rotate the mesh on the x axis. </param>
/// <param name="xrot"> Degrees to rotate the mesh on the y axis. </param>
/// <param name="xrot"> Degrees to rotate the mesh on the z axis. </param>
/// <returns>
/// HF_STATUS::GENERIC_ERROR If the input was empty. HF_STATUS::INVALID_OBJ if the path didn't lead
/// to a valid OBJ file, and HF_STATUS::NOT_FOUND if the file at the given path couldn't be found.
/// </returns>
C_INTERFACE LoadOBJ(
	const char* obj_path,
	int length,
	float xrot,
	float yrot,
	float zrot,
	std::vector<HF::Geometry::MeshInfo>** out_list
);

/// <summary>
/// Store a mesh from python as meshinfo
/// </summary>
/// <param name="indices">An array of indices for the triangles in the mesh. Each integer should correspond to 3 values in vertices, and every 3 integers should represent a complete triangle for the mesh. </param>
/// <param name="num_indices">Length of the indices array.</param>
/// <param name="vertices"> Vertices of the mesh. Each 3 floats represent the X,Y, and Z of a point in space.</param>
/// <param name="num_vertices">Length of the vertices array.</param>
/// <returns>HF_STATUS::OK if the mesh was loaded successfully. HF_STATUS::INVALID_OBJ if the given indices and vertices didn't create a valid mesh. </returns>
C_INTERFACE StoreMesh(
	std::vector<HF::Geometry::MeshInfo>** out_info,
	const int* indices,
	int num_indices,
	const float* vertices,
	int num_vertices,
	const char* name,
	int id
);

/// <summary>
/// Rotate an existing mesh
/// </summary>
/// <param name="xrot"> Rotation around the x axis in degrees.</param>
/// <param name="yrot"> Rotation around the y axis in degrees.</param>
/// <param name="zrot"> Rotation around the z axis in degrees.</param>
C_INTERFACE RotateMesh(
	std::vector<HF::Geometry::MeshInfo>* mesh_to_rotate,
	float xrot,
	float yrot,
	float zrot
);



/// <summary>
/// Delete all instaces of mesh info in the given vector
/// </summary>
/// <param name="mesh_to_destroy"> The mesh info to destroy. </param>
C_INTERFACE DestroyMeshInfo(std::vector<HF::Geometry::MeshInfo>* mesh_to_destroy);

/**@}*/

