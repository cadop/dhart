#include <vector>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace Geometry {
		class MeshInfo;
		enum GROUP_METHOD;
	}
}

/**
* @defgroup Geometry
* Read and manipulate meshes.
* @{
*/

/*! 
	\brief Load an obj from the given path then rotate it by x,y, and z
	
	\param obj_path Filepath to the obj file to load. 
	\param gm Method to use for dividing a single obj file into multiple meshes.
	\param xrot Degrees to rotate the mesh on the x axis.
	\param yrot Degrees to rotate the mesh on the y axis.
	\param zrot Degrees to rotate the mesh on the z axis.
	\param out_data_array Output parameter for a pointer to the new array of MeshInfo
	\param num_meshes Output parameter for size of `out_data_array`

	\returns `HF_STATUS::OK` If the input was a valid mesh and the function completed successfully
	\returns `HF_STATUS::GENERIC_ERROR` If the input was empty. 
	\returns `HF_STATUS::INVALID_OBJ` if the path didn't lead to a valid OBJ file
	\returns` HF_STATUS::NOT_FOUND` if the file at the given path couldn't be found.

	\attention
	Call DestroyMeshInfoPtrArray to deallocate out_data_array, and call DestroyMeshInfo on each
	instance of MeshInfo to deallocate them individually. Failure to do any of this will result
	in memory leaks. In the case that `HF_STATUS::OK` is not called, no memory is allocated,
	and as such no memory must be deallocated.
	
	\code
	\endcode
*/
C_INTERFACE LoadOBJ(
	const char* obj_path,
	HF::Geometry::GROUP_METHOD gm,
	float xrot,
	float yrot,
	float zrot,
	HF::Geometry::MeshInfo *** out_data_array,
	int * num_meshes
);

/*!
	\brief Store a mesh in a format usable with HumanFactors

	\param out_info Output parameter to contain the new instance of meshinfo
	\param indices An array of indices for the triangles in the mesh. Each integer should correspond to
				3 values in vertices, and every 3 integers should represent a complete triangle for the mesh. 
	\param vertices  Vertices of the mesh. Each 3 floats represent the X,Y, and Z of a point in space.
	\param num_vertices Length of the vertices array
	\param name Name of the mesh.
	\param ID Id of the mesh.

	\returns `HF_STATUS::OK` if the mesh was loaded successfully.
	\returns `HF_STATUS::INVALID_OBJ` if the given indices and vertices arrays didn't create a valid mesh. 

	\attention
	Call DestroyMeshInfo when done with this instance of meshinfo to deallocate it, otherwise you risk leaking memory!
	
	\code
	\endcode
*/
C_INTERFACE StoreMesh(
	HF::Geometry::MeshInfo ** out_info,
	const int* indices,
	int num_indices,
	const float* vertices,
	int num_vertices,
	const char* name,
	int id
);

/*!
	\brief Rotate a mesh.
	
	\param mesh_to_rotate A pointer to the mesh to rotate.
	\param xrot Rotation around the x axis in degrees.
	\param yrot Rotation around the y axis in degrees.
	\param zrot Rotation around the z axis in degrees.

	\returns `HF_STATUS::OK`
*/
C_INTERFACE RotateMesh(
	HF::Geometry::MeshInfo * mesh_to_rotate,
	float xrot,
	float yrot,
	float zrot
);


/*! 
	\brief Get a pointer to and the size of a mesh's triangle and vertex arrays
	
	\param index_out Output parameter for a pointer to the mesh's vertex array
	\param num_triangles Number of triangles in the index array. Every 3 indices is a new triangle, so this
						is equal to the length of `index_out` divided by 3.
	\param vertex_out Output parameter for a pointer to the mesh's index array.
	\param num_vertices Number of vertices in the vertex array. Every 3 floats is a new vertex, so this
						is equal to the length of `vertex_out` divided by 3.

	\returns HF_STATUS::OK

*/
C_INTERFACE GetVertsAndTris(
	const HF::Geometry::MeshInfo * MI,
	int** index_out,
	int* num_triangles,
	float** vertex_out,
	int* num_vertices
);

/*! 
	\brief Get the name of a mesh. 

	\param MI Instance of MeshInfo to get the name of
	\param out_name Pointer to the char array to write output to. This must be a valid pointer
	
	\returns `HF_STATUS::OK`
	
*/
C_INTERFACE GetMeshName(
	const HF::Geometry::MeshInfo* MI,
	char** out_name
);

/*!
	\brief Get the ID of a mesh

	\param MI Instance of MeshInfo to get the name of
	\param out_name Pointer to the char array to write output to. This must be a valid pointer

	\returns `HF_STATUS::OK`

*/
C_INTERFACE GetMeshID(
	const HF::Geometry::MeshInfo* MI,
	int * out_id
);


/// <summary>
/// Delete all instances of mesh info in the given vector
/// </summary>
/// <param name="mesh_to_destroy"> The mesh info to destroy. </param>

/*!
	\code
		// Requires #include "objloader_C.h", #include "meshinfo.h"

		// Prepare parameters for LoadOBJ

		// relative path begins where EXE file is located if file_path is not a path to a
		// valid OBJ file, HF::Exceptions::FileNotFound is thrown
		std::string file_path = "big_teapot.obj";

		const int obj_length = file_path.size();

		const float x_rot = 30;
		const float y_rot = 20;
		const float z_rot = 55;

		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		// Call LoadOBJ
		if (LoadOBJ(file_path.c_str(), obj_length, x_rot, y_rot, z_rot, &info)) {
			std::cout << "LoadOBJ successful" << std::endl;
		}
		else {
			std::cout << "LoadOBJ unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
	\endcode
*/
C_INTERFACE DestroyMeshInfo(HF::Geometry::MeshInfo * mesh_to_destroy);


C_INTERFACE DestroyMeshInfoPtrArray(HF::Geometry::MeshInfo** data_array);

/**@}*/

