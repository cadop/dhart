#include <objloader_C.h>
#include <objloader.h>
#include <meshinfo.h>
#include <vector>

#include <HFExceptions.h>
#include <iostream>
#include <cinterface_utils.h>
using namespace HF::Geometry;
using namespace HF::Exceptions;
using std::vector;

C_INTERFACE LoadOBJ(
	const char* obj_paths,
	int length,
	float xrot,
	float yrot,
	float zrot,
	vector<MeshInfo>** out_list
) {
	//Check precondition
	if (length <= 0) {
		std::cerr << "Empty Input" << std::endl;
		return HF_STATUS::GENERIC_ERROR;
	}

	// Copy marshalled char array into strings
	std::string filepath = obj_paths;
	vector<MeshInfo>* meshes = new vector<MeshInfo>();

	// Try to load the mesh
	try {
		*meshes = HF::Geometry::LoadMeshObjects(filepath, HF::Geometry::ONLY_FILE, false);
		for (auto& mesh : *meshes)
			mesh.PerformRotation(xrot, yrot, zrot);
	}

	catch (const HF::Exceptions::InvalidOBJ & e) {
		delete meshes;
		return HF_STATUS::INVALID_OBJ;
	}
	catch (const HF::Exceptions::FileNotFound & e) {
		delete meshes;
		return HF_STATUS::NOT_FOUND;
	}
	catch (...) {
		std::cerr << "Generic Error" << std::endl;
		delete meshes;
		return HF_STATUS::GENERIC_ERROR;
	}

	*out_list = meshes;
	return HF_STATUS::OK;
}

C_INTERFACE StoreMesh(
	vector<MeshInfo>** out_info,
	const int* indices,
	int num_indices,
	const float* vertices,
	int num_vertices,
	const char* name,
	int id
) {
	vector<float> vertex_array(vertices, vertices + num_vertices);
	vector<int> index_array(indices, indices + num_indices);
	std::string mesh_name(name);

	try {
		vector<MeshInfo>* meshes = new vector<MeshInfo>();
		meshes->push_back(MeshInfo(vertex_array, index_array, id, mesh_name));
		*out_info = meshes;
	}
	catch (HF::Exceptions::InvalidOBJ E) {
		return HF::Exceptions::INVALID_OBJ;
	}
	return OK;
}

C_INTERFACE RotateMesh(vector<MeshInfo>* mesh_to_rotate, float xrot, float yrot, float zrot)
{
	for (auto & mesh : *mesh_to_rotate)
		mesh.PerformRotation(xrot, yrot, zrot);
	return HF_STATUS::OK;
}

C_INTERFACE DestroyMeshInfo(vector<MeshInfo>* meshes_to_destroy)
{
	if (meshes_to_destroy)
		delete meshes_to_destroy;
	return OK;
}