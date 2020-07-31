/*!
    \file		test_raycasting.cpp
    \brief		Source file for Ray Casting sample usage

    \author		Gem Aludino
    \date		31 Jul 2020
*/
#include <iostream>
#include <vector>

#include <Windows.h>

namespace CInterfaceTests {
	void raycasting(HINSTANCE dll_hf);
}

namespace HF::Geometry {
	class MeshInfo;
}

namespace HF::RayTracer {
	class EmbreeRayTracer;
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

	Required definition for error codes:
		enum HF::Exceptions::HF_STATUS {
			OK = 1,					///< Operation was successful

			// Special Codes
			NOT_IMPLEMENTED = -54,	///< This function hasn't been implemented yet

			// Error Codes
			GENERIC_ERROR = 0,		///< Not sure what happened here (If this gets thrown, either fix it or give it a status code!).
			NOT_FOUND = -1,			///< The path given did not lead to any file.
			INVALID_OBJ = -2,		///< The given path did not point to a valid obj file.
			NO_GRAPH = -3,			///< This requires a valid graph in the DB to execute successfully.
			INVALID_COST = -4,		///< The given cost name does not exist in the database.
			MISSING_DEPEND = -5,	///< A dependency for this object is missing.
			OUT_OF_MEMORY = -6,		///< Ran out of memory during the last operation.
			MALFORMED_DB = -7,		///< The database exists, but is in some kind of error state.
			DB_BUSY = -8,			///< The database is busy (is there some external connection?).
			INVALID_PTR = -9,		///< One or more of the given pointers didn't lead to anything.
			OUT_OF_RANGE = -10,     ///< Tried to reference something not in the given container.
			NO_PATH = -11,			///< There is no path between the start and end points.
			NO_COST = -12,			///< There is no cost with the given name in the given graph
			NOT_COMPRESSED = -13,	///< Graph wasn't compressed!
		};
*/
void CInterfaceTests::raycasting(HINSTANCE dll_hf) {
	//
	// Load all functions from dll_hf to be used.
	//

	typedef int (*p_LoadOBJ)(const char*, int, float, float, float, std::vector<HF::Geometry::MeshInfo>**);
	typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);
	typedef int (*p_FireRay)(HF::RayTracer::EmbreeRayTracer* ert, float& x, float& y, float& z, float dx, float dy, float dz, float max_distance, bool& result);
	
	typedef int (*p_FireSingleRayDistance)(
		HF::RayTracer::EmbreeRayTracer*,
		const float*,
		const float*,
		const float,
		float*,
		int*
	);

	typedef int (*p_FireOcclusionRays)(
		HF::RayTracer::EmbreeRayTracer*,
		const float*,
		const float*,
		int,
		int,
		float,
		bool*
	);

	typedef int (*p_DestroyRayTracer)(HF::RayTracer::EmbreeRayTracer *);
	typedef int (*p_DestroyMeshInfo)(std::vector<HF::Geometry::MeshInfo>*);

	auto LoadOBJ = (p_LoadOBJ)GetProcAddress(dll_hf, "LoadOBJ");

	if (LoadOBJ == nullptr) {
		std::cerr << "Error loading function LoadOBJ." << std::endl;
		return;
	}
	else {
		std::cout << "Loaded function succesfully: " << "LoadOBJ" << std::endl;
	}

	//
	// TODO load remaining functions for this example
	//
}
