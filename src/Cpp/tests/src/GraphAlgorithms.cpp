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

#include "analysis_C.h"
#include "graph.h"

namespace HF {

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
		auto GG = GraphGenerator::GraphGenerator(rt, 0);

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
		ASSERT_EQ(g.size(), 29);
	}

}