#include "gtest/gtest.h"
#include <array>
#include <graph_generator.h>
#include <unique_queue.h>
#include <embree_raytracer.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <edge.h>
#include <node.h>
#include <Constants.h>
#include<ray_data.h>

#include "analysis_C.h"
#include "graph.h"

namespace HF {

	void CheckForDuplicatesNano(const HF::SpatialStructures::Graph& g, float precision = 0.0001f, bool insta_fail = false) {

		// Get nodes from the graph
		const auto nodes = g.Nodes();

		// Setup variables
		int duplicate_pairs = 0;
		int duplicates_this_node = 0;
		int duplicate_nodes = 0;

		// Iterate through every node in the graph
		for (const auto& node : nodes) {
			--duplicate_pairs; // subrtact one because this node will be compared to itself atleast once
			duplicates_this_node = 0;

			// Iterate through every other node in the graph
			for (const auto& node2 : nodes)
			{
				// Check if this node closer to node2 than our precision value
				if (node.distanceTo(node2) < precision) {

					// If it is, increment duplciate pairs
					++duplicate_pairs;
					++duplicates_this_node;

					// If this has had 2 duplicates (itself and atleast one other node) then it is a duplicate and duplicates
					// should be incremented
					if (duplicates_this_node == 2)
						if (insta_fail)
							FAIL("Duplicate nodes detected");
						else
							duplicate_nodes++;
					
				}
			}
		}


		// Divide duplicate nodes by 2, since both duplicates would increment this counter
		// and technically only one is the original
		duplicate_nodes /= 2;
		int total_pairs = g.size() * g.size() - 1;
		float duplicate_node_percentage = (static_cast<float>(duplicate_nodes) / static_cast<float>(g.size())) * 100.00f;
		float duplicate_pair_percentage = (static_cast<float>(duplicate_pairs) / static_cast<float>(g.size() * g.size())) * 100.00f;

		// print to console	
		std::cerr << duplicate_nodes << "/" << g.size() << "(" << duplicate_node_percentage << "\%) Duplicate Nodes" << std::endl;
		std::cerr << duplicate_pairs << "/" << total_pairs << "(" << duplicate_pair_percentage << "\%) Duplicate Pairs" << std::endl;

		// Fail if there are duplicate pairs
		EXPECT_EQ(0, duplicate_pairs);
	}
	TEST(_GraphAlgorithm, BasicGenerator) {
		//auto mesh = Geometry::LoadMeshObjects("plane.obj");
		auto mesh = Geometry::LoadMeshObjects("energy_blob_zup.obj");
		/*
		std::vector<float> mesh_verts = mesh[0].getRawVertices();
		std::vector<std::array<float, 3>> mesh_arr = mesh[0].GetVertsAsArrays();

		std::cout << "Before Rot, First 3 Raw vertices: " << mesh_verts[0] << ", " << mesh_verts[1] << ", " << mesh_verts[2] << std::endl;
		std::cout << "Before Rot, First 3 vertices in array: " << mesh_arr[0] << std::endl << std::endl;

		mesh[0].PerformRotation(-90, 0, 0);

		std::vector<float> mesh_verts_rot = mesh[0].getRawVertices();
		std::vector<std::array<float, 3>> mesh_arr_rot = mesh[0].GetVertsAsArrays();

		std::cout << "First 3 Raw vertices: " << mesh_verts_rot[0] << ", " << mesh_verts_rot[1] << ", " << mesh_verts_rot[2] << std::endl;
		std::cout << "First 3 vertices in array: " << mesh_arr_rot[0] << std::endl;
		*/


		/*
		Before Rot, First 3 Raw vertices: -20.0794, -18.9406, -8.27922e-07
		Before Rot, First 3 vertices in array: (20.1406,18.8423,8.23625e-07)

		First 3 Raw vertices: -20.0794, 1.42998e-06, 18.9406
		First 3 vertices in array: (20.1406,-1.42256e-06,-18.8424)
		*/


		RayTracer::EmbreeRayTracer rt(mesh);
		auto GG = GraphGenerator::GraphGenerator(rt);

		auto g = GG.BuildNetwork(
			std::array<float, 3>{-22.4280376, -12.856843, 5.4826779},
			std::array<double, 3>{10.0, 10.0, 70.0},
			50,
			10,
			40,
			10,
			1,
			2
		);

		printf("Graph size %i\n", g.size());
		g.Compress();
		ASSERT_EQ(29, g.size());
	}

	TEST(_GraphAlgorithm, Energy_Blob) {
		auto mesh = Geometry::LoadMeshObjects("energy_blob_zup.obj");

		RayTracer::EmbreeRayTracer rt(mesh, true);
		auto GG = GraphGenerator::GraphGenerator(rt);
		int max_nodes = 5000;
		double up_step = 5;
		double up_slope = 60;
		double down_step = 5;
		double down_slope = 60;
		int max_step_connections = 1;
		int cores = -1;


		auto g = GG.BuildNetwork(
			std::array<double, 3>{-30.0, 0.0, 20.0},
			std::array<double, 3>{1.0, 1.0, 10.0},
			max_nodes,
			up_step,
			up_slope,
			down_step,
			down_slope,
			max_step_connections,
			cores
		);

		printf("Graph size %i\n", g.size());
		g.Compress();
		ASSERT_EQ(3450, g.size());
	}

	TEST(_GraphAlgorithm, Energy_BlobNanoRT) {
		auto mesh = Geometry::LoadMeshObjects("energy_blob_zup.obj");

		HF::RayTracer::NanoRTRayTracer nrt(mesh[0]);
		auto GG = GraphGenerator::GraphGenerator(nrt);
		int max_nodes = 5000;
		double up_step = 5;
		double up_slope = 60;
		double down_step = 5;
		double down_slope = 60;
		int max_step_connections = 1;
		int cores = -1;


		auto g = GG.BuildNetwork(
			std::array<double, 3>{-30.0, 0.0, 20.0},
			std::array<double, 3>{1.0, 1.0, 10.0},
			max_nodes,
			up_step,
			up_slope,
			down_step,
			down_slope,
			max_step_connections,
			cores
		);

		printf("Graph size %i\n", g.size());
		g.Compress();
		ASSERT_EQ(3450, g.size());

	}

	TEST(_GraphAlgorithm, NanoRTVisibilityGraphTC) {
		auto mesh = Geometry::LoadMeshObjects("VisibilityTestCases.obj");
		auto double_mesh = Geometry::LoadTMPMeshObjects<double>("VisibilityTestCases.obj");

		HF::RayTracer::NanoRTRayTracer nrt(double_mesh);
		GraphGenerator::GraphGenerator NanoRTGraphGen(nrt);
		
		HF::RayTracer::EmbreeRayTracer ert(mesh[0], true);
		GraphGenerator::GraphGenerator EmbreeGraphGen(ert);

		int max_nodes = 5000;
		double up_step = 1;	double up_slope = 1;
		double down_step = 1;	double down_slope = 1;
		int max_step_connections = 1;
		int cores = -1;
		std::array<double, 3> start_point{ 1.0, 1.0, 20.0};
		std::array<double, 3> spacing{ 1, 1, 15};

		auto EmbreeGraph = EmbreeGraphGen.BuildNetwork(
			start_point, spacing,
			max_nodes, up_step, up_slope,
			down_step,	down_slope,	max_step_connections,
			cores
		);

		auto NanoGraph = NanoRTGraphGen.BuildNetwork(
			start_point, spacing,
			max_nodes, up_step, up_slope,
			down_step, down_slope, max_step_connections,
			cores
		);

		EmbreeGraph.Compress();
		NanoGraph.Compress();

		printf("Embree Size: %i, NanoSize %i\n", EmbreeGraph.size(), NanoGraph.size());

		//CheckForDuplicatesNano(NanoGraph, 0.1f);
		NanoGraph.DumpToJson("NanoRtGraph.json");
		EmbreeGraph.DumpToJson("EmbreeGraph.json");
		CheckForDuplicatesNano(NanoGraph);

		ASSERT_LT(NanoGraph.size(), EmbreeGraph.size());

	}

	TEST(_GraphAlgorithm, VsEmbree_blob) {
		std::string mesh_path = "energy_blob_zup.obj";

		auto mesh = Geometry::LoadMeshObjects(mesh_path);
		auto double_mesh = Geometry::LoadTMPMeshObjects<double>(mesh_path);

		HF::RayTracer::NanoRTRayTracer nrt(double_mesh);
		GraphGenerator::GraphGenerator NanoRTGraphGen(nrt);

		HF::RayTracer::EmbreeRayTracer ert(mesh[0], true);
		GraphGenerator::GraphGenerator EmbreeGraphGen(ert);

		int max_nodes = 50000;
		double up_step = 50;	double up_slope = 45;
		double down_step = 50;	double down_slope = 45;
		int max_step_connections = 2;
		int cores = -1;
		std::array<double, 3> start_point{ -22.42, -12.0, 5.0};
		std::array<double, 3> spacing{ 1.0, 1.0, 10.0 };				
		auto EmbreeGraph = EmbreeGraphGen.BuildNetwork(
			start_point, spacing,
			max_nodes, up_step, up_slope,
			down_step, down_slope, max_step_connections,
			cores
		);
		auto NanoGraph = NanoRTGraphGen.BuildNetwork(
			start_point, spacing,
			max_nodes, up_step, up_slope,
			down_step, down_slope, max_step_connections,
			cores
		);

		EmbreeGraph.Compress();
		NanoGraph.Compress();

		printf("Embree Size: %i, NanoSize %i\n", EmbreeGraph.size(), NanoGraph.size());

		//CheckForDuplicatesNano(NanoGraph, 0.1f);
		NanoGraph.DumpToJson("NanoRtGraph.json");
		EmbreeGraph.DumpToJson("EmbreeGraph.json");
		CheckForDuplicatesNano(NanoGraph, 0.01f, true);


	}



}