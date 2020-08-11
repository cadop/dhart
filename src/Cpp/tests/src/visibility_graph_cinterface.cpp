/*!
	\file		visibility_graph_cinterface.cpp
	\brief		Unit test source file for testing the functionality of visibility_graph_C

	\author		Gem Aludino
	\date		07 Aug 2020
*/

#include "gtest/gtest.h"
#include "objloader_C.h"
#include "spatialstructures_C.h"
#include "visibility_graph_C.h"

#include "graph.h"

namespace CInterfaceTests {
	TEST(_visibility_graph_cinterface, CreateVisibilityGraphAllToAll) {
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

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_setup]
		// The model is a flat plane, so only nodes 0, 2 should connect.

		// Every three floats should represent a single (x, y, z) point.
		const float points[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -10.0f, 0.0f, 2.0f, 0.0f };

		// This should be the array element count within points.
		const int points_size = 9;

		// Represents how far to offset nodes from the ground.
		const float height = 1.7f;

		// Total amount of nodes to use in the visibility graph.
		const int points_count = points_size / 3;
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_setup]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_call]
		// Declare a pointer to Graph.
		//
		// Notice that we pass the address of VG to CreateVisibilityAllToAll.
		// CreateVisibilityAllToAll will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
		HF::SpatialStructures::Graph* VG = nullptr;

		status = CreateVisibilityGraphAllToAll(bvh, points, points_count, &VG, height);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateVisibilityGraphAllToAll, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_call]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_compress]
		// Always compress the graph after generating a graph/adding new edges
		status = Compress(VG);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
		//
		// VG, the visibility graph, is now ready for use.
		//
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_compress]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_output_0]
		// Retrieve CSR representation of graph
		HF::SpatialStructures::CSRPtrs csr;

		// The parameter default name refers to an alternate edge cost type.
		// An empty string means we are using the default cost type
		// (the costs that the graph was created with),
		// but alternate edge costs could also be 'CrossSlope' or 'EnergyExpenditure', etc.
		const char* default_name = "";

		status = GetCSRPointers(VG,
			&csr.nnz, &csr.rows, &csr.cols,
			&csr.data, &csr.inner_indices, &csr.outer_indices,
			default_name);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetCSRPointers, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_output_0]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_output_1]
		//
		// Setting up a CSR 'iterator'
		//
		float* curr = csr.data;					// address of current position within edge data
		float* data_end = csr.data + csr.nnz;	// address of one-past the last element within edge data

		int* inner = csr.inner_indices;			// address of current position within child node id buffer (column value)
		int row = 0;							// value denoting the current parent node id (row number)

		while (curr < data_end) {
			// While edge data remains...

			// Note the current position within the edge data buffer.
			// This is the address that denotes the beginning of a row.
			float* row_begin = curr;

			// If we are at the last row index,
			// row_end is data_end -- else,
			// row_end is the address of the next row's (row + 1) initial value.
			float* row_end =
				(row == csr.rows - 1)
				? data_end : csr.data + csr.outer_indices[row + 1];

			while (curr < row_end) {
				// While curr is not at the end of the current row...

				// row is the parent node id
				// *inner is the child node id
				// *curr is the edge value between parent node id and child node id
				std::cout << "(" << row << ", " << *inner << ")"
					<< "\t\t" << *curr << std::endl;

				++inner;	// advance the address of inner (child node id buffer)
				++curr;		// advance the address of curr (edge data buffer)
			}

			++row;	// advance the row value (parent node id)
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_output_1]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_destroy]
		// destroy VG (visibility graph)
		status = DestroyGraph(VG);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAll_destroy]

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_visibility_graph_cinterface, CreateVisibilityGraphAllToAllUndirected) {
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

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_setup]
		// The model is a flat plane, so only nodes 0, 2 should connect.

		// Every three floats should represent a single (x, y, z) point.
		const float points[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -10.0f, 0.0f, 2.0f, 0.0f };

		// This should be the array element count within points.
		const int points_size = 9;

		// Represents how far to offset nodes from the ground.
		const float height = 1.7f;

		// Total amount of nodes to use in the visibility graph.
		const int points_count = points_size / 3;

		// CPU core count. -1 means to use all avaiable cores on the client system.
		const int core_count = -1;
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_setup]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_call]
		// Declare a pointer to Graph.
		//
		// Notice that we pass the address of VG to CreateVisibilityAllToAllUndirected.
		// CreateVisibilityAllToAllUndirected will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
		HF::SpatialStructures::Graph* VG = nullptr;

		status = CreateVisibilityGraphAllToAllUndirected(bvh, points, points_count, &VG, height, core_count);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateVisibilityGraphAllToAllUndirected, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_call]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_compress]
		// Always compress the graph after generating a graph/adding new edges
		status = Compress(VG);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
		//
		// VG, the visibility graph, is now ready for use.
		//
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_compress]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_output_0]
		// Retrieve CSR representation of graph
		HF::SpatialStructures::CSRPtrs csr;

		// The parameter default name refers to an alternate edge cost type.
		// An empty string means we are using the default cost type
		// (the costs that the graph was created with),
		// but alternate edge costs could also be 'CrossSlope' or 'EnergyExpenditure', etc.
		const char* default_name = "";

		status = GetCSRPointers(VG,
			&csr.nnz, &csr.rows, &csr.cols,
			&csr.data, &csr.inner_indices, &csr.outer_indices,
			default_name);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetCSRPointers, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_output_0]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_output_1]
		//
		// Setting up a CSR 'iterator'
		//
		float* curr = csr.data;					// address of current position within edge data
		float* data_end = csr.data + csr.nnz;	// address of one-past the last element within edge data

		int* inner = csr.inner_indices;			// address of current position within child node id buffer (column value)
		int row = 0;							// value denoting the current parent node id (row number)

		while (curr < data_end) {
			// While edge data remains...

			// Note the current position within the edge data buffer.
			// This is the address that denotes the beginning of a row.
			float* row_begin = curr;

			// If we are at the last row index,
			// row_end is data_end -- else,
			// row_end is the address of the next row's (row + 1) initial value.
			float* row_end =
				(row == csr.rows - 1)
				? data_end : csr.data + csr.outer_indices[row + 1];

			while (curr < row_end) {
				// While curr is not at the end of the current row...

				// row is the parent node id
				// *inner is the child node id
				// *curr is the edge value between parent node id and child node id
				std::cout << "(" << row << ", " << *inner << ")"
					<< "\t\t" << *curr << std::endl;

				++inner;	// advance the address of inner (child node id buffer)
				++curr;		// advance the address of curr (edge data buffer)
			}

			++row;	// advance the row value (parent node id)
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_output_1]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_destroy]
		// destroy VG (visibility graph)
		status = DestroyGraph(VG);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphAllToAllUndirected_destroy]

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}

	TEST(_visibility_graph_cinterface, CreateVisibilityGraphGroupToGroup) {
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

		// Create BVH
		// We now declare a pointer to EmbreeRayTracer, named bvh.
		// Note that we pass the address of this pointer to CreateRaytracer.
		//
		// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
		// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
		// it is only interested in accessing the pointee.
		HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
		status = CreateRaytracer(loaded_obj, &bvh);

		if (status != 1) {
			std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
		}

		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_setup]
		// Element count of the first coordinate array, 'coords_a'.
		const int size_coords_a = 9;

		// Total count of nodes to be in coords_a. 
		// Every three coordinates { x, y, z } represents a single node (point).
		const int count_nodes_a = size_coords_a / 3;

		// The first coordinate array - the source node coordinates.
		const float coords_a[size_coords_a] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -10.0f, 0.0f, 2.0f, 0.0f };
		
		// Element count of the second coordinate array, 'coords_b'.
		const int size_coords_b = 9;

		// Total count of nodes to be in coords_b.
		// Every three coordinates { x, y, z } represents a single node (point).
		const int count_nodes_b = size_coords_b / 3;

		// The second coordinates array - the destination node coordinates.
		const float coords_b[size_coords_b] = { 10.0f, 10.0f, 11.0f, 10.0f, 10.0f, 0.0f, 10.0f, 12.0f, 10.0f };

		// Distance of node offset from the ground.
		const float height = 1.7f;
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_setup]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_call]
		// Declare a pointer to Graph.
		//
		// Notice that we pass the address of VG to CreateVisibilityGraphGroupToGroup.
		// CreateVisibilityGraphGroupToGroup will assign the deferenced address to a pointer that points
		// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
		HF::SpatialStructures::Graph* VG = nullptr;

		status = CreateVisibilityGraphGroupToGroup(bvh, coords_a, count_nodes_a, coords_b, count_nodes_b, &VG, height);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateVisibilityGraphGroupToGroup, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_call]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_compress]
		// Always compress the graph after generating a graph/adding new edges
		status = Compress(VG);

		if (status != 1) {
			// Error!
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_compress]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_output_0]
		// Retrieve CSR representation of graph
		HF::SpatialStructures::CSRPtrs csr;

		// The parameter default name refers to an alternate edge cost type.
		// An empty string means we are using the default cost type
		// (the costs that the graph was created with),
		// but alternate edge costs could also be 'CrossSlope' or 'EnergyExpenditure', etc.
		const char* default_name = "";

		status = GetCSRPointers(VG,
			&csr.nnz, &csr.rows, &csr.cols,
			&csr.data, &csr.inner_indices, &csr.outer_indices,
			default_name);

		if (status != 1) {
			// Error!
			std::cerr << "Error at GetCSRPointers, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_output_0]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_output_1]
		//
		// Setting up a CSR 'iterator'
		//
		float* curr = csr.data;					// address of current position within edge data
		float* data_end = csr.data + csr.nnz;	// address of one-past the last element within edge data

		int* inner = csr.inner_indices;			// address of current position within child node id buffer (column value)
		int row = 0;							// value denoting the current parent node id (row number)

		while (curr < data_end) {
			// While edge data remains...

			// Note the current position within the edge data buffer.
			// This is the address that denotes the beginning of a row.
			float* row_begin = curr;

			// If we are at the last row index,
			// row_end is data_end -- else,
			// row_end is the address of the next row's (row + 1) initial value.
			float* row_end =
				(row == csr.rows - 1)
				? data_end : csr.data + csr.outer_indices[row + 1];

			while (curr < row_end) {
				// While curr is not at the end of the current row...

				// row is the parent node id
				// *inner is the child node id
				// *curr is the edge value between parent node id and child node id
				std::cout << "(" << row << ", " << *inner << ")"
					<< "\t\t" << *curr << std::endl;

				++inner;	// advance the address of inner (child node id buffer)
				++curr;		// advance the address of curr (edge data buffer)
			}

			++row;	// advance the row value (parent node id)
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_output_1]

		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_destroy]
		// destroy VG (visibility graph)
		status = DestroyGraph(VG);

		if (status != 1) {
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		//! [snippet_VisibilityGraph_CreateVisibilityGraphGroupToGroup_destroy]

		//
		// Memory resource cleanup.
		//

		// destroy raytracer
		status = DestroyRayTracer(bvh);

		if (status != 1) {
			std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
		}

		// destroy vector<MeshInfo>
		status = DestroyMeshInfo(loaded_obj);

		if (status != 1) {
			std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
		}
	}
}
