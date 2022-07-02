/*!
	\file		Sample_Raycasting.cpp
	\brief		Driver source file for testing functionality of the Ray casting functions

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

	void raycasting(HINSTANCE dll_hf);
}

/*!
	\brief	Forward declaration CInterfaceTests::raycasting
*/
namespace CInterfaceTests {
	void raycasting(HINSTANCE dll_hf);
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
		CInterfaceTests::raycasting(dll_humanfactors);

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
		FireRay - raytracer_C.h
		FireSingleRayDistance - raytracer_C.h
		FireOcclusionRays - raytracer_C.h
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
*/
void CInterfaceTests::raycasting(HINSTANCE dll_hf) {
	///
	///	Load all functions from dll_hf to be used.
	///

	// typedefs for brevity of syntax
	typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
	typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);
	typedef int (*p_FireRay)(HF::RayTracer::EmbreeRayTracer* ert, float& x, float& y, float& z, float dx, float dy, float dz, float max_distance, bool& result);
	typedef int (*p_FireSingleRayDistance)(HF::RayTracer::EmbreeRayTracer*, const float*, const float*, const float, float*, int*);
	typedef int (*p_FireOcclusionRays)(HF::RayTracer::EmbreeRayTracer*, const float*, const float*, int, int, float, bool*);
	typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer*);
	typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

	// Create pointers-to-functions addressed at the procedures defined in dll_hf, by using GetProcAddress()
	auto LoadOBJ = (p_LoadOBJ)GetProcAddress(dll_hf, "LoadOBJ");
	auto CreateRaytracer = (p_CreateRaytracer)GetProcAddress(dll_hf, "CreateRaytracer");
	auto FireRay = (p_FireRay)GetProcAddress(dll_hf, "FireRay");
	auto FireSingleRayDistance = (p_FireSingleRayDistance)GetProcAddress(dll_hf, "FireSingleRayDistance");
	auto FireOcculsionRays = (p_FireOcclusionRays)GetProcAddress(dll_hf, "FireOcclusionRays");
	auto DestroyRayTracer = (p_DestroyRayTracer)GetProcAddress(dll_hf, "DestroyRayTracer");
	auto DestroyMeshInfo = (p_DestroyMeshInfo)GetProcAddress(dll_hf, "DestroyMeshInfo");

	///
	/// Example begins here
	///
	std::cout << "\n--- Ray casting example ---\n" << std::endl;

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
		// Error!
		std::cerr << "Error at CreateRaytracer, code: " << status << std::endl;
	}

	// Define point to start ray
	// These are Cartesian coordinates.
	float p1[] = { 0.0f, 0.0f, 2.0f };

	// Define direction to cast ray
	// These are vector components, not Cartesian coordinates.
	float dir[] = { 0.0f, 0.0f, -1.0f };

	// Cast a ray for the hitpoint (Cast a ray, get a hit point back)
	float max_distance = -1;
	bool did_hit = false;

	// We copy the contents of p1 into hit_point.
	// hit_point will be initialized to the origin point values,
	// and if a hit occurs, hit_point will be set to the hit coordinate values.
	//
	// We will know if a hit occurs if did_hit is set 'true' by FireRay.
	float hit_point[] = { p1[0], p1[1], p1[2] };
	status = FireRay(bvh, hit_point[0], hit_point[1], hit_point[2], dir[0], dir[1], dir[2], max_distance, did_hit);

	if (status != 1) {
		// Error!
		std::cerr << "Error at FireRay, code: " << status << std::endl;
	}

	if (did_hit) {
		std::cout << "Hit point: " << "[" << hit_point[0] << ", " << hit_point[1] << ", " << hit_point[2] << "]" << std::endl;
	}
	else {
		std::cout << "Hit point: " << "(miss)" << std::endl;
	}

	// Cast a ray for the distance/meshid (Cast a ray, get a distance/mesh ID back)
	float distance = 0.0f;
	int mesh_id = -1;
	status = FireSingleRayDistance(bvh, p1, dir, max_distance, &distance, &mesh_id);

	if (status != 1) {
		// Error!
		std::cerr << "Error at FireSingleRayDistance, code: " << status << std::endl;
	}

	std::cout << "Distance is " << distance << ", " << "meshid is " << mesh_id << std::endl;

	// See if it occludes (Cast occlusion rays)
	//
	// The array p1_occl represents the same point as that of p1,
	// but for clarity, we create another array representing that point for this example.
	const int size_p1_occl = 3;
	const float p1_occl[size_p1_occl] = { p1[0], p1[1], p1[2] };

	// count_origin represents how many sets of origin coordinates we are dealing with.
	const int count_origin = size_p1_occl / 3;

	// All of the direction coordinates, inline, one after another.
	const int size_dir_occl = 9;
	const float dir_occl[size_dir_occl] = { 0.0f, 0.0f, -1.0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

	// count_dir_occl represents how many sets of directions we are dealing with.
	const int count_dir_occl = size_dir_occl / 3;

	// The array results should be the amount of rays we are firing, i.e. the value of count_origin.
	bool results[size_p1_occl] = { false };
	float max_distance_occl = 9999.0f;
	status = FireOcculsionRays(bvh, p1_occl, dir_occl, count_origin, count_dir_occl, max_distance_occl, results);

	if (status != 1) {
		// Error!
		std::cerr << "Error at FireOcclusionRays, code: " << status << std::endl;
	}

	bool does_occlude = results[0];
	std::cout << "Does the ray connect? " << (does_occlude ? std::string("True") : std::string("False")) << std::endl;

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
