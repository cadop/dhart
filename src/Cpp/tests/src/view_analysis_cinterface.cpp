/*!
	\file		view_analysis_cinterface.cpp
	\brief		Unit test source file for C interface functions related to view analysis

	\author		TBA
	\date		11 Aug 2020
*/

#include "gtest/gtest.h"

#include "view_analysis_C.h"
#include "objloader_C.h"

#include "node.h"			// Required for definition of struct HF::SpatialStructures::Node
#include "HFExceptions.h"	// Required for definition of HF::Exceptions::HF_STATUS

using HF::Geometry::MeshInfo;
using HF::RayTracer::EmbreeRayTracer;
using HF::SpatialStructures::Node;

namespace CInterfaceTests {
	TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisAggregate) {
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);
		ASSERT_EQ(MIR, HF::Exceptions::HF_STATUS::OK);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<Node> nodes = {
			Node(0,0,1),
			Node(0,0,2),
			Node(0,0,3),
		};

		// Make settings
		float max_rays = 10000;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<float>* scores;
		float* scores_ptr;
		int scores_size;

		// Run View Analysis
		auto result = SphereicalViewAnalysisAggregate(
			ert,
			nodes.data(),
			nodes.size(),
			max_rays,
			up_fov,
			down_fov,
			height,
			AT,
			&scores,
			&scores_ptr,
			&scores_size
		);
		ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);
		ASSERT_EQ(scores->size(), scores_size);

		// print Results
		for (int i = 0; i < scores->size(); i++)
			std::cerr << (*scores)[i] << std::endl;

		// Deallocate Memory
		DestroyFloatVector(scores);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
	}

	TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisAggregateFlat) {
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);
		ASSERT_EQ(MIR, HF::Exceptions::HF_STATUS::OK);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<float> nodes = {
			0,0,1,
			0,0,2,
			0,0,3,
		};

		// Make settings
		float max_rays = 10000;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<float>* scores;
		float* scores_ptr;
		int scores_size;

		// Run View Analysis
		auto result = SphereicalViewAnalysisAggregateFlat(
			ert,
			nodes.data(),
			nodes.size() / 3,
			max_rays,
			up_fov,
			down_fov,
			height,
			AT,
			&scores,
			&scores_ptr,
			&scores_size
		);
		ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);
		ASSERT_EQ(scores->size(), scores_size);

		// print Results
		for (int i = 0; i < scores->size(); i++)
			std::cerr << (*scores)[i] << std::endl;

		// Deallocate Memory
		DestroyFloatVector(scores);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
	}

	TEST(C_ViewAnalysisCInterface, SphericalDistribute)
	{
		int num_rays = 10;
		std::vector<float>* out_float;
		float* out_float_data;
		float up_fov = 90.0f;
		float down_fov = 90.0f;

		auto status = SphericalDistribute(
			&num_rays,
			&out_float,
			&out_float_data,
			up_fov,
			down_fov
		);
		ASSERT_EQ(status, HF::Exceptions::HF_STATUS::OK);

		std::cerr << "Number of rays: " << num_rays << std::endl;
		for (int i = 0; i < num_rays; i++) {
			int os = i * 3;

			std::cerr << "("
				<< out_float_data[os] << ", "
				<< out_float_data[os + 1] << ", "
				<< out_float_data[os + 2] << ")"
				<< std::endl;
		}

		DestroyFloatVector(out_float);
	}


	TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisNoAggregateFlat) {
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<float> nodes = {
			0,0,1,
			0,0,2,
			0,0,3,
		};

		// Make settings
		int max_rays = 10;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<RayResult>* results;
		RayResult* results_ptr;

		// Run View Analysis
		auto result = SphericalViewAnalysisNoAggregateFlat(
			ert,
			nodes.data(),
			nodes.size() / 3,
			&max_rays,
			up_fov,
			down_fov,
			height,
			&results,
			&results_ptr
		);
		ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);

		// print Results
		std::cerr << "Num Rays: " << max_rays << std::endl;
		for (int i = 0; i < nodes.size() / 3; i++) {
			std::cerr << "Node " << i << ": ";
			for (int k = 0; k < results->size() / 3; k++) {
				int os = max_rays * i;
				std::cerr << "(" << results_ptr[k + os].meshid
					<< ", " << results_ptr[k + os].distance << "), ";
			}
			std::cerr << std::endl;
		}

		std::cerr << std::endl;

		// Deallocate Memory
		DestroyRayResultVector(results);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
	}

	TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisNoAggregate) {
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<Node> nodes = {
			Node(0,0,1),
			Node(0,0,2),
			Node(0,0,3),
		};

		// Make settings
		int max_rays = 10;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<RayResult>* results;
		RayResult* results_ptr;

		// Run View Analysis
		auto result = SphericalViewAnalysisNoAggregate(
			ert,
			nodes.data(),
			nodes.size(),
			&max_rays,
			up_fov,
			down_fov,
			height,
			&results,
			&results_ptr
		);
		ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);

		// print Results
		std::cerr << "Num Rays: " << max_rays << std::endl;
		for (int i = 0; i < nodes.size(); i++) {
			std::cerr << "Node " << i << ": ";
			for (int k = 0; k < results->size() / 3; k++) {
				int os = max_rays * i;
				std::cerr << "(" << results_ptr[k + os].meshid
					<< ", " << results_ptr[k + os].distance << "), ";
			}
			std::cerr << std::endl;
		}

		std::cerr << std::endl;

		// Deallocate Memory
		DestroyRayResultVector(results);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
	}
}