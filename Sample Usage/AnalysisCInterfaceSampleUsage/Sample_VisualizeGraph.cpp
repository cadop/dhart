/*!
	\file		Sample_VisualizeGraph.cpp
	\brief		Driver source file for testing functionality of visualizing a graph

	\author		Gem Aludino
	\date		31 Jul 2020
*/
#include <iostream>
#include <vector>
#include <thread>
#include <Windows.h>

/*!
	\brief  Namespace for sample usage function prototypes

	\details
	All implementation for these sample usage functions
	will be defined in separate .cpp driver source files
	(one driver source file per function prototype/example)
*/
namespace CInterfaceTests {
	/*
		Relative paths to .dll files
	*/
	const wchar_t path_tbb[27] = L"..\\x64-Release\\bin\\tbb.dll";
	const wchar_t path_embree3[31] = L"..\\x64-Release\\bin\\embree3.dll";
	const wchar_t path_DHART_API[36] = L"..\\x64-Release\\bin\\DHARTAPI.dll";

	void visualize_graph(HINSTANCE dll_hf);
}

/*!
	\brief	Forward declaration for HF::Geometry::MeshInfo
*/
namespace HF::Geometry {
	class MeshInfo;
}

/*!
	\brief	Forward declaration for HF::RayTracer::EmbreeRayTracer
*/
namespace HF::RayTracer {
	class EmbreeRayTracer;
}

/*!
	\brief	Required definitions/forward declarations for HF::SpatialStructures
*/
namespace HF::SpatialStructures {
	enum NODE_TYPE {
		GRAPH = 0,
		POI = 1,
		OTHER = 2
	};

	struct Node;
	struct Graph;
	struct CSRPtrs;
}

/*!
	\brief	Required definition, represents a point in space - a vertex in a graph
*/
struct HF::SpatialStructures::Node {
	float x, y, z;
	short type = HF::SpatialStructures::NODE_TYPE::GRAPH;
	int id;
};

namespace CInterfaceTests {
	std::ostream& operator<<(std::ostream& os, const HF::SpatialStructures::CSRPtrs& csr);
}

/*!
	\brief	Required definition, represents a compressed sparse-row matrix
*/
struct HF::SpatialStructures::CSRPtrs {
	int nnz;	///< Number of non-zeros contained by the CSR
	int rows;	///< Number of rows in this CSR.
	int cols;	///< Number of columns in this CSR.

	float* data;			///< Stores the coefficient values of the non-zeros.
	int* outer_indices;		///< Stores for each column (resp. row) the index of the first non-zero in the previous two arrays.
	int* inner_indices;		///< Stores the row (resp. column) indices of the non-zeros.
};


/*!
	\brief  Program execution begins and ends here.

	\param  argc    Argument count
	\param  argv    Command line arguments

	\return         0 on success, else failure.
*/
int main(int argc, const char* argv[]) {
	/*
		The following DLLs must be loaded in this order:
			- tbb.dll
			- embree3.dll
			- DHARTAPI.dll

		If the DLLs are not loaded in this order,
		DHARTAPI.dll will fail to load!
	*/

	/*
		Load tbb.dll first.
	*/
	HINSTANCE dll_tbb = LoadLibrary(CInterfaceTests::path_tbb);

	if (dll_tbb == nullptr) {
		std::cerr << "Unable to load " << "tbb.dll" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "tbb.dll" << std::endl;
	}

	/*
		embree3.dll depends on tbb.dll.
	*/
	HINSTANCE dll_embree3 = LoadLibrary(CInterfaceTests::path_embree3);

	if (dll_embree3 == nullptr) {
		std::cerr << "Unable to load " << "embree3.dll" << std::endl;

		FreeLibrary(dll_tbb);
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "embree3.dll" << std::endl;
	}

	/*
		DHARTAPI.dll depends on both tbb.dll and embree3.dll.
	*/
	HINSTANCE dll_DHART_API = LoadLibrary(CInterfaceTests::path_DHART_API);

	if (dll_DHART_API == nullptr) {
		std::cerr << "Unable to load " << "DHARTAPI.dll" << std::endl;

		FreeLibrary(dll_embree3);
		FreeLibrary(dll_tbb);

		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "DHARTAPI.dll" << std::endl;

		///
		///	Generate graph test.
		///	End status of 1 means OK.
		///
		CInterfaceTests::visualize_graph(dll_DHART_API);

		/*
			When stepping through the debugger, the statement below is not required --
			but when running the executable, FreeLibrary(dll_DHART_API) throws an exception.
			By putting the current thread to sleep for 250 ms, dll_DHART_API can be freed.

			Solution was described here:
			https://forums.ni.com/t5/Instrument-Control-GPIB-Serial/Why-does-FreeLibrary-sometimes-crash/m-p/128079/highlight/true?profile.language=en#M7393
		*/
		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		///
		/// Free libraries in order of creation
		///
		if (FreeLibrary(dll_DHART_API)) {
			std::cout << "Freed successfully: " << "DHARTAPI.dll" << std::endl;
		}

		if (FreeLibrary(dll_embree3)) {
			std::cout << "Freed successfully: " << "embree3.dll" << std::endl;
		}

		if (FreeLibrary(dll_tbb)) {
			std::cout << "Freed successfully: " << "tbb.dll" << std::endl;
		}
	}
	return EXIT_SUCCESS;
}

/*!
	\brief

		Get a sample model path
		Load the obj
		Create a bvh
		Define a start point
		Defined spacing
		Set max nodes
		Generate graph
		Output image of graph nodes in x/y plane

	C Interface functions called here:
	----------------------------------
		LoadOBJ - objloader_C.h
		CreateRaytracer - raytracer_C.h
		GenerateGraph - analysis_C.h
		GetCSRPointers - spatialstrucutres_C.h
		GetAllNodesFromGraph - spatialstructures_C.h
		GetSizeOfNodeVector - spatialstructures_C.h
		Compress - spatialstructures_C.h
		DestroyNodes - spatialstructures_C.h
		DestroyGraph - spatialstructures_C.h
		DestroyRaytracer - raytracer_C.h
		DestroyMeshInfo - objloader_C.h

	Required forward declarations:
		namespace HF::Geometry {
			class MeshInfo;
		}


		namespace HF::RayTracer {
			class EmbreeRayTracer;
		}


		namespace HF::SpatialStructures {
			enum NODE_TYPE {
				GRAPH = 0,
				POI = 1,
				OTHER = 2
			};

			struct Node;
			struct Graph;
			struct CSRPtrs;
		}

	Required definitions:
		struct HF::SpatialStructures::CSRPtrs {
			int nnz;
			int rows;
			int cols;

			float* data;
			int* outer_indices;
			int* inner_indices;
		};

		struct HF::SpatialStructures::Node {
			float x, y, z;
			short type = HF::SpatialStructures::NODE_TYPE::GRAPH;
			int id;
		};

*/
void CInterfaceTests::visualize_graph(HINSTANCE dll_hf) {
	///
	/// Load all functions from dll_hf to be used.
	///

	// typedefs for brevity of syntax
	typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
	typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);

	typedef int (*p_GenerateGraph)(HF::RayTracer::EmbreeRayTracer*, const float*, const float*, int,
		float, float, float, float, int, int, HF::SpatialStructures::Graph**);

	typedef int (*p_GetCSRPointers)(HF::SpatialStructures::Graph*,
		int*, int*, int*,
		float**, int**, int**,
		const char*);

	typedef int (*p_GetAllNodesFromGraph)(
		const HF::SpatialStructures::Graph*,
		std::vector<HF::SpatialStructures::Node>**,
		HF::SpatialStructures::Node**);

	typedef int (*p_GetSizeOfNodeVector)(const std::vector<HF::SpatialStructures::Node>*, int*);
	typedef int (*p_DestroyNodes)(std::vector<HF::SpatialStructures::Node>*);
	typedef int (*p_Compress)(HF::SpatialStructures::Graph*);
	typedef int (*p_DestroyGraph)(HF::SpatialStructures::Graph*);
	typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer*);
	typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

	// Create pointers-to-functions addressed at the procedures defined in dll_hf, by using GetProcAddress()
	auto LoadOBJ = (p_LoadOBJ)GetProcAddress(dll_hf, "LoadOBJ");
	auto CreateRaytracer = (p_CreateRaytracer)GetProcAddress(dll_hf, "CreateRaytracer");
	auto GenerateGraph = (p_GenerateGraph)GetProcAddress(dll_hf, "GenerateGraph");
	auto GetCSRPointers = (p_GetCSRPointers)GetProcAddress(dll_hf, "GetCSRPointers");
	auto GetAllNodesFromGraph = (p_GetAllNodesFromGraph)GetProcAddress(dll_hf, "GetAllNodesFromGraph");
	auto GetSizeOfNodeVector = (p_GetSizeOfNodeVector)GetProcAddress(dll_hf, "GetSizeOfNodeVector");
	auto DestroyNodes = (p_DestroyNodes)GetProcAddress(dll_hf, "DestroyNodes");
	auto Compress = (p_Compress)GetProcAddress(dll_hf, "Compress");
	auto DestroyGraph = (p_DestroyGraph)GetProcAddress(dll_hf, "DestroyGraph");
	auto DestroyRayTracer = (p_DestroyRayTracer)GetProcAddress(dll_hf, "DestroyRayTracer");
	auto DestroyMeshInfo = (p_DestroyMeshInfo)GetProcAddress(dll_hf, "DestroyMeshInfo");

	///
	/// Example begins here
	///
	std::cout << "\n--- Visualize the Graph example ---\n" << std::endl;

	// Status code variable
		// Determines if a C Interface function ran OK, or returned an error code.
	int status = 1;

	// Get model path

	// This is a relative path to your obj file.
	const std::string obj_path_str = "energy_blob_zup.obj";

	// Size of obj file string (character count)
	const int obj_length = static_cast<int>(obj_path_str.size());

	// This will point to memory on free store.
	// The memory will be allocated inside the LoadOBJ function,
	// and it must be freed using DestroyMeshInfo.
	std::vector<HF::Geometry::MeshInfo>* obj = nullptr;

	// Load mesh
	// The array rot can be used to rotate the mesh.
	// We will not be rotating the mesh for this example.
	//
	// Notice that we pass the address of the obj pointer
	// to LoadOBJ. We do not want to pass obj by value, but by address --
	// so that we can dereference it and assign it to the address of (pointer to)
	// the free store memory allocated within LoadOBJ.
	const float rot[] = { 0.0f, 0.0f, 0.0f };	// no rotation.
	status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &obj);

	if (status != 1) {
		// All C Interface functions return a status code.
		// Error!
		std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
	}

	// Create BVH

	// We now declare a pointer to EmbreeRayTracer, named bvh.
	// Note that we pass the address of this pointer to CreateRaytracer.
	// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
	// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
	// it is only interested in accessing the pointee.
	HF::RayTracer::EmbreeRayTracer* bvh = nullptr;
	status = CreateRaytracer(obj, &bvh);

	if (status != 1) {
		// Error!
		std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
	}

	// Define start point.
	// These are Cartesian coordinates.
	float start_point[] = { 0.0f, -20.0f, 20.0f };

	// Define spacing.
	// This is the spacing between nodes, with respect to each axis.
	float spacing[] = { 1.0f, 1.0f, 10.0f };

	// Set max nodes.
	const int max_nodes = 5000;

	// Generate graph.
	// Notice that we pass the address of the graph pointer into GenerateGraph.
	//
	// GenerateGraph will assign the deferenced address to a pointer that points
	// to memory on the free store. We will call DestroyGraph to release the memory resources later on.

	float up_step = 0.5f;					// maximum height of a step the graph can traverse
	float up_slope = 20.0f;					// maximum upward slope the graph can traverse in degrees
	float down_step = 0.5f;					// maximum step down that the graph can traverse
	float down_slope = 20.0f;				// maximum downward slope that the graph can traverse
	int maximum_step_connection = 1;		// multiplier for number of children to generate for each node
	int core_count = -1;					// CPU core count, -1 uses all available cores on the machine

	HF::SpatialStructures::Graph* graph = nullptr;

	status = GenerateGraph(bvh,
		start_point, spacing, max_nodes,
		up_step, up_slope,
		down_step, down_slope,
		maximum_step_connection,
		core_count,
		&graph);

	if (status != 1) {
		// Error!
		std::cerr << "Error at GenerateGraph, code: " << status << std::endl;
	}

	// Always compress the graph after generating a graph/adding new edges
	status = Compress(graph);

	if (status != 1) {
		// Error!
		std::cerr << "Error at Compress, code: " << status << std::endl;
	}

	// Retrieve CSR representation of graph
	HF::SpatialStructures::CSRPtrs csr;

	// The parameter default name refers to an alternate edge cost type.
	// An empty string means we are using the default cost type
	// (the costs that the graph was created with),
	// but alternate edge costs could also be 'CrossSlope' or 'EnergyExpenditure', etc.
	const char* default_name = "";

	status = GetCSRPointers(graph,
		&csr.nnz, &csr.rows, &csr.cols,
		&csr.data, &csr.inner_indices, &csr.outer_indices,
		default_name);

	if (status != 1) {
		// Error!
		std::cerr << "Error at GetCSRPointers, code: " << status << std::endl;
	}

	// Get all nodes from graph first (the container of nodes)
	// The address of the local variable node_vector will be passed to GetAllNodesFromGraph;
	// it will be dereferenced inside that function and assigned memory via operator new.
	//
	// We will have to call DestroyNodes on node_vector to properly release this memory.
	// node_vector_data points to the internal buffer that resides within *(node_vector).
	// When we call DestroyNodes, node_vector_data's memory will also be released.
	std::vector<HF::SpatialStructures::Node>* node_vector = nullptr;
	HF::SpatialStructures::Node* node_vector_data = nullptr;

	status = GetAllNodesFromGraph(graph, &node_vector, &node_vector_data);

	if (status != 1) {
		// Error!
		std::cerr << "Error at GetAllNodesFromGraph, code: " << status << std::endl;
	}

	// Get size of node vector
	int node_vector_size = -1;
	status = GetSizeOfNodeVector(node_vector, &node_vector_size);

	if (status != 1) {
		// Error!
		std::cerr << "Error at GetSizeOfNodeVector code: " << status << std::endl;
	}

	// Get the x, y, z, coordinates of the nodes, separated by axis.

	// all x coordinates in one container, ordered by node ID
	std::vector<float> x(node_vector_size);
	auto it_x = x.begin();

	// all y coordinates in one container, ordered by node ID
	std::vector<float> y(node_vector_size);
	auto it_y = y.begin();

	// all z coordinates in one container, ordered by node ID
	std::vector<float> z(node_vector_size);
	auto it_z = z.begin();

	for (HF::SpatialStructures::Node n : *node_vector) {
		// For all nodes in *node_vector,
		// Populate each of the coordinate vectors;
		// each container represents a particular axis.
		// We have predefined the size of each coordinate vector,
		// so we can safely use iterators to assign the coordinates.
		*(it_x++) = n.x;
		*(it_y++) = n.y;
		*(it_z++) = n.z;
	}

	//
	// Output all points of the graph to the console.
	//
	// Each point will use elements from the x, y, and z containers.
	// Each point outputs as '{x[i], y[i], z[i]}'.
	// i ranges from 0...N - 1, such that N == node_vector_size.
	// (Containers x, y, z, and node_vector all have the same element count.)
	//
	// A set of graph points will begin with a '[' --
	// it will end with a ']'.
	//
	// The output will be as follows:
	//
	// [ {x[0], y[0], z[0]}, {x[1], y[1], z[1]}, {x[2], y[2], z[2]},
	// ..., {x[N - 1], y[N - 1], z[N - 1]} ]
	//
	// Every point, save the last, will be followed by a ', '.
	// Every set of three points, save the last set,
	// will be followed by a newline after the last ', ' of that set.
	//

	// Set iterators it_x, it_z, it_z to their base addresses.
	it_x = x.begin();
	it_y = y.begin();
	it_z = z.begin();

	int i = 0;	// We will print every three points on a single line, then line break.

	std::cout << "[ ";

	for (HF::SpatialStructures::Node n : *node_vector) {
		std::cout << "{" << *(it_x++) << ", "
			<< *(it_y++) << ", " << *(it_z++)
			<< "}";

		if (i < node_vector_size - 1) {
			std::cout << ", ";
			++i;
		}

		if (i % 3 == 0) {
			std::cout << std::endl;
		}
	}

	std::cout << " ]" << std::endl;

	///
	/// Memory resource cleanup.
	///

	// destroy vector<Node>
	status = DestroyNodes(node_vector);

	if (status != 1) {
		// Error!
		std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
	}

	// destroy graph
	status = DestroyGraph(graph);

	if (status != 1) {
		std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
	}

	// destroy raytracer
	status = DestroyRayTracer(bvh);

	if (status != 1) {
		std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
	}

	// destroy vector<MeshInfo>
	status = DestroyMeshInfo(obj);

	if (status != 1) {
		std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
	}

	std::cout << "\nEnd status: " << status << std::endl;
	std::cout << "\n--- End Example ---\n" << std::endl;
	if (status == 1) {
		std::cout << "[OK]" << std::endl;
	}
	else {
		std::cout << "[Error occurred]" << std::endl;
	}
}

/*!
	\brief	Operator overload for <<, writes string representation of a CSRPtrs struct to an output stream

	\param	os	reference to an output stream, i.e. std::cout, or a file
	\param	csr	reference to a HF::SpatialStructures::CSRPtrs, the CSR to stream

	\return	reference to mutated output stream, with data from parameter csr
*/
std::ostream& CInterfaceTests::operator<<(std::ostream& os, const HF::SpatialStructures::CSRPtrs& csr) {
	///
	/// CSR traversal
	///

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
			os << "(" << row << ", " << *inner << ")"
				<< "\t\t" << *curr << std::endl;

			++inner;	// advance the address of inner (child node id buffer)
			++curr;		// advance the address of curr (edge data buffer)
		}

		++row;	// advance the row value (parent node id)
	}

	return os;
}
