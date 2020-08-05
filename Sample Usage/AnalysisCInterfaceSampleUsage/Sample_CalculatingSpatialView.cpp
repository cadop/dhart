/*!
	\file		Sample_CalculatingSpatialView.cpp
	\brief		Driver source file for testing calcuation of spatial view

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

	void calculating_spatial_view(HINSTANCE dll_hf);
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
	\brief	Required definition for spherical view analysis
*/
struct RayResult {
	float distance = -1;
	int meshid = -1;
};

/*!
	\brief	Required definition for spherical view analysis (edge aggregation)
*/
enum class AGGREGATE_TYPE {
	/// <summary> Number of rays that hit. </summary>
	COUNT = 0,
	/// <summary> Sum of distances from the origin to each of its hitpoints. </summary>
	SUM = 1,
	/// <summary> Average distance of origin to its hit points. </summary>
	AVERAGE = 2,
	/// <summary> Maximum distance from origin to its hit points. </summary>
	MAX = 3,
	/// <summary> Minimum distance from origin to its hit points. </summary>
	MIN = 4
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
		///	Ray casting test.
		///	End status of 1 means OK.
		///
		CInterfaceTests::calculating_spatial_view(dll_humanfactors);

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
	\brief	Testing of raycasting functionality

		Load a sample model path
		Load the obj
		Create a bvh
		Define a start point
		Define a direction
		Cast a ray
		Print resulting hit location xyz
		Print distance of hit
		Change direction of ray
		Cast new ray
		Print resulting hit location xyz
		Print distance of hit

	C Interface functions called here:
	----------------------------------
		LoadOBJ - objloader_C.h
		CreateRaytracer - raytracer_C.h
		SphereicalViewAnalysisAggregateFlat - view_analysis_C.h
		SphericalViewAnalysisNoAggregateFlat - view_analysis_C.h
		DestroyRaytracer - raytracer_C.h
		DestroyMeshInfo - objloader_C.h

	Required forward declarations:
	------------------------------
		namespace HF::Geometry {
			class MeshInfo;
		}

		namespace HF::RayTracer {
			class EmbreeRayTracer;
		}

	Required definitions:
		struct RayResult {
			float distance = -1; int meshid = -1;
		};

		enum class AGGREGATE_TYPE {
			COUNT = 0,
			SUM = 1,
			AVERAGE = 2,
			MAX = 3,
			MIN = 4
		};
*/
void CInterfaceTests::calculating_spatial_view(HINSTANCE dll_hf) {
	///
	///	Load all functions from dll_hf to be used.
	///

	// typedefs for brevity of syntax
	typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
	typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);

	typedef int (*p_SphereicalViewAnalysisAggregateFlat)(
		HF::RayTracer::EmbreeRayTracer*,
		const float*,
		int,
		int,
		float,
		float,
		float,
		AGGREGATE_TYPE,
		std::vector<float>**,
		float**,
		int*
		);

	typedef int (*p_SphericalViewAnalysisNoAggregateFlat)(
		HF::RayTracer::EmbreeRayTracer*,
		const float*,
		int,
		int*,
		float,
		float,
		float,
		std::vector<RayResult>**,
		RayResult**
		);

	typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer*);
	typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

	// Create pointers-to-functions addressed at the procedures defined in dll_hf, by using GetProcAddress()
	auto LoadOBJ = (p_LoadOBJ)GetProcAddress(dll_hf, "LoadOBJ");
	auto CreateRaytracer = (p_CreateRaytracer)GetProcAddress(dll_hf, "CreateRaytracer");
	auto SphereicalViewAnalysisAggregateFlat = (p_SphereicalViewAnalysisAggregateFlat)GetProcAddress(dll_hf, "SphereicalViewAnalysisAggregateFlat");
	auto SphericalViewAnalysisNoAggregateFlat = (p_SphericalViewAnalysisNoAggregateFlat)GetProcAddress(dll_hf, "SphericalViewAnalysisNoAggregateFlat");
	auto DestroyRayTracer = (p_DestroyRayTracer)GetProcAddress(dll_hf, "DestroyRayTracer");
	auto DestroyMeshInfo = (p_DestroyMeshInfo)GetProcAddress(dll_hf, "DestroyMeshInfo");

	///
	/// Example begins here
	///
	std::cout << "\n--- Calculating Spatial View example ---\n" << std::endl;

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

	// Preparing the parameters for SphericalViewAnalysisNoAggregate

	// Define point to start ray
	// These are Cartesian coordinates.
	float p1[] = { 0.0f, 0.0f, 2.0f };

	// Define direction to cast ray
	// These are vector components, not Cartesian coordinates.
	float dir[] = { 0.0f, 0.0f, -1.0f };

	// This is a container of nodes to be analyzed.
	// SphericalViewAnalysisNoAggregate accepts a (Node *) parameter,
	// so a raw stack-allocated array will suffice.
	//
	// This may also be a pointer to a heap-allocated buffer
	// (which is owned by a vector<Node>, accessed via the vector<Node>::data() method).
	//
	// ...or, it can be a pointer to memory allocated by the caller via operator new[node_count]
	// (if this route is taken, be sure to release the memory with operator delete[] after use)

	const int node_count = 1;			// should be the element count in nodes
	int ray_count = 1000;				// will be mutated by SphericalViewAnalysisNoAggregate
	const float height = 1.7f;			// height to offset nodes from the ground, in meters
	const float upward_fov = 50.0f;		// default parameter is 50.0f in Python code
	const float downward_fov = 70.0f;	// default parameter is 70.0f in Python code

	// Declare a pointer to vector<RayResult>, named results.
	// This pointer will point to memory on the free store,
	// allocated within SphericalViewAnalysisNoAggregateFlat.
	//
	// results_data will point to the underlying buffer within *results,
	// which will be assigned inside SphericalViewAnalysisNoAggregateFlat.
	//
	// Note that we must call operator delete on results when we are finished with it.
	std::vector<RayResult>* results = nullptr;
	RayResult* results_data = nullptr;

	// Conducting a view analysis on the node at position p1.
	status = SphericalViewAnalysisNoAggregateFlat(bvh,
		p1, node_count, &ray_count,
		upward_fov, downward_fov, height,
		&results, &results_data);

	if (status != 1) {
		// Error!
		std::cerr << "Error at SphericalViewAnalysisNoAggregateFlat, code: " << status << std::endl;
	}

	// Declare a pointer to vector<float>, named aggregate_results.
	// This pointer will point to memory on the free store,
	// allocated within SphericalViewAnalysisAggregateFlat.
	//
	// aggregate_results_data will point to the underlying buffer within *aggregate_results,
	// which will be assigned inside SphericalViewAnalysisNoAggregateFlat.
	//
	// Note that we must call operator delete on aggregate_results when we are finished with it.
	std::vector<float>* aggregate_results = nullptr;
	float* aggregate_results_data = nullptr;

	// Will be equal to aggregate_results->size();
	int aggregate_results_size = -1;

	// Select the aggregate type.
	// This determines how to aggregate the edges within the results of the view analysis.
	//
	// AGGREGATE_TYPE::AVERAGE means that the edges will be aggregated
	// by the maximum distance from the origin point to its hit points.
	AGGREGATE_TYPE agg_type = AGGREGATE_TYPE::AVERAGE;

	status = SphereicalViewAnalysisAggregateFlat(bvh,
		p1, node_count, ray_count,
		upward_fov, downward_fov, height,
		agg_type,
		&aggregate_results, &aggregate_results_data, &aggregate_results_size);

	if (status != 1) {
		// Error!
		std::cerr << "Error at SphereicalViewAnalysisAggregateFlat, code: " << status << std::endl;
	}

	// Print results vector (vector<RayResult>)
	const int start_range = 15;
	const int end_range = 20;

	std::cout << "[";
	for (int i = start_range; i < end_range; i++) {
		auto result = (*results)[i];

		std::cout << "(" << result.distance << ", " << result.meshid << ")";

		if (i < end_range - 1) {
			std::cout << ", ";
		}
	}
	std::cout << "]" << std::endl;

	// Print aggregate results vector (vector<float>)
	std::cout << "[";
	int i = 0;
	for (auto agg_result : *aggregate_results) {
		std::cout << agg_result;

		if (i < aggregate_results->size() - 1) {
			std::cout << ", ";
		}

		++i;
	}
	std::cout << "]" << std::endl;

	///
	/// Memory resource cleanup.
	///

	// destroy vector<float>
	if (aggregate_results) {
		delete aggregate_results;
	}

	// destroy vector<RayResult>
	if (results) {
		delete results;
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
	std::cout << "\n--- End Example ---\n" << std::endl;
	if (status == 1) {
		std::cout << "[OK]" << std::endl;
	}
	else {
		std::cout << "[Error occurred]" << std::endl;
	}
	std::cout << "\n--- End Example ---\n" << std::endl;
}
