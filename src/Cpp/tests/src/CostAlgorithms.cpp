#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>

#include "meshinfo.h"
#include "objloader.h"
#include "graph_generator.h"

#include "spatialstructures_C.h"
#include "cost_algorithms.h"

using namespace HF::SpatialStructures;

namespace CostAlgorithmsTests {
	TEST(_CostAlgorithms, CalculateCrossSlope) {
		///
		/// Would prefer to test this function with GraphGenerator and an OBJ file.
		///
		Node n0(-5, 2, 0);
		Node n1(1, 2, 3);
		Node n2(4, 5, 6);
		Node n3(4, 5, 7);
		Node n4(5, 6, 6);
		Node n5(6, 6, 6);
		Node n6(3, 1, 2);
		Node n7(1, 4, 2);
		Node n8(5, 3, 2);

		Graph g;
		// all edges will have a default score, or distance, of 1.0f

		// Note that edges must be added in order of appearance.
		// E.g. you should not add an edge with Node n8 to the graph 
		// without having added an edge with Node n7 first,
		// or else the g.next_id field will be off by however many nodes were skipped
		// in the sequence.
		// which will be very confusing when it is time to debug.

		g.addEdge(n0, n1);
		g.addEdge(n0, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n3, n6);
		g.addEdge(n4, n5);
		g.addEdge(n5, n6);
		g.addEdge(n6, n7);
		g.addEdge(n5, n7);
		g.addEdge(n4, n8);
		g.addEdge(n5, n8);
		g.addEdge(n7, n8);

		g.Compress();

		CSRPtrs csr = g.GetCSRPointers();
		// csr.nnz = 14
		// csr.rows = 9
		// csr.cols = 9

		// csr.inner_indices
		// { 1, 2, 3, 4, 4, 5, 6, 5, 7, 6, 7, 8, 8, 7 }

		for (int i = 0; i < csr.nnz; i++) {
			std::cout << "child id: " << csr.inner_indices[i] << std::endl;
		}

		std::vector<IntEdge> edge_result = CostAlgorithms::CalculateCrossSlope(g);

		for (IntEdge ie : edge_result) {
			std::cout << ie.child << " has weight " << ie.weight << std::endl;
		}
	}

	/*
	TEST(_CostAlgorithms, CalculateCrossSlopeWithEnergyBlob) {
		///
		///	Cannot run this test until GraphGenerator is fixed.
		///
		using HF::Geometry::LoadMeshObjects;
		using HF::RayTracer::EmbreeRayTracer;
		using HF::GraphGenerator::GraphGenerator;
		using HF::SpatialStructures::Graph;

		std::string file_path = "energy_blob.obj";
		bool z_up = true; // LoadMeshObjects makes that param true by default anyway.
		std::vector<HF::Geometry::MeshInfo> mesh = LoadMeshObjects(file_path, HF::Geometry::GROUP_METHOD::ONLY_FILE, z_up);
		mesh[0].PerformRotation(90, 0, 0);

		EmbreeRayTracer ray_tracer(mesh);
		GraphGenerator graph_generator = GraphGenerator(ray_tracer, 0);

		std::array<float, 3> starting_position = { -22.4280376, -12.856843,  5.4826779 };
		std::array<float, 3> spacing = { 10, 10, 70 };
		auto upstep = 10;
		auto downstep = 10;
		auto up_slope = 40;
		auto down_slope = 1;
		auto max_connections_out = 2;
		auto max_nodes = 50;

		auto core_count = 1;

		Graph g = graph_generator.BuildNetwork(starting_position,
			spacing, max_nodes, upstep, up_slope,
			downstep, down_slope, core_count);

		g.Compress();

		std::vector<IntEdge> int_edge = HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope(g);

		CSRPtrs csr = g.GetCSRPointers();
		auto data = csr.data;
		auto col = csr.inner_indices;
		int i = 0;
		for (int parent_id = 0; parent_id < csr.rows; parent_id++) {
			float* row_begin = csr.row_begin(parent_id);
			float* row_curr = row_begin;
			float* row_end = csr.row_end(parent_id);


			while (row_curr < row_end) {
				Node parent = g.NodeFromID(parent_id);
				Node child = g.NodeFromID(col[i]);

				std::cout << "parent ID # " << parent_id << "with pos " 
				          << parent.getArray() << " has child ID # " << col[i]
					      << " at pos " << child.getArray() << " with edge value " 
						  << data[i] << std::endl;
				++row_curr;
				++i;
			}
		}

	}
	*/

	TEST(_CostAlgorithms, CalculateEnergyExpenditure) {
		//
		// Would prefer to test with GraphGenerator and an OBJ file.
		//
		Node n0(1, 2, 1);
		Node n1(1, 2, 3);
		Node n2(4, 5, 6);
		Node n3(4, 6, 7);
		Node n4(5, 6, 6);
		Node n5(6, 7, 6);
		Node n6(3, 1, 2);
		Node n7(1, 4, 2);
		Node n8(5, 3, 2);
		
		Graph g;
		// all edges will have a default score, or distance, of 1.0f

		// Note that edges must be added in order of appearance.
		// E.g. you should not add an edge with Node n8 to the graph 
		// without having added an edge with Node n7 first,
		// or else the g.next_id field will be off by however many nodes were skipped
		// in the sequence.
		// which will be very confusing when it is time to debug.

		g.addEdge(n0, n1);
		
		g.addEdge(n0, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n3, n6);
		g.addEdge(n4, n5);
		g.addEdge(n5, n6);
		g.addEdge(n6, n7);
		g.addEdge(n5, n7);
		g.addEdge(n4, n8);
		g.addEdge(n5, n8);
		g.addEdge(n7, n8);
		
		g.Compress();
	
		std::vector<EdgeSet> edge_set = HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure(g);

		auto edge_num = 0;
		for (auto e : edge_set) {
			for (auto c : e.children) {
				std::cout << "Edge # " << edge_num << "\tparent " << (e.parent) << " has child " << (c.child) << " with weight " << c.weight << std::endl;
				++edge_num;
			}
		}
	}

	/*
	TEST(_CostAlgorithms, CalculateEnergyExpenditureWithEnergyBlob) {
		///
		///	Cannot run this test until GraphGenerator is fixed.
		///
		using HF::Geometry::LoadMeshObjects;
		using HF::RayTracer::EmbreeRayTracer;
		using HF::GraphGenerator::GraphGenerator;
		using HF::SpatialStructures::Graph;

		std::string file_path = "energy_blob_zup.obj";
		std::vector<HF::Geometry::MeshInfo> mesh = LoadMeshObjects(file_path, HF::Geometry::GROUP_METHOD::ONLY_FILE, false);
		//mesh[0].PerformRotation(90, 0, 0);

		//std::cout << mesh[0].getRawVertices()[0] << ", " << mesh[0].getRawVertices()[1] << ", " << mesh[0].getRawVertices()[2] << std::endl;

		EmbreeRayTracer ray_tracer(mesh);
		GraphGenerator graph_generator = GraphGenerator(ray_tracer, 0);

		std::array<float, 3> starting_position = { -22.4280376, -12.856843,  5.4826779 };
		std::array<float, 3> spacing = { 10, 10, 70 };
		auto upstep = 10;
		auto downstep = 10;
		auto up_slope = 40;
		auto down_slope = 1;
		auto max_connections_out = 2;
		auto max_nodes = 50;

		auto core_count = 1;

		Graph g = graph_generator.BuildNetwork(starting_position,
			spacing, max_nodes, upstep, up_slope,
			downstep, down_slope, core_count);

		g.Compress();

		std::vector<EdgeSet> edge_set = HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure(g);

		auto edge_num = 0;
		for (auto e : edge_set) {
			for (auto c : e.children) {
				std::cout << "Edge # " << edge_num << "\tparent " << (e.parent) << " has child " << (c.child) << " with weight " << c.weight << std::endl;
				++edge_num;
			}
		}

	}
	*/
}

namespace CInterfaceTests {
	TEST(_CostAlgorithmsCInterface, CalculateAndStoreCrossSlope) {
		Node n0(-5, 2, 0);
		Node n1(1, 2, 3);
		Node n2(4, 5, 6);
		Node n3(4, 5, 7);
		Node n4(5, 6, 6);
		Node n5(6, 6, 6);
		Node n6(3, 1, 2);
		Node n7(1, 4, 2);
		Node n8(5, 3, 2);

		Graph g;
		// all edges will have a default score, or distance, of 1.0f

		// Note that edges must be added in order of appearance.
		// E.g. you should not add an edge with Node n8 to the graph 
		// without having added an edge with Node n7 first,
		// or else the g.next_id field will be off by however many nodes were skipped
		// in the sequence.
		// which will be very confusing when it is time to debug.

		g.addEdge(n0, n1);
		g.addEdge(n0, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n3, n6);
		g.addEdge(n4, n5);
		g.addEdge(n5, n6);
		g.addEdge(n6, n7);
		g.addEdge(n5, n7);
		g.addEdge(n4, n8);
		g.addEdge(n5, n8);
		g.addEdge(n7, n8);

		g.Compress();

		auto edge_set = g.GetEdges();
		for (auto e : edge_set) {
			for (auto c : e.children) {
				std::cout << "parent " << e.parent << " has child " << c.child << " with weight " << c.weight << std::endl;
			}
		}

		CSRPtrs csr = g.GetCSRPointers();
		// csr.nnz = 14
		// csr.rows = 9
		// csr.cols = 9

		// csr.inner_indices
		// { 1, 2, 3, 4, 4, 5, 6, 5, 7, 6, 7, 8, 8, 7 }

		std::cout << "C interface test (cross slope): " << std::endl;
		CalculateAndStoreCrossSlope(&g);

		for (int i = 0; i < csr.nnz; i++) {
			std::cout << "child id: " << csr.inner_indices[i] << std::endl;
		}

		edge_set = g.GetEdges();
		for (auto e : edge_set) {
			for (auto c : e.children) {
				std::cout << "parent " << e.parent << " has child " << c.child << " with weight " << c.weight << std::endl;
			}
		}
	}

	TEST(_CostAlgorithmsCInterface, CalculateAndStoreEnergyExpenditure) {
		Node n0(5, 2, 3);
		Node n1(1, 2, 3);
		Node n2(4, 5, 6);
		Node n3(4, 5, 7);
		Node n4(5, 6, 6);
		Node n5(6, 6, 6);
		Node n6(3, 1, 2);
		Node n7(1, 4, 2);
		Node n8(5, 3, 2);

		Graph g;
		// all edges will have a default score, or distance, of 1.0f

		// Note that edges must be added in order of appearance.
		// E.g. you should not add an edge with Node n8 to the graph 
		// without having added an edge with Node n7 first,
		// or else the g.next_id field will be off by however many nodes were skipped
		// in the sequence.
		// which will be very confusing when it is time to debug.

		g.addEdge(n0, n1);
		g.addEdge(n0, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n3, n6);
		g.addEdge(n4, n5);
		g.addEdge(n5, n6);
		g.addEdge(n6, n7);
		g.addEdge(n5, n7);
		g.addEdge(n4, n8);
		g.addEdge(n5, n8);
		g.addEdge(n7, n8);

		g.Compress();

		auto edge_set = g.GetEdges();
		for (auto e : edge_set) {
			for (auto c : e.children) {
				std::cout << "parent " << e.parent << " has child " << c.child << " with weight " << c.weight << std::endl;
			}
		}

		CSRPtrs csr = g.GetCSRPointers();
		// csr.nnz = 14
		// csr.rows = 9
		// csr.cols = 9

		// csr.inner_indices
		// { 1, 2, 3, 4, 4, 5, 6, 5, 7, 6, 7, 8, 8, 7 }

		//std::cout << "C interface test (EnergyExpenditure): " << std::endl;
		//CalculateAndStoreEnergyExpenditure(&g);

		for (int i = 0; i < csr.nnz; i++) {
			std::cout << "child id: " << csr.inner_indices[i] << std::endl;
		}

		edge_set = g.GetEdges();
		for (auto e : edge_set) {
			for (auto c : e.children) {
				std::cout << "parent " << e.parent << " has child " << c.child << " with weight " << c.weight << std::endl;
			}
		}
	}
}
