/*!
	\file		Sample_PathPlanCosts.cpp
	\brief		Driver source file for testing functionality of the pathfinding functions (alternate costs)

	\author		Gem Aludino
	\date		12 Aug 2020
*/
#include <iostream>
#include <vector>
#include <array>
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

	void path_plan_costs(HINSTANCE dll_hf);
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

/*!
	\brief	Required definition, 
			represents indices of keys for costs returned from calling CalculateAndStoreEnergyExpenditure
*/
const enum COST_ALG_KEY { CROSS_SLOPE, ENERGY_EXPENDITURE };

/*!
	\brief	Required definition,
			represents keys of costs for calling "CalculateAndStore"-- functions
*/
const std::vector<std::string> Key_To_Costs{ "CrossSlope", "EnergyExpenditure" };

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
		///	Path Plan with Different Costs test.
		///	End status of 1 means OK.
		///
		CInterfaceTests::path_plan_costs(dll_humanfactors);

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
	\brief	Testing functionality of path traversal with different costs

		Load a sample model path
		Load the obj
		Create a bvh
		Define a start point
		Define spacing
		Define maximum node count
		Define up step, down step
		Define up slope, down slope
		Define max step connections
		Generate graph
		Retrieve nodes
		Define a start and end point in x, y
		From the graph, get node closest to that point. Output coordinates.
		Call Dijkstra's Shortest Path algorithm, using default cost (euclidean distance) - save total path cost and output it.
		Get the x, y, z values of the nodes at the given path ids
		Extract the x, y, z locations of the nodes
		Extract the x, y, z locations of the path nodes
		Plot the graph
		Calculate energy expenditure of the graph edges, which internally allows access to this weight
		Get the key: ENERGY_EXPENDITURE
		Call the shortest path again, with the optional cost type (ENERGY_EXPENDITURE). Save total path cost and output it.
		Get the x, y, z values of the nodes at the given path ids
		Extract the x, y, z locations of the nodes
		Extract the x, y, z locations of the path nodes
		Plot the graph

	C Interface functions called here:
	----------------------------------
		LoadOBJ - objloader_C.h
		CreateRaytracer - raytracer_C.h
		GenerateGraph - analysis_C.h
		GetAllNodesFromGraph - spatialstructures_C.h
		GetSizeOfNodeVector - spatialstructures_C.h
		GetNodeID - spatialstructures_C.h
		Compress - spatialstructures_C.h
		CreatePath - pathfinder_C.h
		CalculateAndStoreEnergyExpenditure - spatialstructures_C.h
		DestroyPath - pathfinder_C.h
		DestroyNodes - spatialstructures_C.h
		DestroyGraph - spatialstructures_C.h
		DestroyRaytracer - raytracer_C.h
		DestroyMeshInfo - objloader_C.h

	Required forward declarations:
	-----------------------------
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
	----------------------------
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

	Required enum definitions:
	--------------------------
		const enum COST_ALG_KEY { CROSS_SLOPE, ENERGY_EXPENDITURE };

	Required global instances:
	---------------------------
	const std::vector<std::string> Key_To_Costs{ "CrossSlope", "EnergyExpenditure" };
*/
void CInterfaceTests::path_plan_costs(HINSTANCE dll_hf) {
	//
	//	Load all functions from dll_hf to be used.
	//

	// typedefs for brevity of syntax
	typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
	typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);

	typedef int (*p_GenerateGraph)(HF::RayTracer::EmbreeRayTracer*, const float*, const float*, int,
		float, float, float, float, int, int, HF::SpatialStructures::Graph**);

	typedef int (*p_GetAllNodesFromGraph)(
		const HF::SpatialStructures::Graph*,
		std::vector<HF::SpatialStructures::Node>**,
		HF::SpatialStructures::Node**);

	typedef int (*p_GetSizeOfNodeVector)(const std::vector<HF::SpatialStructures::Node>*, int*);
	typedef int (*p_GetNodeID)(HF::SpatialStructures::Graph*, const float*, int*);
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

	typedef int (*p_CalculateAndStoreEnergyExpenditure)(HF::SpatialStructures::Graph*);
	typedef int (*p_DestroyPath)(HF::SpatialStructures::Path*);
	typedef int (*p_DestroyNodes)(std::vector<HF::SpatialStructures::Node>*);
	typedef int (*p_DestroyGraph)(HF::SpatialStructures::Graph*);
	typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer*);
	typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

	// Create pointers-to-functions addressed at the procedures defined in dll_hf, by using GetProcAddress()
	auto LoadOBJ = reinterpret_cast<p_LoadOBJ>(GetProcAddress(dll_hf, "LoadOBJ"));
	auto CreateRaytracer = reinterpret_cast<p_CreateRaytracer>(GetProcAddress(dll_hf, "CreateRaytracer"));
	auto GenerateGraph = reinterpret_cast<p_GenerateGraph>(GetProcAddress(dll_hf, "GenerateGraph"));
	auto GetAllNodesFromGraph = reinterpret_cast<p_GetAllNodesFromGraph>(GetProcAddress(dll_hf, "GetAllNodesFromGraph"));
	auto GetSizeOfNodeVector = reinterpret_cast<p_GetSizeOfNodeVector>(GetProcAddress(dll_hf, "GetSizeOfNodeVector"));
	auto GetNodeID = reinterpret_cast<p_GetNodeID>(GetProcAddress(dll_hf, "GetNodeID"));
	auto Compress = reinterpret_cast<p_Compress>(GetProcAddress(dll_hf, "Compress"));
	auto CreatePath = reinterpret_cast<p_CreatePath>(GetProcAddress(dll_hf, "CreatePath"));

	auto CalculateAndStoreEnergyExpenditure 
		= reinterpret_cast<p_CalculateAndStoreEnergyExpenditure>(GetProcAddress(dll_hf, "CalculateAndStoreEnergyExpenditure"));

	auto DestroyPath = reinterpret_cast<p_DestroyPath>(GetProcAddress(dll_hf, "DestroyPath"));
	auto DestroyNodes = reinterpret_cast<p_DestroyNodes>(GetProcAddress(dll_hf, "DestroyNodes"));
	auto DestroyGraph = reinterpret_cast<p_DestroyGraph>(GetProcAddress(dll_hf, "DestroyGraph"));
	auto DestroyRayTracer = reinterpret_cast<p_DestroyRayTracer>(GetProcAddress(dll_hf, "DestroyRayTracer"));
	auto DestroyMeshInfo = reinterpret_cast<p_DestroyMeshInfo>(GetProcAddress(dll_hf, "DestroyMeshInfo"));

	//
	// Example begins here
	//
	std::cout << "\n--- Path Plan with Different Costs example ---\n" << std::endl;

	// Status code variable, value returned by C Interface functions
	// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
	int status = 0;

	// Get model path
	// This is a relative path to your obj file.
	const std::string obj_path_str = "energy_blob_zup.obj";

	// Size of obj file string (character count)
	const int obj_length = static_cast<int>(obj_path_str.size());

	// This will point to memory on free store.
	// The memory will be allocated inside the LoadOBJ function,
	// and it must be freed using DestroyMeshInfo.
	std::vector<HF::Geometry::MeshInfo>* loaded_obj = nullptr;

	// Load mesh.
	// Notice that we pass the address of the loaded_obj pointer
	// to LoadOBJ. We do not want to pass loaded_obj by value, but by address --
	// so that we can dereference it and assign it to the address of (pointer to)
	// the free store memory allocated within LoadOBJ.
	const float rot[] = { 0.0f, 0.0f, 0.0f };	// No rotation.
	status = LoadOBJ(obj_path_str.c_str(), obj_length, rot[0], rot[1], rot[2], &loaded_obj);

	if (status != 1) {
		// All C Interface functions return a status code.
		// Error!
		std::cerr << "Error at LoadOBJ, code: " << status << std::endl;
	}
	else {
		std::cout << "LoadOBJ loaded mesh successfully into loaded_obj at address " << loaded_obj << ", code: " << status << std::endl;
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
		// Error!
		std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
	}
	else {
		std::cout << "CreateRaytracer created EmbreeRayTracer successfully into bvh at address " << bvh << ", code: " << status << std::endl;
	}

	//
	// Set the graph parameters
	//

	// Define start point.
	// These are Cartesian coordinates.
	// If not above solid ground, no nodes will be generated.
	const std::array<float, 3> start_point { -30.0f, 0.0f, 20.0f };

	// Define spacing.
	// This is the spacing between nodes, with respect to each axis.
	// Lower values create more nodes, yielding a higher resolution graph.
	const std::array<float, 3> spacing { 2.0f, 2.0f, 180.0f };

	// Value of - 1 will generate infinitely many nodes.
	// Final node count may be greater than this value.
	const int max_nodes = 5000;

	const int up_step = 30;					// Maximum step height the graph can traverse. 
											// Steps steeper than this are considered to be inaccessible.

	const int down_step = 70;				// Maximum step down the graph can traverse.
											// Steps steeper than this are considered to be inaccessible.

	const int up_slope = 60;				// Maximum upward slope the graph can traverse (in degrees).
											// Slopes steeper than this are considered to be inaccessible.

	const int down_slope = 60;				// Maximum downward slope the graph can traverse (in degrees).
											// Slopes steeper than this are considered to be inaccessible.

	const int max_step_connections = 1;		// Multiplier for number of children to generate for each node.
											// Increasing this value increases the number of edges in the graph,
											// therefore increasing the memory footprint of the algorithm overall.

	const int core_count = -1;				// CPU core count. A value of (-1) will use all available cores.

	// Generate graph.
	// Notice that we pass the address of the graph pointer into GenerateGraph.
	//
	// GenerateGraph will assign the deferenced address to a pointer that points
	// to memory on the free store. We will call DestroyGraph to release the memory resources later on.
	HF::SpatialStructures::Graph* graph = nullptr;

	status = GenerateGraph(bvh, start_point.data(), spacing.data(), max_nodes, 
		                   up_step, down_step, up_slope, down_slope, 
						   max_step_connections, core_count, &graph);

	if (status != 1) {
		std::cerr << "Error at GenerateGraph, code: " << status << std::endl;
	}
	else {
		std::cout << "Generate graph ran successfully - graph stored at address " << graph << std::endl;
	}

	// Always compress the graph after generating a graph/adding new edges
	status = Compress(graph);

	if (status != 1) {
		// Error!
		std::cerr << "Error at Compress, code: " << status << std::endl;
	}
	
	// Get nodes.
	//
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

	/*
		# Define a start and end point in x,y
		p_desired = np.array([[-30,0],[30,0]])
		closest_nodes = graph.get_closest_nodes(p_desired,z=False)
		print("Closest Node: ", closest_nodes)

		# Closest Node:  [  0 795]
	*/

	/*
		from scipy.spatial.distance import cdist
		closest_nodes_all = cdist(graph.get_node_points(), graph.get_node_points())
		# >>>

		# Define a start and end node to use for the path (from, to)
		start_id, end_id = closest_nodes[0], closest_nodes[1]
		# >>>

		# Call the shortest path
		path = DijkstraShortestPath(graph, start_id, end_id)
		# >>>

		# As the cost array is numpy, simple operations to sum the total cost can be calculated
		path_sum = np.sum(path['cost_to_next'])
		print('Total path cost: ', path_sum)

		# Total path cost:  62.02023
	*/

	/*
		# Get the x,y,z values of the nodes at the given path ids
		path_xyz = np.take(nodes[['x','y','z']], path['id'])
		# >>>

		# Extract the xyz locations of the nodes
		x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
		# >>>

		# Extract the xyz locations of the path nodes
		x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']
		# >>>

		# Plot the graph
		plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5) # doctest: +SKIP
		plt.plot(x_path,y_path, c="red", marker='.',linewidth=3) # doctest: +SKIP
		plt.show() # doctest: +SKIP
	*/

	//
	// Memory resource cleanup.
	//

	// destroy vector<Node>
	status = DestroyNodes(node_vector);

	if (status != 1) {
		std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
	}

	// destroy Path
	//
	// TODO
	//

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
	status = DestroyMeshInfo(loaded_obj);

	if (status != 1) {
		std::cerr << "Error at DestroyMeshInfo, code: " << status << std::endl;
	}

	std::cout << "\nEnd status: " << status << std::endl;
	if (status == 1) {
		std::cout << "[OK]" << std::endl;
	}
	else {
		std::cout << "[Error occurred]" << std::endl;
	}
	std::cout << "\n--- End Example ---\n" << std::endl;
}
