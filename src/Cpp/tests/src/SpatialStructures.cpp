#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>

using namespace HF::SpatialStructures;
namespace GraphTests {
    TEST(_Graph, Creation) {
        HF::SpatialStructures::Graph g;
        ASSERT_EQ(g.size(), 0);
    }

    TEST(_Graph, AddNodeTest) {
        HF::SpatialStructures::Graph g;
        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        ASSERT_TRUE(g.hasKey(N1));
    }

    TEST(_Graph, HasKeyFailTest) {
        HF::SpatialStructures::Graph g;
        
        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);
        
        g.addEdge(N1, N2);
        g.Compress();
        EXPECT_TRUE(g.size() == 2);
        ASSERT_TRUE(g.hasKey(N2));
        ASSERT_TRUE(g.hasKey(N1));
    }
    
    TEST(_Graph, GetUndirectedEdges) {
        HF::SpatialStructures::Graph g;
        
        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();

        ASSERT_EQ(g.GetUndirectedEdges(N2).size(), 1);
        ASSERT_EQ(g[N2].size(), 0);
    }

    TEST(_Graph, HasEdgeTrue) {
        HF::SpatialStructures::Graph g;
        
        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();
        ASSERT_TRUE(g.HasEdge(N1, N2));
    }

    TEST(_Graph, HasEdgeFalse) {
        HF::SpatialStructures::Graph g;
        
        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();
        ASSERT_FALSE(g.HasEdge(N1, N1));
    }

	TEST(_Graph, AggregateCosts) {
		HF::SpatialStructures::Graph g;

		HF::SpatialStructures::Node N1(39, 39, 39);
		HF::SpatialStructures::Node N2(54, 54, 54);

		g.addEdge(N1, N2, 30);
		g.Compress();
		ASSERT_EQ(g.AggregateGraph(COST_AGGREGATE::SUM)[0], 30);
		ASSERT_EQ(g.AggregateGraph(COST_AGGREGATE::SUM)[1], 0);
	}

    TEST(_Graph, SizeEqualsNumberOfNodes) {
        HF::SpatialStructures::Graph g;

        HF::SpatialStructures::Node N1(1, 1, 2);
        HF::SpatialStructures::Node N2(2, 3, 4, 5);

        g.addEdge(N1, N2);
        auto nodes = g.Nodes();
        assert(g.size() == nodes.size());
    }

	TEST(_Graph, SizeEqualsNumberOfInts) {
		HF::SpatialStructures::Graph g;
		
		int N1 = 1;
		int N2 = 2;

		g.addEdge(N1, N2, 100);
		g.Compress();
		ASSERT_EQ(g.size(), 2);
	}

	TEST(_Graph, GroupInsert) {
		HF::SpatialStructures::Node N1(1, 1, 2);
		HF::SpatialStructures::Node N2(2, 3, 4, 5);
		HF::SpatialStructures::Node N3(11, 22, 140);
	
		//g.addEdge(N1, N2, 5);
		//g.addEdge(N2, N1, 6);
		//g.addEdge(N1, N3, 10);

		std::vector<std::vector<int>> edges = {
			{1, 2},
			{2},
			{1}
		};
		std::vector<std::vector<float>> costs = {
			{1.0f, 2.5f},
			{54.0f},
			{39.0f}
		};
		
		std::vector<Node> nodes = { N1,N2,N3 };
		Graph g(edges, costs, nodes);

		assert(g.HasEdge(0, 1));
		assert(g.HasEdge(0, 2));
		assert(g.HasEdge(1, 2));
		assert(!g.HasEdge(2, 0));

		assert(g.hasKey(N1));
		assert(g.hasKey(N2));
		assert(g.hasKey(N3));
	}

	bool Contains(const std::vector<int>& domain, int target) {
		for (int suspect : domain) {
			if (suspect == target)
				return true;
		}
		return false;
	}

	TEST(_Graph, GetEdges) {
		HF::SpatialStructures::Node N1(1, 1, 2);
		HF::SpatialStructures::Node N2(2, 3, 4);
		HF::SpatialStructures::Node N3(11, 22, 140);

		//g.addEdge(N1, N2, 5);
		//g.addEdge(N2, N1, 6);
		//g.addEdge(N1, N3, 10);

		std::vector<std::vector<int>> edges = {
			{1, 2},
			{2},
			{1}
		};
		std::vector<std::vector<float>> costs = {
			{1.0f, 2.5f},
			{54.0f},
			{39.0f}
		};

		std::vector<Node> nodes = { N1,N2,N3 };
		Graph g(edges, costs, nodes);

		std::vector<EdgeSet> edgesets = g.GetEdges();
		ASSERT_TRUE(edgesets.size() == edges.size());
		for (const EdgeSet edgeset : edgesets) {
			ASSERT_TRUE(edgeset.children.size() == edges[edgeset.parent].size());
			for (const IntEdge & edge : edgeset.children) {
				ASSERT_TRUE(Contains(edges[edgeset.parent], edge.child));
			}
		}
		
	}
}

namespace NodeTests {
    TEST(_Node, Distance) {
        Node N1(0, 0, 1);
        Node N2(0, 0, 0);

        ASSERT_EQ(N1.distanceTo(N2), 1);
    }
    TEST(_Node, Equality) {
        Node N1(0, 0, 1);
        Node N2(0, 0, 0);

        EXPECT_FALSE(N1 == N2);
        EXPECT_TRUE(N1 == N1);
        EXPECT_TRUE(N2 == N2);
    }

    TEST(_Node, RoundingAndEquality) {
        Node N1(0, 0, 0.0001);
        Node N2(0, 0, 0);

        ASSERT_TRUE(N1 != N2);
    }


}

///
///	The following are tests for the code samples for HF::SpatialStructures::Node
///

namespace NodeExampleTests {
	TEST(_Node, DefaultConstructor) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);

		/*
		std::cout << 12.0 << " " << node_0.x << std::endl;
		std::cout << 23.1 << " " << node_0.y << std::endl;
		std::cout << 34.2 << " " << node_0.z << std::endl;
		std::cout << 456 << " " << node_0.id << std::endl;
		*/
	}

	TEST(_Node, ParamConstructorNoID) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2);
	}

	TEST(_Node, ParamConstructorArray) {
#include <array>

		std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
		HF::SpatialStructures::Node node(pos);
	}

	TEST(_node, ParamConstructorArrayTypeID) {
#include <array>
	
		std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
		HF::SpatialStructures::Node node(pos, HF::SpatialStructures::NODE_TYPE::GRAPH, 456);
	}

	TEST(_node, DistanceTo) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
	
		float euclidean_distance = node_0.distanceTo(node_1);
	}

	TEST(_node, AngleTo) {
		/// TODO code sample: line 40 of node.cpp - 'This needs an actual angle formula'
	}

	TEST(_node, DirectionTo) {
#include <array>
		
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		std::array<float, 3> direction_vector = node_0.directionTo(node_1);
	}

	TEST(_node, GetArray) {
#include <array>

		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);
		
		// An array is created from within the getArray() member function
		std::array<float, 3> arr = node.getArray();
		
		// ref_arr and ref_node have the same value, but
		// refer to different locations in memory -- 
		// arr does not consist of the same memory locations as
		// that of the coordinate fields within node.
		float& ref_arr = arr[0];
		float& ref_node = node.x;

		/*
		std::cout << "ref_arr: " << ref_arr << "\tref_node: " << ref_node << std::endl;
		std::cout << "ref_arr: " << &ref_arr << "\tref_node: " << &ref_node << std::endl;
		*/
	}

	TEST(_node, OperatorIndexByRef) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID
		
		float& position = node[1];			// access by reference
		position = 93.5;					// node.y is now 93.5

		/*
		std::cout << "node.y = " << node.y << std::endl;
		std::cout << "position = " << position << std::endl;
		*/
	}

	TEST(_node, OperatorIndexByVal) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID
		
		float position = node[1];			// access by reference
		position = 93.5;					// node.y is still 23.1
	
		std::cout << "node.y = " << node.y << std::endl;
		std::cout << "position = " << position << std::endl;
	}

	TEST(_node, OperatorEquality) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		bool same_position = node_0 == node_1;
		
		if (same_position) {
			std::cout << "Occupies the same space" << std::endl;
		} else {
			std::cout << "Different positions" << std::endl;
		}
		
		// same_position evaluates to false
	}

	TEST(_node, OperatorAssignment) {
#include <array>
		/* Cannot test -- not defined in node.cpp
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2);
		std::cout << node.x << ", " << node.y << ", " << node.z << std::endl;
		std::array<float, 3> position = { 45.3, 56.4, 67.5 };

		node = position;

		std::cout << node.x << ", " << node.y << ", " << node.z << std::endl;
		*/
	}

	TEST(_node, OperatorNEquality) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		// Does the inverse of operator==.
		bool different_positions = node_0 != node_1;
		
		/*/
		if (different_positions) {
			std::cout << "Different positions" << std::endl;
		}
		else {
			std::cout << "Occupies the same space" << std::endl;
		}
		*/
		
		// different_positions evaluates to true
	}

	TEST(_node, OperatorMinus) {

	}

	TEST(_node, OperatorPlus) {

	}

	TEST(_node, OperatorMultiply) {

	}

	TEST(_node, OperatorLessThanConst) {

	}

	TEST(_node, OperatorLessThan) {

	}
	
	TEST(_node, OperatorGreaterThanConst) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		// operator< compares ID fields of node_0 and node_1
		bool compare = node_0 > node_1;	// evaluates to false, since 456 < 789

		/*std::cout << compare << std::endl;*/
	}
}