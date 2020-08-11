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

/*!
	\defgroup	Geometry
	Read and manipulate meshes
	
	\section mesh_setup Mesh setup
	First, we prepare the relative path to the <b>mesh</b>. (.obj file)

	\code
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());
	\endcode

	Then, we prepare a pointer to a vector<\link HF::Geometry::MeshInfo \endlink>.<br>

	\code
		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;
	\endcode

	We pass the address of this pointer to \link LoadOBJ \endlink .

	\code
		// Load mesh
		// The array rot will rotate the mesh 90 degrees with respect to the x-axis,
		// i.e. makes the mesh 'z-up'.
		//
		// Notice that we pass the address of the loaded_obj pointer
		// to LoadOBJ. We do not want to pass loaded_obj by value, but by address --
		// so that we can dereference it and assign it to the address of (pointer to)
		// the free store memory allocated within LoadOBJ.
		const float rot[] = { 90.0f, 0.0f, 0.0f };	// Y up to Z up
		status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}

		//
		// loaded_obj contains the mesh.
		//
	\endcode

	<b>loaded_obj</b> points to the loaded mesh.

	\section mesh_teardown Mesh teardown
	When you are finished with the <b>mesh</b>, you must then <b>relinquish</b> its memory resources:<br>

	\code
		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	\endcode

	<b>The client is responsible for releasing the memory for<br>
	the <b>mesh</b> (vector<\link HF::Geometry::MeshInfo \endlink> *).</b><br>
	Every example for each function should be followed up by the 'teardown' code described above.

	\section notes_perform_rotation Notes on MeshInfo::PerformRotation
		\link LoadOBJ \endlink and \link RotateMesh \endlink both call<br>
		HF::Geometry::MeshInfo::PerformRotation, which contains the following:<br>

		\snippet objloader\src\meshinfo.cpp snippet_objloader_assert

		<b>rotation_matrix</b> is a local variable in \link PerformRotation \endlink,<br>
		and <b>verts</b> is a private field within HF::Geometry::MeshInfo.<br>
		<br>
		These assertion statements may evaluate <b>false</b> (<b>which will halt execution</b>)<br>
		if NANs (not-a-number) or infinity values were created<br>
		from the <b>xrot</b>, <b>yrot</b>, or <b>zrot</b> values passed to \link LoadOBJ \endlink or \link RotateMesh \endlink.
	@{	
*/

/*!
	\brief				Load an obj from the given path, then rotate it about the x, y, and z axes.

	\param	obj_path	File path to an .obj file, relative to the project's working directory
	\param	length		Character count of the obj_path string
	\param	xrot		Degrees to rotate the mesh about the x axis. 0.0f would mean no rotation about the x axis.
	\param	yrot		Degrees to rotate the mesh about the y axis. 0.0f would mean no rotation about the y axis.
	\param	zrot		Degrees to rotate the mesh about the z axis. 0.0f would mean no rotation about the z axis.

	\param	out_list	Address of a (vector<\link HF::Geometry::MeshInfo \endlink> *); will be dereferenced and assigned the address of memory on the free store.
						When out_list is passed to this function, *(out_list) should point to nullptr.

	\returns			HF_STATUS::OK, if the file described by obj_path was loaded successfully
						HF_STATUS::NOT_FOUND, if obj_path is an invalid/non-existent path
						HF_STATUS::INVALID_OBJ, if obj_path does not represent a valid .obj file
						HF_STATUS::GENERIC_ERROR, if the input described at obj_path was empty

	\see \ref mesh_setup (how to create a mesh), \ref notes_perform_rotation (assertion statement)
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
	\brief		Store a mesh from Python as a container of \link HF::Geometry::MeshInfo \endlink - the operand container, out_info, will have one mesh inside it

	\param		out_info		Address of a vector<\link HF::Geometry::MeshInfo \endlink> *; the address of a pointer to a container of \link HF::Geometry::MeshInfo \endlink.
								\link StoreMesh \endlink will allocate memory for *(out_info), the caller must call \link DestroyMeshInfo \endlink
								on *(out_info) when finished with its resources. *(out_info) should point to nullptr
								when out_info is passed to this function.

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

	\see \ref mesh_teardown (how to destroy a mesh)

	First, we must prepare the values for the mesh to store.
	\snippet tests\src\objloader_cinterface.cpp snippet_StoreMesh_setup

	We are now ready to call <b>StoreMesh</b>.
	\snippet tests\src\objloader_cinterface.cpp snippet_StoreMesh_call

	`>>> The mesh was stored properly at address 0000029C6317BEB0.`\n
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
	\brief			Rotate an existing mesh (\link HF::Geometry::MeshInfo \endlink)

	\param	mesh_to_rotate	An operand vector<\link HF::Geometry::MeshInfo \endlink> * that addresses memory allocated by \link LoadOBJ \endlink or \link StoreMesh \endlink
	\param	xrot			Degrees to rotate the mesh about the x axis. 0.0f would mean no rotation about the x axis.
	\param	yrot			Degrees to rotate the mesh about the y axis. 0.0f would mean no rotation about the y axis.
	\param	zrot			Degrees to rotate the mesh about the z axis. 0.0f would mean no rotation about the z axis.

	\returns			HF_STATUS::OK on return

	\see \ref mesh_setup (how to create a mesh), StoreMesh (creating a mesh from arrays), \ref mesh_teardown (how to destroy a mesh), \ref notes_perform_rotation (assertion statement)

	\snippet tests\src\objloader_cinterface.cpp snippet_RotateMesh
*/
C_INTERFACE RotateMesh(
	std::vector<HF::Geometry::MeshInfo>* mesh_to_rotate,
	float xrot,
	float yrot,
	float zrot
);

/*!
	\brief		Destroy the memory addressed by mesh_to_destroy, which was allocated by either \link LoadOBJ \endlink or \link StoreMesh \endlink
	
	\param		mesh_to_destroy		The vector<\link HF::Geometry::MeshInfo \endlink> * whose memory will be released

	\returns	HF_STATUS::OK on return

	\see \ref mesh_setup (how to create a mesh), \ref mesh_teardown (how to destroy a mesh)
*/
C_INTERFACE DestroyMeshInfo(std::vector<HF::Geometry::MeshInfo>* mesh_to_destroy);

/**@}*/
