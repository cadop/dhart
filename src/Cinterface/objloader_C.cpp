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
	MeshInfo<float> *** out_data_array,
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
		MeshInfo<float> ** data_array  = new MeshInfo<float>*[*num_meshes];
		for (int i = 0; i < *num_meshes; i++) {
			data_array[i] = new MeshInfo<float>(loaded_objs[i]);
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
	MeshInfo<float> ** out_info,
	const int* indices,
	int num_indices,
	const float* vertices,
	int num_vertices,
	const char* name,
	int id
) {
	// Map vertex/index arrays and name
	vector<float> vertex_array(vertices, vertices + num_vertices);
	vector<int> index_array(indices, indices + num_indices);
	std::string mesh_name(name);

	// Try to load mesh
	try {
		*out_info = new MeshInfo<float>(vertex_array, index_array, id, mesh_name);
	}
	catch (HF::Exceptions::InvalidOBJ E) {
		// If the input wasn't correctly formed, or contained nans, then return an error code
		return HF::Exceptions::INVALID_OBJ;
	}
	return OK;
}

C_INTERFACE RotateMesh(MeshInfo<float> * mesh_to_rotate, float xrot, float yrot, float zrot)
{
	if (mesh_to_rotate->meshid < 0)
		return HF_STATUS::INVALID_OBJ;

	mesh_to_rotate->PerformRotation(xrot, yrot, zrot);

	return HF_STATUS::OK;
}

C_INTERFACE GetVertsAndTris(const MeshInfo<float> * MI, int** index_out, int* num_triangles, float** vertex_out, int* num_vertices)
{
	auto mesh_vertices = MI->GetVertexPointer();
	auto mesh_indices = MI->GetIndexPointer();

	*vertex_out = mesh_vertices.data;
	*num_vertices = mesh_vertices.size / 3;

	*index_out = mesh_indices.data;
	*num_triangles = mesh_indices.size / 3;

	return HF_STATUS::OK;
}

C_INTERFACE GetMeshName(const HF::Geometry::MeshInfo<float>* MI, char** out_name)
{
	auto name = MI->name;

	int num_characters = name.size()+1; // Need extra space for null terminator
	*out_name = new char[num_characters];
	std::copy(name.c_str(), name.c_str() + num_characters, *out_name);

	return HF_STATUS::OK;
}

C_INTERFACE GetMeshID(const HF::Geometry::MeshInfo<float>* MI, int* out_id) {
	*out_id = MI->meshid;
	return HF_STATUS::OK;
}



C_INTERFACE DestroyMeshInfo(MeshInfo<float> * mesh_to_destroy)
{
	DeleteRawPtr(mesh_to_destroy);
	return OK;
}

C_INTERFACE DestroyMeshInfoPtrArray(MeshInfo<float>** data_array)
{
	if (*data_array)
		delete[] data_array;
	return OK;
}
