/*!
	\file		objloader_cinterface.cpp
	\brief		Unit test source file for testing the examples for objloader_C documentation

	\author		Gem Aludino
	\date		05 Aug 2020
*/

#include "gtest/gtest.h"

#include "objloader_C.h"

namespace CInterfaceTests {
	TEST(_objloader_CInterface, LoadOBJ) {
		//! [snippet_LoadOBJ]
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
		//! [snippet_LoadOBJ]
	}

	TEST(_objloader_CInterface, StoreMesh) {
		//! [snippet_StoreMesh]
		// Requires #include "objloader_C.h", #include "meshinfo.h"

		// Prepare parameters for StoreMesh
		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		int mesh_indices[] = { 0, 1, 2 };
		const int mesh_num_indices = 3;
		float mesh_vertices[] = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
		const int mesh_num_vertices = 9;

		std::string mesh_name = "This mesh";
		const int mesh_id = 0;

		// Call StoreMesh
		if (StoreMesh(&info, mesh_indices, mesh_num_indices, mesh_vertices, mesh_num_vertices, mesh_name.c_str(), mesh_id)) {
			std::cout << "StoreMesh successful" << std::endl;
		}
		else {
			std::cout << "StoreMesh unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
		//! [snippet_StoreMesh]
	}

	TEST(_objloader_CInterface, RotateMesh) {
		//! [snippet_RotateMesh]
		// Requires #include "objloader_C.h", #include "meshinfo.h"

		// Prepare parameters for StoreMesh
		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		int mesh_indices[] = { 0, 1, 2 };
		const int mesh_num_indices = 3;
		float mesh_vertices[] = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
		const int mesh_num_vertices = 9;

		std::string mesh_name = "This mesh";
		const int mesh_id = 0;

		// Call StoreMesh
		if (StoreMesh(&info, mesh_indices, mesh_num_indices, mesh_vertices, mesh_num_vertices, mesh_name.c_str(), mesh_id)) {
			std::cout << "StoreMesh successful" << std::endl;
		}
		else {
			std::cout << "StoreMesh unsuccessful" << std::endl;
		}

		// Prepare desired rotation values
		const float x_rot = 10;
		const float y_rot = 10;
		const float z_rot = 20;

		// Call RotateMesh
		if (RotateMesh(info, x_rot, y_rot, z_rot)) {
			std::cout << "RotateMesh successful" << std::endl;
		}
		else {
			std::cout << "RotateMesh unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
		//! [snippet_RotateMesh]
	}

	TEST(_objloader_CInterface, DestroyMeshInfo) {
		//! [snippet_DestroyMeshInfo]
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
		//! [snippet_DestroyMeshInfo]
	}
}
