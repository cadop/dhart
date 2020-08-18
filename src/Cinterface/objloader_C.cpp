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
	const char* obj_path,
	HF::Geometry::GROUP_METHOD gm,
	float xrot,
	float yrot,
	float zrot,
	MeshInfo *** out_data_array,
	int* num_meshes
) {
	// Copy marshalled char array into strings
	std::string filepath(obj_path);

	// Create a new array of meshinfo pointers
	try {
		// Try to load the mesh
		auto loaded_objs = HF::Geometry::LoadMeshObjects(filepath, gm, false);
	
		// Rotate the meshes if necessary
		*num_meshes = loaded_objs.size();
		for (auto& mesh : loaded_objs)
			mesh.PerformRotation(xrot, yrot, zrot);
	
		// Insert into output array (this should be an instance of copy elision.
		MeshInfo ** data_array  = new MeshInfo*[*num_meshes];
		for (int i = 0; i < *num_meshes; i++) {
			data_array[i] = new MeshInfo(loaded_objs[i]);
			assert(data_array[i]->GetMeshID() == loaded_objs[i].GetMeshID());
		}
		*out_data_array = data_array;

		return HF_STATUS::OK;
	}
	catch (const HF::Exceptions::InvalidOBJ & e) {
		return HF_STATUS::INVALID_OBJ;
	}
	catch (const HF::Exceptions::FileNotFound & e) {
		return HF_STATUS::NOT_FOUND;
	}
	catch (...) {
		std::cerr << "Generic Error" << std::endl;
		return HF_STATUS::GENERIC_ERROR;
	}

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

C_INTERFACE DestroyMeshInfo(MeshInfo * mesh_to_destroy)
{
	DeleteRawPtr(mesh_to_destroy);
	return OK;
}

C_INTERFACE DestroyMeshInfoPtrArray(MeshInfo** data_array)
{
	if (*data_array)
		delete[] data_array;
	return OK;
}
