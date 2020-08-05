/*!
	\file		objloader_C.h
	\brief		Header file for functions related to loading/storing an OBJ file (a mesh)

	\author		TBA
	\date		05 Aug 2020
*/

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

/*!
	\brief				Load an obj from the given path, then rotate it about the x, y, and z axes.

	\param	obj_path	File path to an .obj file, relative to the project's working directory
	\param	length		Character count of the obj_path string
	\param	xrot		Degrees to rotate the mesh about the x axis. 0.0f would mean no rotation about the x axis.
	\param	yrot		Degrees to rotate the mesh about the y axis. 0.0f would mean no rotation about the y axis.
	\param	zrot		Degrees to rotate the mesh about the z axis. 0.0f would mean no rotation about the z axis.

	\returns			HF_STATUS::OK, if the file described by obj_path was loaded successfully
						HF_STATUS::NOT_FOUND, if obj_path is an invalid/non-existent path
						HF_STATUS::INVALID_OBJ, if obj_path does not represent a valid .obj file
						HF_STATUS::GENERIC_ERROR, if the input described at obj_path was empty

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\objloader_cinterface.cpp snippet_LoadOBJ

	\remarks
		<b>Memory management:</b><br>
		info, a (vector<MeshInfo> *), points to nullptr upon initialization,<br>
		because the memory that it will address will be allocated by StoreMesh.<br>
		<br>
		Within its definition, LoadOBJ will allocate memory for info by receiving its address as a parameter,<br>
		a (vector<MeshInfo> **), and dereference it so that info will then point<br>
		to a block of memory large enough for a (vector<MeshInfo>).<br>
		<br>
		Therefore, it will be the caller's responsibilty to call DestroyMeshInfo(info) (call DestroyMeshInfo on info),<br>
		so that the memory that info points to can be released.<br>
		<br>
		<b>Assertion statement:</b><br>
		This function calls HF::Geometry::MeshInfo::PerformRotation,<br>
		which contains the following:<br>
		
		\snippet objloader\src\meshinfo.cpp snippet_objloader_assert

		rotation_matrix is a local variable in PerformRotation,<br>
		and verts is a private field within HF::Geometry::MeshInfo.<br>
		<br>
		These assertion statements may evaluate false (which will halting execution)<br>
		if NANs (not-a-number) or infinity values were created.<br>
*/
C_INTERFACE LoadOBJ(
	const char* obj_path,
	int length,
	float xrot,
	float yrot,
	float zrot,
	std::vector<HF::Geometry::MeshInfo>** out_list
);

/*!
	\brief		Store a mesh from Python as a container of MeshInfo - the operand container, out_info, will have one mesh inside it

	\param		out_info		Address of a vector<MeshInfo> *; the address of a pointer to a container of MeshInfo.
								StoreMesh will allocate memory for *(out_info), the caller must call DestroyMeshInfo
								on *(out_info) when finished with its resources.

	\param		indices			An array of indices for the triangles within the mesh.
								Each member in this array is an integer that corresponds to a set of 3 vertex coordinates, { x, y, z }.
								indices[0] represents { vertices[0], vertices[1], vertices[2] }
								indices[1] represents { vertices[3], vertices[4], vertices[5] }, etc.
								Every 3 array members in indices represents a complete triangle for the mesh.

	\param		num_indices		Member count (size) of the indices array. This value should be (size of vertices / 3) && (num_indices % 3 == 0)

	\param		vertices		An array of vertex coordinates, which represents the location of a mesh.
								Starting at i = 0, { vertices[i], vertices[i + 1], vertices[i + 2] } represents a point in space.

	\param		num_vertices	Member count (size) of the vertices array. This value should be (size of indices * 3) && (num_vertices % 3 == 0)
	\param		name			The desired name for the mesh (a human-readable identifier)
	\param		id				The desired ID for the mesh (an integral value)
	
	\returns					HF_STATUS::OK, if the mesh was loaded successfully
								HF_STATUS::INVALID_OBJ if the values in indices and/or vertices did not create a valid mesh

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>

	Functions called in the example below:<br>
		\link StoreMesh \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\objloader_cinterface.cpp snippet_StoreMesh

	\remarks
		<b>Memory management:</b><br>
		info, a (vector<MeshInfo> *), points to nullptr upon initialization,<br>
		because the memory that it will address will be allocated by StoreMesh.<br>
		<br>
		Within its definition, StoreMesh will allocate memory for info by receiving its address as a parameter,<br>
		a (vector<MeshInfo> **), and dereference it so that info will then point<br>
		to a block of memory large enough for a (vector<MeshInfo>).<br>
		<br>
		Therefore, it will be the caller's responsibilty to call DestroyMeshInfo(info) (call DestroyMeshInfo on info),<br>
		so that the memory that info points to can be released.
*/
C_INTERFACE StoreMesh(
	std::vector<HF::Geometry::MeshInfo>** out_info,
	const int* indices,
	int num_indices,
	const float* vertices,
	int num_vertices,
	const char* name,
	int id
);

/*!
	\brief			Rotate an existing mesh (MeshInfo)

	\param	xrot	Degrees to rotate the mesh about the x axis. 0.0f would mean no rotation about the x axis.
	\param	yrot	Degrees to rotate the mesh about the y axis. 0.0f would mean no rotation about the y axis.
	\param	zrot	Degrees to rotate the mesh about the z axis. 0.0f would mean no rotation about the z axis.

	\returns		HF_STATUS::OK on return

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>

	Functions called in the example below:<br>
		\link StoreMesh \endlink <br>
		\link RotateMesh \endlink <br>
		\link DestroyMeshinfo \endlink <br>

	\snippet tests\src\objloader_cinterface.cpp snippet_RotateMesh

	\remarks
		This function calls HF::Geometry::MeshInfo::PerformRotation,
		which contains the following:

		\snippet objloader\src\meshinfo.cpp snippet_objloader_assert

		rotation_matrix is a local variable in PerformRotation, and verts is a private field within HF::Geometry::MeshInfo.
		These assertion statements may evaluate false (which will halting execution) if NANs (not-a-number) or infinity values were created.
*/
C_INTERFACE RotateMesh(
	std::vector<HF::Geometry::MeshInfo>* mesh_to_rotate,
	float xrot,
	float yrot,
	float zrot
);

/*!
	\brief		Destroy the memory addressed by mesh_to_destroy, which was allocated by either LoadOBJ or StoreMesh
	
	\param		mesh_to_destroy		The vector<MeshInfo> * whose memory will be released

	\returns	HF_STATUS::OK on return

	Required forward-declarations for the example below:<br>
		\link HF::Geometry::MeshInfo \endlink <br>

	Functions called in the example below:<br>
		\link LoadOBJ \endlink <br>
		\link DestroyMeshInfo \endlink <br>

	\snippet tests\src\objloader_cinterface.cpp snippet_DestroyMeshInfo

	\remarks
		<b>Memory management:</b><br>
		For every call to LoadOBJ or StoreMesh<br>
		(which accepts the address of a vector<MeshInfo> * as a parameter),<br>
		a call to DestroyMeshInfo must accompany it<br>
		when the memory addressed by the vector<MeshInfo> * is no longer needed.
*/
C_INTERFACE DestroyMeshInfo(std::vector<HF::Geometry::MeshInfo>* mesh_to_destroy);

/**@}*/
