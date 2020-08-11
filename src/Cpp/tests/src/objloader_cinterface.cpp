/*!
	\file		objloader_cinterface.cpp
	\brief		Unit test source file for testing the examples for objloader_C documentation

	\author		Gem Aludino
	\date		05 Aug 2020
*/

#include "gtest/gtest.h"
#include "objloader_C.h"

#include <filesystem>

namespace CInterfaceTests {
	TEST(_objloader_cinterface, LoadOBJ) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		//
		// gtest: Does the file at obj_path_str exist?
		//
		ASSERT_TRUE(std::filesystem::exists(obj_path_str));

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());

		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

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

		//
		// Test the following:
		// - status; could be the following
		//		NOT_FOUND = -1, if HF::Exceptions::FileNotFound thrown due to invalid file path
		//		INVALID_OBJ = -2, if HF::Exceptions::InvalidOBJ thrown due to invalid .obj file
		//		GENERIC_ERROR = 0, if any other exception thrown
		//		OK = 1, operation successful
		//
		// - ensure that loaded_obj != nullptr, which would mean that status == 1 (OK)
		//

		//
		// gtest: Is status == HF_STATUS::OK? (LoadOBJ ran successfully)
		//
		ASSERT_EQ(status, 1);					// status of 1 is HF::Exceptions::HF_STATUS::OK

		//
		// gtest: Is loaded_obj non-null?
		//		  (if loaded_obj == nullptr, it was never assigned the address of a vector<MeshInfo>)
		//
		ASSERT_TRUE(loaded_obj != nullptr);		// if loaded_obj non-null, mesh was loaded successfully

		if (status != 1) {
			// All C Interface functions return a status code.
			// Error!
			std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
		}
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		//
		// loaded_obj contains the mesh.
		// It is now ready for use.
		//

		//
		// Memory management
		//

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_objloader_CInterface, StoreMesh) {
		//! [snippet_StoreMesh_setup]
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		//
		// Prepare the mesh parameters
		//

		// Each member in this array represents a set of 3 vertex coordinates, { x, y, z }.
		// mesh_indices should begin at 0, be monotonically increasing,
		// and size of mesh_indices % 3 == 0, that is to say --
		// the member count of mesh_indices should be a multiple of 3.
		// Every three members in mesh_indices constitutes a complete triangle for the mesh.
		const int size_indices = 3;
		const int mesh_indices[size_indices] = {   0,   1,   2 };
		                                    // { v_0, v_1, v_2 };

		// This is an array of vertex coordinates. 
		// Every three members in mesh_vertices constitutes a vertex.
		// Every nine members constitutes a complete triangle for the mesh.
		// Size of mesh_vertices % 3 == 0, that is to say --
		// the member count of mesh_vertices should be a multiple of 3.
		const int size_vertices = 9;
		const float mesh_vertices[size_vertices] = { 34.1f, 63.9f, 16.5f, 23.5f, 85.7f, 45.2f, 12.0f, 24.6f, 99.4f };
		                                        // {  v_0x,  v_0y,  v_0z,  v_1x,  v_1y,  v_1z,  v_2x,  v_2y,  v_2z };

		// Give the mesh a name, and an ID.
		const auto mesh_name = "This mesh";
		const int mesh_id = 0;
		//! [snippet_StoreMesh_setup]

		//! [snippet_StoreMesh_call]
		// This will point to memory on free store.
		// The memory will be allocated inside the StoreMesh function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		// Call StoreMesh
		// If the values in mesh_indices and/or mesh_vertices did not create a valid mesh,
		// or the values size_indices and/or size_vertices are not in alignment with the arrays they represent
		// (or not multiples of 3) -- an error will occur.
		status = StoreMesh(&info, mesh_indices, size_indices, mesh_vertices, size_vertices, mesh_name, mesh_id);

		if (status != 1) {
			std::cerr << "Error at StoreMesh, code: " << status << std::endl;
		}
		
		//
		// Verify that the mesh, at info (vector<MeshInfo> *),
		// was stored properly by StoreMesh.
		//
		if (info == nullptr) {
			std::cerr << "Mesh was not stored properly." << std::endl;
		}
		else {
			std::cout << "The mesh was stored properly at address " << info << "." << std::endl;
		}
		//! [snippet_StoreMesh_call]

		//
		// Memory management
		//
		
		//
		// All calls to LoadOBJ or StoreMesh must be followed up with a call to DestroyMeshInfo
		// by the client. DestroyMeshInfo will check if info is non-null before attempting to free its resources.
		//
		status = DestroyMeshInfo(info);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_objloader_CInterface, RotateMesh) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		//
		// Prepare the mesh parameters
		//

		// Each member in this array represents a set of 3 vertex coordinates, { x, y, z }.
		// mesh_indices should begin at 0, be monotonically increasing,
		// and size of mesh_indices % 3 == 0, that is to say --
		// the member count of mesh_indices should be a multiple of 3.
		// Every three members in mesh_indices constitutes a complete triangle for the mesh.
		const int size_indices = 3;
		const int mesh_indices[size_indices] = { 0,   1,   2 };
		                                  // { v_0, v_1, v_2 };

		// This is an array of vertex coordinates. 
		// Every three members in mesh_vertices constitutes a vertex.
		// Every nine members constitutes a complete triangle for the mesh.
		// Size of mesh_vertices % 3 == 0, that is to say --
		// the member count of mesh_vertices should be a multiple of 3.
		const int size_vertices = 9;
		const float mesh_vertices[size_vertices] = { 34.1f, 63.9f, 16.5f, 23.5f, 85.7f, 45.2f, 12.0f, 24.6f, 99.4f };
		                                        // {  v_0x,  v_0y,  v_0z,  v_1x,  v_1y,  v_1z,  v_2x,  v_2y,  v_2z };

		// Give the mesh a name, and an ID.
		const auto mesh_name = "This mesh";
		const int mesh_id = 0;

		// This will point to memory on free store.
		// The memory will be allocated inside the StoreMesh function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		// Call StoreMesh
		// If the values in mesh_indices and/or mesh_vertices did not create a valid mesh,
		// or the values size_indices and/or size_vertices are not in alignment with the arrays they represent
		// (or not multiples of 3) -- an error will occur.
		status = StoreMesh(&info, mesh_indices, size_indices, mesh_vertices, size_vertices, mesh_name, mesh_id);

		if (status != 1) {
			std::cerr << "Error at StoreMesh, code: " << status << std::endl;
		}
		//! [snippet_RotateMesh]
		// Prepare the desired rotation values.
		// A value of 0.0f means no rotation about the given axis.
		const float x_rot = 90.0f;
		const float y_rot = 0.0f;
		const float z_rot = 0.0f;

		// Rotate the mesh. Here, we are rotating the mesh so that it is 'z-up'.
		status = RotateMesh(info, x_rot, y_rot, z_rot);

		if (status != 1) {
			std::cerr << "Error at RotateMesh, code: " << status << std::endl;
		}
		//! [snippet_RotateMesh]
		//
		// Memory management
		//

		//
		// All calls to LoadOBJ or StoreMesh must be followed up with a call to DestroyMeshInfo
		// by the client. DestroyMeshInfo will check if info is non-null before attempting to free its resources.
		//
		status = DestroyMeshInfo(info);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_objloader_CInterface, DestroyMeshInfo) {
		//! [snippet_DestroyMeshInfo]
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Get model path
		// This is a relative path to your obj file.
		const std::string obj_path_str = "plane.obj";

		// Size of obj file string (character count)
		const int obj_length = static_cast<int>(obj_path_str.size());

		// This will point to memory on free store.
		// The memory will be allocated inside the LoadOBJ function,
		// and it must be freed using DestroyMeshInfo.
		std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

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
		else {
			std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
		}

		//
		// loaded_obj contains the mesh.
		//

		//
		// Memory management
		//

		//
		// All calls to LoadOBJ or StoreMesh must be followed up with a call to DestroyMeshInfo
		// by the client. DestroyMeshInfo will check if info is non-null before attempting to free its resources.
		//
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
		//! [snippet_DestroyMeshInfo]
	}
}
