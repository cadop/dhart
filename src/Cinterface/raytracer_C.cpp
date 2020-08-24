#include <raytracer_C.h>
#include <iostream>

#include <embree_raytracer.h>
#include <meshinfo.h>
#include <HFExceptions.h>
#include <cinterface_utils.h>

using std::vector;
using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;
using namespace HF::Exceptions;
//TODO: Use a template for this

C_INTERFACE CreateRaytracer(MeshInfo * mesh, EmbreeRayTracer** out_raytracer, bool use_precise)
{
	// Create the raytracer with the first mesh.
	try {

		// Iterate through all of the meshes in our input and add
		// them to the raytracer
		*out_raytracer = new EmbreeRayTracer(*mesh);
		return OK;
	}
	// Thrown if Embree is missing
	catch (const HF::Exceptions::MissingDependency & e) { 
		if (*out_raytracer != NULL)
			delete *out_raytracer;
		return MISSING_DEPEND; 
	}
	catch (const HF::Exceptions::InvalidOBJ & e) {
		if (*out_raytracer != NULL)
			delete *out_raytracer;
		return INVALID_OBJ;
	}
	return GENERIC_ERROR;
}

C_INTERFACE CreateRaytracerMultiMesh(MeshInfo** meshes, int num_meshes, EmbreeRayTracer** out_raytracer, bool use_precise)
{
	// Create the raytracer with the first mesh.
	*out_raytracer = new EmbreeRayTracer(use_precise);
	try {

		// Iterate through all of the meshes in our input and add
		// them to the raytracer
		for (int i = 0; i < num_meshes; i++) {
			// Only commit to scene if this is the final mesh in the array
			bool should_commit = (i == num_meshes - 1);

			(*out_raytracer)->AddMesh(*(meshes[i]), should_commit);
		}

		return OK;
	}
	// Thrown if Embree is missing
	catch (const HF::Exceptions::MissingDependency& e) {
		if (*out_raytracer != NULL)
			delete* out_raytracer;
		return MISSING_DEPEND;
	}
	catch (const HF::Exceptions::InvalidOBJ& e) {
		if (*out_raytracer != NULL)
			delete* out_raytracer;
		return INVALID_OBJ;
	}
	return GENERIC_ERROR;
}


C_INTERFACE AddMeshes(HF::RayTracer::EmbreeRayTracer* ERT, HF::Geometry::MeshInfo ** MI, int number_of_meshes)
{
	// Iterate through each input mesh, only committing the scene
	// at the final mesh. 
	for (int i = 0; i < number_of_meshes; i++) {
		bool should_commit = (i == number_of_meshes - 1);
	
		ERT->AddMesh(*(MI[i]), should_commit);
	}

	return HF_STATUS::OK;
}

C_INTERFACE AddMesh(HF::RayTracer::EmbreeRayTracer* ERT, HF::Geometry::MeshInfo* MI)
{
	ERT->AddMesh(*MI, true);

	return HF_STATUS::OK;
}

C_INTERFACE DestroyRayTracer(HF::RayTracer::EmbreeRayTracer* rt_to_destroy)
{
	if (rt_to_destroy)
		delete rt_to_destroy;
	return OK;
}

C_INTERFACE FireSingleRayDistance(
	HF::RayTracer::EmbreeRayTracer* ert,
	const float* origin,
	const float* direction,
	const float max_distance,
	float* out_distance,
	int* out_meshid
)
{
	ert->IntersectOutputArguments(origin, direction, *out_distance, *out_meshid);
	return HF::Exceptions::HF_STATUS::OK;
}

C_INTERFACE FireRaysDistance(
	HF::RayTracer::EmbreeRayTracer* ert,
	float* origins,
	int num_origins,
	float* directions,
	int num_directions,
	std::vector<RayResult>** out_results,
	RayResult** results_data
)
{
	enum FireType {
		ONE_ORIGIN,
		ONE_DIRECTION,
		MULTIPLE_RAY
	};
	FireType type;
	if (num_origins == num_directions)
		type = MULTIPLE_RAY;
	else if (num_origins == 1 && num_directions > 1)
		type = ONE_ORIGIN;
	else if (num_origins > 1 && num_directions == 1)
		type = ONE_DIRECTION;
	else {
		fprintf(stderr, "[C++] Invalid input. num_origins = %d, num_directions = %d\n", num_origins, num_directions);
		return HF::Exceptions::GENERIC_ERROR;
	}
	std::vector<RayResult>* output_results;

	switch (type) {
	case MULTIPLE_RAY:
	{
		auto origin_pts = ConvertRawFloatArrayToPoints(origins, num_origins);
		auto direction_pts = ConvertRawFloatArrayToPoints(directions, num_directions);

		output_results = new std::vector<RayResult>(num_origins);

#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < num_origins; i++) {
			float out_distance = -1;
			int out_id = -1;
			if (ert->IntersectOutputArguments(origin_pts[i], direction_pts[i], out_distance, out_id))
				(*output_results)[i].SetHit(origin_pts[i], direction_pts[i], out_distance, out_id);
		}
		break;
	}

	case ONE_ORIGIN:
	{
		std::array<float, 3> origin = { origins[0], origins[1], origins[2] };
		auto direction_pts = ConvertRawFloatArrayToPoints(directions, num_directions);

		output_results = new std::vector<RayResult>(num_origins);
	#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < num_directions; i++) {
			float out_distance = -1; int out_id = -1;
			if (ert->IntersectOutputArguments(origin, direction_pts[i], out_distance, out_id))
				(*output_results)[i].SetHit(origin, direction_pts[i], out_distance, out_id);
		}
		break;
	}
	case ONE_DIRECTION:
	{
		std::array<float, 3> direction = { directions[0], directions[1], directions[2] };
		auto origin_pts = ConvertRawFloatArrayToPoints(origins, num_origins);

		output_results = new std::vector<RayResult>(num_origins);
	#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < num_origins; i++) {
			float out_distance = -1; int out_id = -1;
			if (ert->IntersectOutputArguments(origin_pts[i], direction, out_distance, out_id))
				(*output_results)[i].SetHit(origin_pts[i], direction, out_distance, out_id);

		}
		break;
	}
	}
	
	*out_results = output_results;
	*results_data = output_results->data();
	return OK;
}

C_INTERFACE PointIntersection(EmbreeRayTracer* ert, float& x, float& y, float& z, float dx, float dy, float dz, float max_distance, bool& result)
{
	result = ert->PointIntersection(x, y, z, dx, dy, dz, max_distance);
	return OK;
}

C_INTERFACE FireMultipleRays(
	EmbreeRayTracer* ert,
	float* origins,
	const float* directions,
	int size,
	float max_distance,
	bool* result_array
) {
	auto origin_array = ConvertRawFloatArrayToPoints(origins, size);
	auto dir_array = ConvertRawFloatArrayToPoints(directions, size);
	auto results = ert->PointIntersections(origin_array, dir_array, size, true, max_distance);

	for (int i = 0; i < size; i++) {
		if (results[i])
		{
			const int offset = i * 3;
			const auto& hit_point = origin_array[i];
			origins[offset] = hit_point[0];
			origins[offset + 1] = hit_point[1];
			origins[offset + 2] = hit_point[2];
			result_array[i] = true;
		}
		else
			result_array[i] = false;
	}
	return OK;
}

C_INTERFACE FireMultipleOriginsOneDirection(EmbreeRayTracer* ert, float* origins, const float* direction, int size, float max_distance, bool* result_array)
{
	auto origin_array = ConvertRawFloatArrayToPoints(origins, size);
	auto dir_array = ConvertRawFloatArrayToPoints(direction, 1);
	auto results = ert->PointIntersections(origin_array, dir_array, size, true, max_distance);

	for (int i = 0; i < size; i++) {
		if (results[i])
		{
			const int offset = i * 3;
			const auto& hit_point = origin_array[i];
			origins[offset] = hit_point[0];
			origins[offset + 1] = hit_point[1];
			origins[offset + 2] = hit_point[2];
			result_array[i] = true;
		}
		else
			result_array[i] = false;
	}
	return OK;
}

C_INTERFACE FireMultipleDirectionsOneOrigin(EmbreeRayTracer* ert, const float* origin, float* directions, int size, float max_distance, bool* result_array)
{
	auto origin_array = ConvertRawFloatArrayToPoints(origin, 1);
	auto dir_array = ConvertRawFloatArrayToPoints(directions, size);
	auto results = ert->PointIntersections(origin_array, dir_array, size, true, max_distance);

	for (int i = 0; i < size; i++) {
		if (results[i])
		{
			const int offset = i * 3;
			const auto& hit_point = dir_array[i];
			directions[offset] = hit_point[0];
			directions[offset + 1] = hit_point[1];
			directions[offset + 2] = hit_point[2];
			result_array[i] = true;
		}
		else
			result_array[i] = false;
	}
	return OK;
}

C_INTERFACE Occlusions(EmbreeRayTracer* ert, const float* origins, const float* directions, int origin_size, int direction_size, float max_distance, bool* result_array)
{
	auto origin_array = ConvertRawFloatArrayToPoints(origins, origin_size);
	auto direction_array = ConvertRawFloatArrayToPoints(directions, direction_size);
	const auto results = ert->Occlusions(origin_array, direction_array, max_distance, true);

	std::copy(results.begin(), results.end(), result_array);
	return OK;
}

C_INTERFACE DestroyRayResultVector(std::vector<RayResult>* var) {
	DeleteRawPtr(var);
	return OK;
}

C_INTERFACE PreciseIntersection(
	HF::RayTracer::EmbreeRayTracer* RT,
	double x,
	double y,
	double z, 
	double dx,
	double dy, 
	double dz, 
	double * out_distance)
{

	*out_distance = -1.0;
	HF::RayTracer::HitStruct<double> hs = RT->Intersect(x, y, z, dx, dy, dz, -1.0, -1);
	
	*out_distance = hs.distance;

	return OK;
}
