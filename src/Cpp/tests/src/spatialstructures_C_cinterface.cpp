/*!
	\file		spatialstructures_C_cinterface.cpp
	\brief		Unit test source file for functions tested from spatialstructures_C.h

	\author		TBA
	\date		19 Aug 2020
*/

#include "gtest/gtest.h"

#include "graph.h"
#include "constants.h"
#include "HFExceptions.h"

#include "spatialstructures_C.h"

namespace CInterfaceTests {
	TEST(_spatialstructures_cinterface, AlgorithmCostTitle) {
		//! [snippet_spatialstructuresC_AlgorithmCostTitle]
		// In the enum COST_ALG_KEY, there are two members:
		// COST_ALG_KEY::CROSS_SLOPE, and COST_ALG_KEY::ENERGY_EXPENDITURE.

		// If we want a string representation of COST_ALG_KEY::CROSS_SLOPE,
		// we invoke AlgorithmCostTitle:

		auto cost_str = AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE);
		std::cout << "Cost type: " << cost_str << std::endl;
		//! [snippet_spatialstructuresC_AlgorithmCostTitle]
	}

	TEST(_spatialstructures_cinterface, GetAllNodesFromGraph) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_GetAllNodesFromGraph]
		// Create three nodes, in the form of { x, y, z } coordinates
		std::array<float, 3> n0 { 0, 0, 0 };
		std::array<float, 3> n1 { 0, 1, 2 };
		std::array<float, 3> n2 { 0, 1, 3 };

		// Leave cost_type blank. We do not need a specific cost type now.
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), 1, cost_type);
		status = AddEdgeFromNodes(g, n0.data(), n2.data(), 2, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n0.data(), 3, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n2.data(), 4, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n0.data(), 5, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n1.data(), 6, cost_type);

		// GetNodesFromGraph will allocate memory for out_vec.
		// out_data will point to *out_vec's internal buffer.
		std::vector<HF::SpatialStructures::Node>* out_vec = nullptr;
		HF::SpatialStructures::Node* out_data = nullptr;

		status = GetAllNodesFromGraph(g, &out_vec, &out_data);
		
		// Destroy vector<Node>
		status = DestroyNodes(out_vec);
		//! [snippet_spatialstructuresC_GetAllNodesFromGraph]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, GetEdgesForNode) {
		//! [snippet_spatialstructuresC_GetEdgesForNode]
		// TODO example
		//! [snippet_spatialstructuresC_GetEdgesForNode]
	}

	TEST(_spatialstructures_cinterface, GetSizeOfNodeVector) {
		//! [snippet_spatialstructuresC_GetSizeOfNodeVector]
		// Requires #include "node.h", #include <vector>

		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Since GetSizeOfNodeVector only operators on a vector<Node>,
		// we can construct from free nodes and put them into a container.
		// GetSizeOfNodeVector will return the size of *node_vec below.
		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		std::vector<HF::SpatialStructures::Node>* node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		int node_vec_size = -1;
		status = GetSizeOfNodeVector(node_vec, &node_vec_size);

		// node_vec_size will have the size of *node_vec.
		//! [snippet_spatialstructuresC_GetSizeOfNodeVector]
		// Destroy vector<Node>
		status = DestroyNodes(node_vec);

		if (status != 1) {
			std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyNodes ran successfully on address " << node_vec << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, GetSizeOfEdgeVector) {
		//! [snippet_spatialstructuresC_GetSizeOfEdgeVector]
		// TODO example
		//! [snippet_spatialstructuresC_GetSizeOfEdgeVector]
	}

	TEST(_spatialstructures_cinterface, GetEdgeCost) {
		//! [snippet_spatialstructuresC_GetEdgeCost]
		// TODO example
		//! [snippet_spatialstructuresC_GetEdgeCost]
	}

	TEST(_spatialstructures_cinterface, AggregateCosts) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_AggregateCosts]
		// Create three nodes in the form of { x, y, z } coordinates.
		std::array<float, 3> n0 { 0, 0, 0 };
		std::array<float, 3> n1 { 0, 1, 2 };
		std::array<float, 3> n2 { 0, 1, 3 };

		// We do not need a specific cost type now. Leave this as an empty string.
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), 1, cost_type);
		status = AddEdgeFromNodes(g, n0.data(), n2.data(), 2, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n0.data(), 3, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n2.data(), 4, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n0.data(), 5, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n1.data(), 6, cost_type);

		// AggregateCosts will allocate memory for out_vector.
		// out_data will point to *out_vector's internal buffer.
		std::vector<float>* out_vector = nullptr;
		float* out_data = nullptr;
		int aggregation_type = 0;

		status = AggregateCosts(g, 
								aggregation_type, false, 
								cost_type, 
								&out_vector, &out_data);

		// Destroy vector<float>
		status = DestroyFloatVector(out_vector);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyFloatVector, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyFloatVector ran successfully on address " << g << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_AggregateCosts]

		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, CreateGraph) {
		//! [snippet_spatialstructuresC_CreateGraph]
		// Requires #include "graph.h"

		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//
		// use Graph
		//
		//! [snippet_spatialstructuresC_CreateGraph]
		// Destroy graph
		status = DestroyGraph(g);
		
		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, AddEdgeFromNodes) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_AddEdgeFromNodes]
		// Create two nodes, in the form of coordinates
		std::array<float, 3> n0 = { 0, 0, 0 };
		std::array<float, 3> n1 = { 0, 1, 2 };

		// Define an edge weight
		const float edge_weight = 3;

		// Define a cost type.
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), edge_weight, cost_type);

		//! [snippet_spatialstructuresC_AddEdgeFromNodes]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, AddEdgeFromNodeIDs) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_AddEdgeFromNodeIDs]
		const int node_id_0 = 0;
		const int node_id_1 = 1;
		const float edge_weight = 3;

		const char* cost_type = "";

		status = AddEdgeFromNodeIDs(g, node_id_0, node_id_1, edge_weight, cost_type);
		//! [snippet_spatialstructuresC_AddEdgeFromNodeIDs]

		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, GetCSRPointers) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_GetCSRPointers]
		// Create three nodes in the form of { x, y, z } coordinates.
		std::array<float, 3> n0 { 0, 0, 0 };
		std::array<float, 3> n1 { 0, 1, 2 };
		std::array<float, 3> n2 { 0, 1, 3 };

		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), 1, cost_type);
		status = AddEdgeFromNodes(g, n0.data(), n2.data(), 2, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n0.data(), 3, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n2.data(), 4, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n0.data(), 5, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n1.data(), 6, cost_type);

		status = Compress(g);

		if (status != 1) {
			std::cerr << "Error at Compress, code: " << status << std::endl;
		}
		else {
			std::cout << "Compress ran successfully on graph addressed at " << g << ", code: " << status << std::endl;
		}

		// Retrieve the CSR from the graph
		HF::SpatialStructures::CSRPtrs csr;
		status = GetCSRPointers(g, &csr.nnz, &csr.rows, &csr.cols, &csr.data, &csr.inner_indices, &csr.outer_indices, cost_type);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		if (status != 1) {
			std::cerr << "Error at GetCSRPointers, code: " << status << std::endl;
		}
		else {
			std::cout << "GetCSRPointers ran successfully on graph addressed at " << g << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_GetCSRPointers]

		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, GetNodeID) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_GetNodeID]
		// Create two nodes in the form of { x, y, z } coordinates.
		std::array<float, 3> n0 { 0, 0, 0 };
		std::array<float, 3> n1 { 0, 1, 2 };

		const float edge_weight = 3;
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), edge_weight, cost_type);

		// Determine the coordinates of the node whose ID you want to retrieve. result_id will store the retrieved ID.
		std::array<float, 3> point { 0, 1, 2 };
		int result_id = -1;

		status = GetNodeID(g, point.data(), &result_id);
		//! [snippet_spatialstructuresC_GetNodeID]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, Compress) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_Compress]
		// Create three nodes in the form of { x, y, z } coordinates.
		std::array<float, 3> n0{ 0, 0, 0 };
		std::array<float, 3> n1{ 0, 1, 2 };
		std::array<float, 3> n2{ 0, 1, 3 };

		// We do not need to mind the cost type for now.
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), 1, cost_type);
		status = AddEdgeFromNodes(g, n0.data(), n2.data(), 2, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n0.data(), 3, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n2.data(), 4, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n0.data(), 5, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n1.data(), 6, cost_type);

		// Any time edges are added to a graph, the graph must be compressed.
		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// rows = { 0, 2, 4 }
		// columns = { 1, 2, 0, 2, 0, 1 }
		//! [snippet_spatialstructuresC_Compress]

		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_Compress]
	}

	TEST(_spatialstructures_cinterface, ClearGraph) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_ClearGraph]
		// Create two nodes in the form of { x, y, z } coordinates.
		std::array<float, 3> n0{ 0, 0, 0 };
		std::array<float, 3> n1{ 0, 1, 2 };

		const float edge_weight = 3;
		const char* cost_type = "";

		AddEdgeFromNodes(g, n0.data(), n1.data(), edge_weight, cost_type);

		ClearGraph(g, cost_type);

		// The graph will now have a node count of 0.
		//! [snippet_spatialstructuresC_ClearGraph]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, DestroyNodes) {
		//! [snippet_spatialstructuresC_DestroyNodes]
		// Requires #include "node.h", #include <vector>

		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		// Use node_vec

		// Destroy vector<Node>
		status = DestroyNodes(node_vec);

		if (status != 1) {
			std::cerr << "Error at DestroyNodes, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyNodes ran successfully on address " << node_vec << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_DestroyNodes]
	}

	TEST(_spatialstructures_cinterface, DestroyEdges) {
		//! [snippet_spatialstructuresC_DestroyEdges]
		// Requires #include "node.h", #include <vector>

		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		HF::SpatialStructures::Edge e0(n1); // parent is n0
		HF::SpatialStructures::Edge e1(n3); // parent is n2

		auto edge_vec = new std::vector<HF::SpatialStructures::Edge>{ e0, e1 };

		// Use edge_vec

		// Destroy vector<Edge>
		status = DestroyEdges(edge_vec);

		if (status != 1) {
			std::cerr << "Error at DestroyEdges, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyEdges ran successfully on address " << edge_vec << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_DestroyEdges]
	}

	TEST(_spatialstructures_cinterface, DestroyGraph) {
		//! [snippet_spatialstructuresC_DestroyGraph]
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		// use Graph

		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
		//! [snippet_spatialstructuresC_DestroyGraph]
	}

	TEST(_spatialstructures_cinterface, CalculateAndStoreCrossSlope) {
		// Requires #include "graph.h"

		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_CalculateAndStoreCrossSlope]
		// Create 7 nodes, in the form of {x, y, z}
		std::array<float, 3> n0{ 0, 0, 0 };
		std::array<float, 3> n1{ 1, 3, 5 };
		std::array<float, 3> n2{ 3, -1, 2 };
		std::array<float, 3> n3{ 1, 2, 1 };
		std::array<float, 3> n4{ 4, 5, 7 };
		std::array<float, 3> n5{ 5, 3, 2 };
		std::array<float, 3> n6{ -2, -5, 1 };

		// Retrieve the cost type string
		const char* cost_type = AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE).c_str();

		AddEdgeFromNodes(g, n0.data(), n1.data(), 0, cost_type);
		AddEdgeFromNodes(g, n1.data(), n2.data(), 0, cost_type);
		AddEdgeFromNodes(g, n1.data(), n3.data(), 0, cost_type);
		AddEdgeFromNodes(g, n1.data(), n4.data(), 0, cost_type);
		AddEdgeFromNodes(g, n2.data(), n4.data(), 0, cost_type);
		AddEdgeFromNodes(g, n3.data(), n5.data(), 0, cost_type);
		AddEdgeFromNodes(g, n5.data(), n6.data(), 0, cost_type);
		AddEdgeFromNodes(g, n4.data(), n6.data(), 0, cost_type);

		// Always compress the graph after adding edges!
		status = Compress(g);

		// Cross Slope will be calculated and stored within edges
		CalculateAndStoreCrossSlope(g);
		//! [snippet_spatialstructuresC_CalculateAndStoreCrossSlope]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, CalculateAndStoreEnergyExpenditure) {
		// Requires #include "graph.h"

		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_CalculateAndStoreEnergyExpenditure]
		// Create 7 nodes, in the form of {x, y, z}
		std::array<float, 3> n0{ 0, 0, 0 };
		std::array<float, 3> n1{ 1, 3, 5 };
		std::array<float, 3> n2{ 3, -1, 2 };
		std::array<float, 3> n3{ 1, 2, 1 };
		std::array<float, 3> n4{ 4, 5, 7 };
		std::array<float, 3> n5{ 5, 3, 2 };
		std::array<float, 3> n6{ -2, -5, 1 };

		// Retrieve the cost type string
		const char* cost_type = AlgorithmCostTitle(COST_ALG_KEY::CROSS_SLOPE).c_str();

		AddEdgeFromNodes(g, n0.data(), n1.data(), 0, cost_type);
		AddEdgeFromNodes(g, n1.data(), n2.data(), 0, cost_type);
		AddEdgeFromNodes(g, n1.data(), n3.data(), 0, cost_type);
		AddEdgeFromNodes(g, n1.data(), n4.data(), 0, cost_type);
		AddEdgeFromNodes(g, n2.data(), n4.data(), 0, cost_type);
		AddEdgeFromNodes(g, n3.data(), n5.data(), 0, cost_type);
		AddEdgeFromNodes(g, n5.data(), n6.data(), 0, cost_type);
		AddEdgeFromNodes(g, n4.data(), n6.data(), 0, cost_type);

		// Always compress the graph after adding edges!
		status = Compress(g);

		// Energy Expenditure will be calculated and stored within edges
		CalculateAndStoreEnergyExpenditure(g);
		//! [snippet_spatialstructuresC_CalculateAndStoreEnergyExpenditure]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, AddNodeAttributes) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		//! [snippet_spatialstructuresC_AddNodeAttributes]
		// We do not need edge cost type now.
		const char* cost_type = "";

		// Add edges by node IDs.
		AddEdgeFromNodeIDs(g, 0, 1, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 0, 2, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 1, 3, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 1, 4, 1, cost_type);
		AddEdgeFromNodeIDs(g, 2, 4, 1, cost_type);
		AddEdgeFromNodeIDs(g, 3, 5, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 3, 6, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 4, 5, 1, cost_type);
		AddEdgeFromNodeIDs(g, 5, 6, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 5, 7, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 5, 8, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 4, 8, 1, cost_type);
		AddEdgeFromNodeIDs(g, 6, 7, 1, cost_type);	
		AddEdgeFromNodeIDs(g, 7, 8, 1, cost_type);

		// Add some node attributes
		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };

		AddNodeAttributes(g, ids.data(), attr_type.c_str(), scores, ids.size());
		//! [snippet_spatialstructuresC_AddNodeAttributes]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}

	TEST(_spatialstructures_cinterface, GetNodeAttributes) {
		//! [snippet_spatialstructuresC_GetNodeAttributes]
		// Create a graph and add edges
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1); g.addEdge(1, 3, 1); g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1); g.addEdge(3, 5, 1); g.addEdge(3, 6, 1); g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1); g.addEdge(5, 7, 1); g.addEdge(5, 8, 1); g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Create a vector of node IDs and their corresponding values for our attribute
		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };

		// Add node attributes to the graph
		AddNodeAttributes(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		// Allocate an array of char arrays to meet the preconditions of GetNodeAttributes
		char** scores_out = new char* [g.size()];
		int scores_out_size = 0;

		// By the postconditions of GetNodeAttributes, this should update scores_out,
		// and scores_out_size with the variables we need
		GetNodeAttributes(&g, attr_type.c_str(), scores_out, &scores_out_size);

		// Assert that we can get the scores from this array
		for (int i = 0; i < scores_out_size; i++)
		{
			// Convert score at this index to a string. 
			std::string score = scores_out[i];

			// If it's in our input array, ensure that the score at this value
			// matches the one we passed
			auto itr = std::find(ids.begin(), ids.end(), i);
			if (itr != ids.end())
			{
				// Get the index of the id in the scores array so we
				// can compare use it to get our input score at that
				// index as well.
				int index = std::distance(ids.begin(), itr);
			}

		}
		
		// Resource cleanup
		DeleteScoreArray(scores_out, g.size());
		//! [snippet_spatialstructuresC_GetNodeAttributes]
	}

	TEST(_spatialstructures_cinterface, DeleteScoreArray) {
		//! [snippet_spatialstructuresC_DeleteScoreArray]

		//! [snippet_spatialstructuresC_DeleteScoreArray]
	}

	TEST(_spatialstructures_cinterface, ClearAttributeType) {
		//! [snippet_spatialstructuresC_ClearAttributeType]
		// Create a graph and add some edges.
		HF::SpatialStructures::Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Create score arrays, then assign them to the graph
		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };
		AddNodeAttributes(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		// Clear the attribute type and capture the error code
		auto res = ClearAttributeType(&g, attr_type.c_str());
		//! [snippet_spatialstructuresC_ClearAttributeType]
	}

	TEST(_spatialstructures_cinterface, GetSizeOfGraph) {
		// Status code variable, value returned by C Interface functions
		// See documentation for HF::Exceptions::HF_STATUS for error code definitions.
		int status = 0;

		// Declare a pointer to Graph.
		// This will point to memory on the free store;
		// it will be allocated within CreateGraph.
		// It is the caller's responsibility to call DestroyGraph on g.
		HF::SpatialStructures::Graph* g = nullptr;

		// The first two parameters are unused, according to documentation.
		status = CreateGraph(nullptr, -1, &g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at CreateGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "CreateGraph ran successfully, graph is at address " << g << ", code: " << status << std::endl;
		}

		//! [snippet_spatialstructuresC_GetSizeOfGraph]
		// Create three nodes in the form of { x, y, z } coordinates.
		std::array<float, 3> n0{ 0, 0, 0 };
		std::array<float, 3> n1{ 0, 1, 2 };
		std::array<float, 3> n2{ 0, 1, 3 };

		// We do not need a specific cost type now. Leave this as an empty string.
		const char* cost_type = "";

		status = AddEdgeFromNodes(g, n0.data(), n1.data(), 1, cost_type);
		status = AddEdgeFromNodes(g, n0.data(), n2.data(), 2, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n0.data(), 3, cost_type);
		status = AddEdgeFromNodes(g, n1.data(), n2.data(), 4, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n0.data(), 5, cost_type);
		status = AddEdgeFromNodes(g, n2.data(), n1.data(), 6, cost_type);

		int graph_size = -1;
		status = GetSizeOfGraph(g, &graph_size);

		// graph_size will have the node count (graph size)
		//! [snippet_spatialstructuresC_GetSizeOfGraph]
		// Destroy graph
		status = DestroyGraph(g);

		if (status != 1) {
			// Error!
			std::cerr << "Error at DestroyGraph, code: " << status << std::endl;
		}
		else {
			std::cout << "DestroyGraph ran successfully on address " << g << ", code: " << status << std::endl;
		}
	}
}
