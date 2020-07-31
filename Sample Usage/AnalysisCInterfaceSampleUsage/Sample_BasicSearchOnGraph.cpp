/*!
	\file		Sample_BasicSearchOnGraph.cpp
	\brief		Driver source file for testing functionality of searching on a graph

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
	const wchar_t path_humanfactors[36] = L"..\\x64-Release\\bin\\HumanFactors.dll";

	void basic_search_graph(HINSTANCE dll_hf);
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
	struct Path;
	struct PathMember;
}

/*!
	\brief	Required definition, represents a point in space - a vertex in a graph
*/
struct HF::SpatialStructures::Node {
	float x, y, z;
	short type = HF::SpatialStructures::NODE_TYPE::GRAPH;
	int id;
};

/*!
	\brief	Required definition, represents a 'stop' along the way of a path
*/
struct HF::SpatialStructures::PathMember {
	float cost;
	int node;
};

/*!
	\brief	Required definition, represents an entire path (container of PathMember)
*/
struct HF::SpatialStructures::Path {
	std::vector<HF::SpatialStructures::PathMember> members;
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
			- HumanFactors.dll

		If the DLLs are not loaded in this order,
		HumanFactors.dll will fail to load!
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
		HumanFactors.dll depends on both tbb.dll and embree3.dll.
	*/
	HINSTANCE dll_humanfactors = LoadLibrary(CInterfaceTests::path_humanfactors);

	if (dll_humanfactors == nullptr) {
		std::cerr << "Unable to load " << "HumanFactors.dll" << std::endl;

		FreeLibrary(dll_embree3);
		FreeLibrary(dll_tbb);

		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "HumanFactors.dll" << std::endl;

		///
		///	Generate graph test.
		///	End status of 1 means OK.
		///
		CInterfaceTests::basic_search_graph(dll_humanfactors);

		/*
			When stepping through the debugger, the statement below is not required --
			but when running the executable, FreeLibrary(dll_humanfactors) throws an exception.
			By putting the current thread to sleep for 250 ms, dll_humanfactors can be freed.

			Solution was described here:
			https://forums.ni.com/t5/Instrument-Control-GPIB-Serial/Why-does-FreeLibrary-sometimes-crash/m-p/128079/highlight/true?profile.language=en#M7393
		*/
		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		///
		/// Free libraries in order of creation
		///
		if (FreeLibrary(dll_humanfactors)) {
			std::cout << "Freed successfully: " << "HumanFactors.dll" << std::endl;
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
		Define a start and end node id
		Calculate shortest path
		Print the list of node ids in the path
		Print the path xyz points in order
		Print the costs
		Print the sum of costs for the path

	C Interface functions called here:
	----------------------------------
		LoadOBJ - objloader_C.h
		CreateRaytracer - raytracer_C.h
		GenerateGraph - analysis_C.h
		GetCSRPointers - spatialstrucutres_C.h
		GetAllNodesFromGraph - spatialstructures_C.h
		GetSizeOfNodeVector - spatialstructures_C.h
		GetNodeID - spatialstructures_C.h
		Compress - spatialstructures_C.h
		CreatePath - pathfinder_C.h
		DestroyPath - pathfinder_C.h
		DestroyNodes - spatialstructures_C.h
		DestroyGraph - spatialstructures_C.h
		DestroyRaytracer - raytracer_C.h
		DestroyMeshInfo - objloader_C.h

	Required forward declarations
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
			struct Path;
			struct PathMember;
		}

	Required struct definitions:
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

		struct HF::SpatialStructures::PathMember {
			float cost;
			int node;
		};

		struct HF::SpatialStructures::Path {
			std::vector<HF::SpatialStructures::PathMember> members;
		};
*/
void CInterfaceTests::basic_search_graph(HINSTANCE dll_hf) {
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
	
	typedef int (*p_CreatePath)(
		const HF::SpatialStructures::Graph*,
		int,
		int,
		const char*,
		int*,
		HF::SpatialStructures::Path**,
		HF::SpatialStructures::PathMember**
	);

	typedef int (*p_GetNodeID)(HF::SpatialStructures::Graph*, const float*, int*);

	typedef int (*p_DestroyPath)(HF::SpatialStructures::Path*);
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
	auto CreatePath = (p_CreatePath)GetProcAddress(dll_hf, "CreatePath");
	auto GetNodeID = (p_GetNodeID)GetProcAddress(dll_hf, "GetNodeID");
	auto DestroyPath = (p_DestroyPath)GetProcAddress(dll_hf, "DestroyPath");
	auto DestroyGraph = (p_DestroyGraph)GetProcAddress(dll_hf, "DestroyGraph");
	auto DestroyRayTracer = (p_DestroyRayTracer)GetProcAddress(dll_hf, "DestroyRayTracer");
	auto DestroyMeshInfo = (p_DestroyMeshInfo)GetProcAddress(dll_hf, "DestroyMeshInfo");

	///
	/// Example begins here
	///
	std::cout << "\n--- Basic Search on a Graph example ---\n" << std::endl;

	// Status code variable
		// Determines if a C Interface function ran OK, or returned an error code.
	int status = 1;

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
		// Should an error occur, we can interpret the error code via code_to_str
		// when reading stderr.
		// Error!
		std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
	}

	// Create BVH

	// We now declare a pointer to EmbreeRayTracer, named embree_bvh.
	// Note that we pass the address of this pointer to CreateRaytracer.
	//
	// Note also that we pass the (vector<MeshInfo> *), loaded_obj, to CreateRaytracer -- by value.
	// This is okay, because CreateRaytracer is not assigning loaded_obj any new addresses,
	// it is only interested in accessing the pointee.
	HF::RayTracer::EmbreeRayTracer* embree_bvh = nullptr;
	status = CreateRaytracer(loaded_obj, &embree_bvh);

	if (status != 1) {
		// Error!
		std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
	}

	// Define start point.
	// These are Cartesian coordinates.
	float start_point[] = { 0.0f, 0.0f, 1.0f };

	// Define spacing.
	// This is the spacing between nodes, with respect to each axis.
	float spacing[] = { 1.0f, 1.0f, 1.0f };

	// Set max nodes.
	const int max_nodes = 100000;

	// Generate graph.
	// Notice that we pass the address of the graph pointer into GenerateGraph.
	//
	// GenerateGraph will assign the deferenced address to a pointer that points
	// to memory on the free store. We will call DestroyGraph to release the memory resources later on.

	float up_step = 1;						// maximum height of a step the graph can traverse
	float up_slope = 1;						// maximum upward slope the graph can traverse in degrees
	float down_step = 1;					// maximum step down that the graph can traverse
	float down_slope = 1;					// maximum downward slope that the graph can traverse
	int maximum_step_connection = 1;		// multiplier for number of children to generate for each node
	int core_count = -1;					// CPU core count, -1 uses all available cores on the machine

	HF::SpatialStructures::Graph* graph = nullptr;

	status = GenerateGraph(embree_bvh,
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

	// Get all nodes from graph first (the container of nodes)
	// The address of the local variable node_vector will be passed to GetAllNodesFromGraph;
	// it will be dereferenced inside that function and assigned memory via operator new.
	//
	// We will have to call DestroyNodes on node_vector to properly release this memory.
	// node_vector_data points to the internal buffer that resides within *(node_vector).
	//
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

	// Print number of nodes in the graph
	const int max_node = node_vector_size - 1;	// This is the max index of *node_vector
	std::cout << "Graph Generated with " << node_vector_size << " nodes" << std::endl;

	// Define a start and end node to use for the path (from, to)
	const int start_id = 0;
	const int end_id = 100;

	// Retrieve the nodes from node_vector using the IDs and operator[]
	HF::SpatialStructures::Node start_node = (*node_vector)[start_id];
	HF::SpatialStructures::Node end_node = (*node_vector)[end_id];

	// Print the starting and ending nodes for the path
	std::cout << "Start: \t" << "(" << start_node.x << ", " << start_node.y << ", " << start_node.z << ")" << std::endl;
	std::cout << "End:   \t" << "(" << end_node.x << ", " << end_node.y << ", " << end_node.z << ")" << std::endl;

	// Call Dijkstra's Shortest Path Algorithm
	const char* default_cost_type = "";
	int path_size = -1;

	// Declare a pointer to Path, and a pointer to PathMember.

	// A Path ADT contains a vector<PathMember>.
	// path_data will point to the internal buffer within (*path)'s vector<PathMember>.
	// &path will be passed to CreatePath, dereferenced, and assigned memory on the free store.
	//
	// We must call DestroyPath on path when we are finished with it.
	// By calling DestroyPath, we also take care of path_data, so we do not need to worry about this.
	HF::SpatialStructures::Path* path = nullptr;
	HF::SpatialStructures::PathMember* path_data = nullptr;

	status = CreatePath(graph, start_id, end_id, default_cost_type, &path_size, &path, &path_data);

	if (status != 1) {
		// Error!
		std::cerr << "Error at CreatePath, code: " << status << std::endl;
	}

	// Print the path
	std::cout << "Path:                 [";
	for (int i = 0; i < path_size; i++) {
		float cost = path_data[i].cost;
		int node = path_data[i].node;

		std::cout << "(" << cost << ", " << node << ")";

		if (i < path_size - 1) {
			std::cout << " ";
		}
	}
	std::cout << "]" << std::endl;

	///
	/// Just as the node xyz value can be extracted, the path costs and ids can be as well
	///

	// Print all costs along the path
	std::cout << "All costs along path: [";
	for (int i = 0; i < path_size; i++) {
		float cost = path_data[i].cost;

		std::cout << cost;

		if (i < path_size - 1) {
			std::cout << " ";
		}
	}
	std::cout << "]" << std::endl;

	// Print all IDs along the path
	std::cout << "All IDs along path:   [";
	for (int i = 0; i < path_size; i++) {
		int node = path_data[i].node;

		std::cout << node;

		if (i < path_size - 1) {
			std::cout << " ";
		}
	}
	std::cout << "]" << std::endl;

	// Print the total path cost
	float total_cost = 0.0;
	std::cout << "Total path cost: ";
	for (int i = 0; i < path_size; i++) {
		float cost = path_data[i].cost;

		total_cost += cost;

		if (i < path_size - 1) {
			std::cout << " ";
		}
	}
	std::cout << total_cost << std::endl;

	for (int i = 0; i < path_size; i++) {
		int node = path_data[i].node;
		float cost = path_data[i].cost;
		int type = 0;

		///
		///	The graph generated guarantees the order of the nodes in the array
		/// to correspond with the id. However, you can manually find the ID of the node
		/// itself as well. We can use the original structured node array to identify
		/// the location and value of the returned node IDs from the shortest path.
		///
		HF::SpatialStructures::Node n = (*node_vector)[node];
		float point[] = { n.x, n.y, n.z };
		int id = -1;

		status = GetNodeID(graph, point, &id);

		if (status != 1) {
			std::cerr << "Error at GetNodeID, code: " << status << std::endl;
		}

		std::cout << "Index: " << node << " Node id: " << id << ", is:\t"
			<< "[(" << n.x << ", " << n.y << ", " << n.z << ", " << type << ", " << node << ")]"
			<< std::endl;
	}
	std::cout << std::endl;


	///
	/// Memory resource cleanup.
	///

	// destroy path (we also resolve path_data here, despite not passing it to a destructor function)
	status = DestroyPath(path);

	if (status != 1) {
		// Error!
		std::cerr << "Error at DestroyPath, code: " << status << std::endl;
	}

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
	status = DestroyRayTracer(embree_bvh);

	if (status != 1) {
		std::cerr << "Error at DestroyRayTracer, code: " << status << std::endl;
	}

	// destroy vector<MeshInfo>
	status = DestroyMeshInfo(loaded_obj);

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
	std::cout << "\n--- End Example ---\n" << std::endl;
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
