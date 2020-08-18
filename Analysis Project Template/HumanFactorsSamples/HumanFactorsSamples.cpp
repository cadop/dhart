/*!
	\file	HumanFactorsSamples.cpp
	\brief	Template project file for HumanFactors
*/

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <Windows.h>

/*!
	Paths to DLLs
*/
const wchar_t path_tbb[27] = L"..\\x64-Release\\bin\\tbb.dll";
const wchar_t path_embree3[31] = L"..\\x64-Release\\bin\\embree3.dll";
const wchar_t path_humanfactors[36] = L"..\\x64-Release\\bin\\HumanFactors.dll";

/*!
	Paths to .obj files used by examples
*/
const std::string plane_path_str = "..\\plane.obj";
const std::string energy_blob_path_str = "..\\energy_blob_zup.obj";

/*!
	\brief	Use case example code that uses HumanFactors DLL. All examples should begin here.

	\param	dll_hf	Loaded HumanFactors DLL from which all function pointers will be loaded

	\details
		At this point, the HumanFactors DLL (and the DLLs it depends on)
		will have been loaded. When this function receives dll_hf,
		there must be logic in this function that will load the functions
		that will be called by the client.

		In order to use the HumanFactors DLL,
		you must take stock of all of the data types (structures/enums/classes)
		and determine what must be forward-declared, and what must be a complete type.
		(You do not have access to the header files for these types!)

		If you want access to a structs/enum's members,
		the type must be completely defined, otherwise -- a forward-declaration will suffice.

		0) Determine what functions/types are required for your example.
		1) Forward declare/define all required types for your example.
		2) Create typedefs for all functions that are required for your example.
		3) Retrieve the function pointers using GetProcAddress.
		4) Call the functions desired via the function pointers retrieved.

		Assuming you have forward declared/defined your required types,
		here is how you would load the functions from the DLL:

		\code
			// Already forward-declared HF::Geometry::MeshInfo 
			// and HF::RayTracer::EmbreeRayTracer before main()

			// Create typedefs
			typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
			typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);

			// Obtain functions
			auto LoadOBJ = reinterpret_cast<p_LoadOBJ>(GetProcAddress(dll_hf, "LoadOBJ"));
			auto CreateRaytracer = reinterpret_cast<p_CreateRaytracer>(GetProcAddress(dll_hf, "CreateRaytracer"));

			// Now you can call LoadOBJ and CreateRaytracer.
		\endcode
*/
void HF_routine(HINSTANCE dll_hf) {
	//
	// HumanFactors example code goes here.
	//
}

/*!
	\brief	Program execution begins and ends here.

	\param	argc	Command line argument count
	\param	argv	Command line arguments

	\returns		0 on success, else failure (see error code)
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
	HINSTANCE dll_tbb = LoadLibrary(path_tbb);

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
	HINSTANCE dll_embree3 = LoadLibrary(path_embree3);

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
	HINSTANCE dll_humanfactors = LoadLibrary(path_humanfactors);

	if (dll_humanfactors == nullptr) {
		std::cerr << "Unable to load " << "HumanFactors.dll" << std::endl;

		FreeLibrary(dll_embree3);
		FreeLibrary(dll_tbb);

		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "HumanFactors.dll" << std::endl;
	}

	//
	// Ready to use dll_humanfactors here.
	//
	HF_routine(dll_humanfactors);

	//
	// Free libraries in reverse order of creation.
	//
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	
	if (FreeLibrary(dll_humanfactors)) {
		std::cout << "Freed successfully: " << "HumanFactors.dll" << std::endl;
	}

	if (FreeLibrary(dll_embree3)) {
		std::cout << "Freed successfully: " << "embree3.dll" << std::endl;
	}

	if (FreeLibrary(dll_tbb)) {
		std::cout << "Freed successfully: " << "tbb.dll" << std::endl;
	}
	
	return EXIT_SUCCESS;
}
