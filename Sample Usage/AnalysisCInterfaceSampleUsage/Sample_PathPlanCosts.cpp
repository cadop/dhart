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
	///
	///	Load all functions from dll_hf to be used.
	///

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
	auto GetSizeOfNodeVector = reinterpret_cast<p_GetAllNodesFromGraph>(GetProcAddress(dll_hf, "GetSizeOfNodeVector"));
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

	///
	/// Example begins here
	///
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

	// Load mesh
	// The array rot will rotate the mesh 90 degrees with respect to the x-axis,
	// i.e. makes the mesh 'z-up'.
	//
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

	///
	/// TODO implementation
	///

	///
	/// Memory resource cleanup.
	///

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
