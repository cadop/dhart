#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>

#include "spatialstructures_C.h"

#include "cost_algorithms.h"

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
		std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
		HF::SpatialStructures::Node node(pos);
	}

	TEST(_node, ParamConstructorArrayTypeID) {
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
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		std::array<float, 3> direction_vector = node_0.directionTo(node_1);
	}

	TEST(_node, GetArray) {
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
		
		/*
		std::cout << "node.y = " << node.y << std::endl;
		std::cout << "position = " << position << std::endl;
		*/
	}

	TEST(_node, OperatorEquality) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		bool same_position = node_0 == node_1;
		
		/*
		if (same_position) {
			std::cout << "Occupies the same space" << std::endl;
		} else {
			std::cout << "Different positions" << std::endl;
		}
		*/
		
		// same_position evaluates to false
	}

	TEST(_node, OperatorAssignment) {
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
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		// removed inline keyword from line 98 in node.cpp
		HF::SpatialStructures::Node node_2 = node_1 - node_0;

		// node_2 has values (x = 33.3, y = 33.3, z = 33.3, id = -1, type = NODE_TYPE::GRAPH)
		// id and type are given default values as per Node::Node(const std::array<float, 3>& position)
	
		//std::cout << node_2.x << " " << node_2.y << " " << node_2.z << std::endl;
	}

	TEST(_node, OperatorPlus) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		HF::SpatialStructures::Node node_2 = node_1 + node_0;
		
		// node_2 has values (x = 57.3, y = 79.5, z = 101.7, id = -1, type = NODE_TYPE::GRAPH)
		// id and type are given default values as per Node::Node(const std::array<float, 3>& position)

		//std::cout << node_2.x << " " << node_2.y << " " << node_2.z << std::endl;
	}

	TEST(_node, OperatorMultiply) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		HF::SpatialStructures::Node node_2 = node_1 * node_0;
		
		// node_2 has values (x = 543.6, y = 1302.84, z = 2308.5, id = -1, type = NODE_TYPE::GRAPH)
		// id and type are given default values as per Node::Node(const std::array<float, 3>& position)

		//std::cout << node_2.x << " " << node_2.y << " " << node_2.z << std::endl;
	}

	TEST(_node, OperatorLessThanConst) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		 // operator< compares ID fields of node_0 and node_1
		bool compare = node_0 < node_1;		// evaluates to true, since 456 < 789

		/*
		auto str = compare ? "true" : "false";
		std::cout << str << std::endl;
		*/
	}

	TEST(_node, OperatorLessThan) {		
		// For this example, we are not concerned about the node coordinates.
		HF::SpatialStructures::Node node_0(0.0, 0.0, 0.0, 3);
		HF::SpatialStructures::Node node_1(0.0, 0.0, 0.0, 1);
		HF::SpatialStructures::Node node_2(0.0, 0.0, 0.0, 2);
		HF::SpatialStructures::Node node_3(0.0, 0.0, 0.0, 0);
		
		std::vector<Node> vec{ node_0, node_1, node_2, node_3 };
		
		// operator< sorts Node by ID, in non-decreasing order
		std::sort(vec.begin(), vec.end());	// uses natural ordering through operator<, non-const
		
		std::vector<Node>::iterator it = vec.begin();
		
		/*
		while (it != vec.end()) {
			std::cout << "Node ID: " << it->id << std::endl;
			++it;
		} // Node ID will print in order by ID, from smallest to largest
		*/
	}
	
	TEST(_node, OperatorGreaterThanConst) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);
		
		// operator< compares ID fields of node_0 and node_1
		bool compare = node_0 > node_1;	// evaluates to false, since 456 < 789

		/*
		auto str = compare ? "true" : "false";
		std::cout << str << std::endl;
		*/
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Edge
///

namespace EdgeExampleTests {
	TEST(_edge, EdgeConstructor) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);
		float score = 4.3f;
		
		HF::SpatialStructures::Edge edge(node, score, STEP::NOT_CONNECTED);
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Path
///

#include "path.h"
namespace PathExampleTests {
	TEST(_pathMember, OperatorEquality) {
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 2.78f, 2 };

		bool is_true = p1 == p2;
		bool is_false = p0 == p1;
		
		// cost and node fields of p1 and p2 are identical, so equiv evaluates true
		// not_equiv evaluates false, since fields of p0 and p1 do not have matching values

		/*
		auto str_equiv = equiv ? "true" : "false";
		auto str_nequiv = not_equiv ? "true" : "false";
		std::cout << str_equiv << " " << str_nequiv << std::endl;
		*/
	}

	TEST(_pathMember, OperatorNEquality) {
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 2.78f, 2 };
		
		bool is_true = p1 == p2;
		bool is_false = p0 != p1;
		
		// applies the inverse of operator== to evaluate operator!=
		// since fields of p1 and p2 have different values, is_true is in fact, true
		// is_false evaluates to true.

		/*
		auto str_equiv = is_true ? "true" : "false";
		auto str_nequiv = is_false ? "true" : "false";
		std::cout << str_equiv << " " << str_nequiv << std::endl;
		*/
	}

	TEST(_path, NoArgConstructor) {
		HF::SpatialStructures::Path path;
	}

	TEST(_path, ParamConstructor) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35, 7 };
		
		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};
		
		// Create the path, using the container of PathMembers
		HF::SpatialStructures::Path path(members);
	}

	TEST(_path, AddNode) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{ p0, p1, p2, p3 };

		// Create the path, using the container of PathMembers
		HF::SpatialStructures::Path path(members);
		
		int node_id = 278;
		float cost = 8.92f;
		
		path.AddNode(node_id, cost);	// A PathMember is constructed within AddNode from node_id and cost
										// and is then appended to the underlying members vector (via push_back)
	}

	TEST(_path, Empty) {
		HF::SpatialStructures::Path mypath;
		
		// There are no PathMembers in path's members container.
		
		// if empty() returns true, that means the underlying members vector is of size 0 (no members)
		// otherwise, empty returns false. In this case, path.empty() returns true.
		std::string result = mypath.empty() ? "is empty" : "has at least one member";
		
		//std::cout << "The Path object " << result << std::endl;
	}

	TEST(_path, Size) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };
		
		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};
		
		HF::SpatialStructures::Path path(members);			// Create the Path object, path
		path.AddNode(278, 3.14f);	// Add one more PathMember to path
		
		std::string result = path.size() >= 5 ? "at least 5 members" : "under 5 members";
		
		//std::cout << "The Path object has " << result << std::endl;
	}

	TEST(_path, Reverse) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };
		
		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};
		
		HF::SpatialStructures::Path path(members);			// Create the Path object, path
		path.AddNode(278, 3.14f);	// Append one more PathMember to path
		
		path.Reverse();				// The order of the PathMembers within members is now that of
									// p3, p2, p1, p0

		//std::cout << path << std::endl;
	}

	TEST(_path, OperatorEquality) {
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };				// Create all the PathMember objects
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		
		HF::SpatialStructures::PathMember p2 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p3 = { 2.78f, 2 };
		
		HF::SpatialStructures::PathMember p4 = { 1.1f, 9 };
		HF::SpatialStructures::PathMember p5 = { 123.0f, 10 };
		
		std::vector<HF::SpatialStructures::PathMember> members_0{p0, p1};	// Create the HF::SpatialStructures::PathMember vectors
		std::vector<HF::SpatialStructures::PathMember> members_1{p2, p3};
		std::vector<HF::SpatialStructures::PathMember> members_2{p4, p5};
		std::vector<HF::SpatialStructures::PathMember> members_3{p1, p0};

		HF::SpatialStructures::Path path_0(members_0);						// Create the Path objects
		HF::SpatialStructures::Path path_1(members_1);
		HF::SpatialStructures::Path path_2(members_2);
		HF::SpatialStructures::Path path_3(members_3);
		
		bool same_values_same_order = path_0 == path_1;
		bool totally_different = path_0 == path_2;
		bool same_values_different_order = path_0 == path_3;
		
		// path_0 and path_1 share the same PathMember values, with an identical permutation,
		// so they are equivalent.

		//ASSERT_TRUE(same_values_different_order);
		
		// path_0 and path_2 are not equivalent, because they have completely different PathMember
		// values.

		//ASSERT_TRUE(!totally_different);
		
		// path_0 and path_3 are not equivalent, because although they have PathMember objects of
		// the same values, the order in which path_0 and path_3 have their member vectors arranged
		// are different.

		//ASSERT_TRUE(!same_values_different_order);
	}

	TEST(_path, OperatorIndex) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };
		
		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};
		
		HF::SpatialStructures::Path path(members);							// Create the Path object, path
		
		const int desired_index = 2;
		HF::SpatialStructures::PathMember result = path[desired_index];	// a copy of the element at desired_index
														// within the internal members vector
														// is assigned to result

		//ASSERT_TRUE(&result != &path[desired_index]);
	}

	TEST(_path, GetPMPointer) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };
		
		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};
		
		HF::SpatialStructures::Path path(members);							// Create the Path object, path
		
		HF::SpatialStructures::PathMember *ptr = path.GetPMPointer();
		
		// You now have a pointer to the underlying buffer of the members vector with in a PathMember.
		HF::SpatialStructures::PathMember *curr = ptr;
		HF::SpatialStructures::PathMember *finish = ptr + path.size();
		
		while (curr != finish) {
			std::cout << "Cost: " << curr->cost << " " 
					  << "Node: " << curr->node << std::endl;
			++curr;
		}
	}

	TEST(_path, OperatorLessThanLessThan) {
		 // Create the PathMembers
		 HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		 HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		 HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		 HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };
		
			// Create the container of PathMembers
		 std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};
		
		 HF::SpatialStructures::Path path(members);							// Create the Path object, path
		
		 std::cout << path << std::endl;				// Output path to an ostream, like cout
		 // Output is:
			// (3) -3.14-> (2) -2.78-> (1) -1.64-> (7) -9.35->
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Graph
///

namespace GraphExampleTests {
	TEST(_csrptrs, AreValid) {
		 std::unique_ptr<float[]> data(new float[16]);
		 std::unique_ptr<int[]> outer_indices(new int[16]);
		 std::unique_ptr<int[]> inner_indices(new int[16]);
		
		 float* p_data = data.get();
		 int* p_outer_indices = outer_indices.get();
		 int* p_inner_indices = inner_indices.get();

		 HF::SpatialStructures::CSRPtrs csr = { 16, 16, 16, p_data, p_outer_indices, p_inner_indices };
	
		 bool validity = csr.AreValid();	// validity == true, since all pointer fields are non-null

		 ASSERT_TRUE(validity);
	}

	TEST(_graph, ParamConstructor) {		
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
	}

	TEST(_graph, NoArgConstructor) {
		HF::SpatialStructures::Graph graph;		// This represents an order-zero graph (null graph)
												// It lacks vertices and edges.
	}

	TEST(_graph, HasEdge_Array) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		// Prepare {x, y, z} coordinates (positions)
		auto parent_pos = node_1.getArray();		// (2.0, 3.0, 4.0)
		auto child_pos = node_2.getArray();			// (11.0, 22.0, 140.0)

		// last argument can be true/false for undirected/directed graph respectively
		bool has_edge = graph.HasEdge(parent_pos, child_pos, true);

		ASSERT_TRUE(has_edge);
	}

	TEST(_graph, HasEdge_Node) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);										  
		
		// last argument can be true/false for undirected/directed graph respectively
		bool has_edge = graph.HasEdge(node_1, node_2, true);

		ASSERT_TRUE(has_edge);
	}

	TEST(_graph, HasEdge_Int) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		// last argument can be true/false for undirected/directed graph respectively
		bool has_edge = graph.HasEdge(0, 1, true);

		ASSERT_TRUE(has_edge);
	}

	TEST(_graph, Nodes) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);	
		
		// Nodes() returns a copy of the ordered_nodes field
		std::vector<HF::SpatialStructures::Node> nodes_from_graph = graph.Nodes();

		/*
		for (auto n : nodes_from_graph) {
			std::cout << "(" << n.x << ", " << n.y << ", " << n.z << ") : " << n.id << std::endl;
		}
		*/
	}

	TEST(_graph, GetUndirectedEdges) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);	
		
		// Retrieve the nodes from the graph, or use the original instance of 
		// std::vector<HF::SpatialStructures::Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

		/*
		for (auto n : get_nodes) {
			std::cout << n.getArray() << std::endl;
		}
		*/
		
		std::cout << "\n";

		// nodes[index] yields an instance of Node that we can pass to GetUndirectedEdges.
		// Any node that exists with graph can be passed to this member function
		// to retrieve a vector of undirected edges.
		int index = 2;
		std::vector<HF::SpatialStructures::Edge> undirected_edges = graph.GetUndirectedEdges(get_nodes[index]);

		/*
		for (auto e : undirected_edges) {
			std::cout << e.child.getArray() << std::endl;
		}
		*/
	}

	TEST(_graph, GetEdges) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);	
		
		// graph must be compressed, or a exception will be thrown
		
		// To brief, an EdgeSet has the following layout:
			//		struct EdgeSet {
			//			int parent;
			//			std::vector<IntEdge> children;
			//		};
			//
			//	An IntEdge has the following layout:
			//		struct IntEdge {
			//			int child;
			//			float weight;
			//		};
		
		// A std::vector<EdgeSet> is a Graph, in the form of IDs.
		std::vector<HF::SpatialStructures::EdgeSet> edge_set = graph.GetEdges();

		/*
		for (auto e : edge_set) {
			for (auto f : e.children) {
				std::cout << e.parent << "->" << "(" << f.child << ")" << std::endl;
			}
		}
		*/
	}

	TEST(_graph, AggregateGraph) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		// graph must be compressed, or a exception will be thrown
		HF::SpatialStructures::COST_AGGREGATE aggregate = HF::SpatialStructures::COST_AGGREGATE::AVERAGE;		// aggregate == 1 in this case
		 
		// directed parameter may be true or false
		std::vector<float> aggregate_graph = graph.AggregateGraph(aggregate, true);	
		
		/*
		for (auto a : aggregate_graph) {
			std::cout << a << std::endl;
		}
		*/
	}

	TEST(_graph, OperatorIndex) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		// Retrieve the nodes from the graph, or use the original instance of 
		// std::vector<Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
		
		// nodes[index] yields an instance of Node that we can pass to GetUndirectedEdges.
		// Any node that exists with graph can be passed to this member function
		// to retrieve a vector of edges.
		int index = 2;
		HF::SpatialStructures::Node node = get_nodes[index];
		
		// Note that if node does not exist within graph, that an exception will be thrown.
		std::vector<HF::SpatialStructures::Edge> undirected_edges = graph[node];

		std::cout << node.getArray() << "->";
		for (auto e : undirected_edges) {
			std::cout << e.child.getArray() << std::endl;
		}
	}

	TEST(_graph, AddEdge_Node) {
		// be sure to #include "graph.h"
		
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		// Create a pair of nodes
		HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
		HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);
		
		graph.addEdge(n_parent, n_child);	// default score is 1.0f
	}

	TEST(_graph, AddEdge_Int) {
		// be sure to #include "graph.h"
		
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		int parent = 1;
		int child = 2;
		
		graph.addEdge(parent, child, 1.0f);
		graph.Compress();
	}

	TEST(_graph, HasKey) {
		// be sure to #include "graph.h"
		
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
		bool has_key = graph.hasKey(other_node);	// other_node does not exist in graph, so has_key == false;
		
		// Likewise, if we pass a Node instance that indeed exists...
		
		// Retrieve the nodes from the graph, or use the original instance of 
		// std::vector<Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
		
		// nodes[index] yields an instance of Node that we can pass to hasKey.
		// Any node that exists with graph can be passed to this member function
		// to determine if the graph has the node's key, or not.
		int index = 2;
		HF::SpatialStructures::Node good_node = get_nodes[index];
		
		has_key = graph.hasKey(good_node);		// now has_key is true

		ASSERT_TRUE(has_key);
	}

	TEST(_graph, NodesAsFloat3) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		// A container of std::array<float, 3> is constructed and populated within
		// NodesAsFloat3, and returned. Each array of 3 floats represents a Node's position
		// within the Cartesian coordinate system. { x, y, z }
		std::vector<std::array<float, 3>> nodes_as_floats = graph.NodesAsFloat3();
			
		// The two loops below will yield the same output
		for (auto n : graph.Nodes()) {
			std::cout << "(" << n.x << "," << n.y << "," << n.z << ")" << std::endl;
		}

		for (auto a : nodes_as_floats) {
			std::cout << a << std::endl;
		}
	}

	TEST(_graph, Size) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		int id_count = graph.size();		  // We retrieve the size of the node id count within graph
		
		ASSERT_TRUE(id_count == 3);
	}

	TEST(_graph, GetID) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);
		
		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };
		
		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };
		
		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
		
		HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
		bool has_key = graph.hasKey(other_node);	// other_node does not exist in graph, so has_key == false;
		
		int ID = graph.getID(other_node);			// ID will assigned -1, because other_node is not a part of graph.
		
		// Likewise, if we pass a Node instance that indeed exists...
		
		// Retrieve the nodes from the graph, or use the original instance of 
		// std::vector<Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();
		
		// nodes[index] yields an instance of Node that we can pass to hasKey.
		// Any node that exists with graph can be passed to this member function
		// to determine if the graph has the node's key, or not.
		int index = 2;					// we assume for this example that index 2 is valid.
		HF::SpatialStructures::Node good_node = get_nodes[index];
		
		ID = graph.getID(good_node);	// ID > -1, i.e. it is a Node instance that exists within this Graph.

		std::cout << ID << std::endl;
	}

	TEST(_graph, Compress) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Create a pair of nodes
		HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
		HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

		graph.addEdge(n_parent, n_child);	// default score is 1.0f

		// In order to use GetEdges, or AggregateGraph, we must compress our graph instance
		graph.Compress();						// GetEdges and AggregateGraph are now usable
	}

	TEST(_graph, GetCSRPointers) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Create a pair of nodes
		HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
		HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

		graph.addEdge(n_parent, n_child);	// default score is 1.0f

		// Graph will be compressed automatically be GetCSRPointers
		CSRPtrs returned_csr = graph.GetCSRPointers();
	}

	TEST(_graph, NodeFromID) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Let's retrieve node_1.
		int desired_node_id = 2;
		HF::SpatialStructures::Node node_from_id = graph.NodeFromID(desired_node_id);

		// Note that NodeFromID ceases to work if the id argument provided does not exist as an ID among
		// the nodes within graph
		ASSERT_TRUE(node_from_id.id == desired_node_id);
	}

	TEST(_graph, Clear) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// If we want to remove all nodes and edges from graph, we may do so with Clear:
		graph.Clear();						// edge_matrix is zeroed out, buffer is squeezed,
											// triplets are also cleared, and needs_compression == true
	
		auto v = graph.NodesAsFloat3();
		for (auto n : v) {
			std::cout << n << std::endl;
		}
	}

	TEST(_graph, GenerateCrossSlope) {
		// TODO example - code commented out in graph.cpp
	}

	TEST(_graph, GenerateEnergy) {
		// TODO example - code commented out in graph.cpp
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Constants
///

namespace ConstantsExampleTests {
	TEST(_functions, RoundHF) {
		// std::round from the cmath library is used in roundhf.
		// Precision is to the nearest ten-thousandth
		const float my_pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
		float rounded = HF::SpatialStructures::roundhf(my_pi);	// 	rounded == 3.1416
		//std::cout << rounded << std::endl;
	}
}

namespace CInterfaceTests {
	TEST(_NodeCInterface, GetAllNodesFromGraph) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		auto out_vec = new std::vector<HF::SpatialStructures::Node>;
		HF::SpatialStructures::Node* out_data = nullptr;

		GetAllNodesFromGraph(g, &out_vec, &out_data);

		DestroyGraph(g);
	}

	TEST(_NodeCInterface, GetSizeOfNodeVector) {
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		int node_vec_size = -1;
		GetSizeOfNodeVector(node_vec, &node_vec_size);

		DestroyNodes(node_vec);
	}

	TEST(_NodeCInterface, AggregateCosts) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		std::vector<float>* out_vector = nullptr;
		float* out_data = nullptr;

		int aggregation_type = 0;
		AggregateCosts(g, aggregation_type, false, &out_vector, &out_data);

		DestroyGraph(g);
	}

	TEST(_NodeCInterface, CreateGraph) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		// use Graph

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, AddEdgeFromNodes) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		AddEdgeFromNodes(g, n0, n1, distance);

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, AddEdgeFromNodeIDs) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		const int id0 = 0;
		const int id1 = 1;
		const float distance = 3;

		AddEdgeFromNodeIDs(g, id0, id1, distance);

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, GetCSRPointers) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		// Retrieve the CSR from the graph
		CSRPtrs csr;
		GetCSRPointers(g, &csr.nnz, &csr.rows, &csr.cols, &csr.data, &csr.inner_indices, &csr.outer_indices);

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, GetNodeID) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		AddEdgeFromNodes(g, n0, n1, distance);

		float point[] = { 0, 1, 2 };
		int result_id = -1;

		GetNodeID(g, point, &result_id);

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, Compress) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		AddEdgeFromNodes(g, n0, n1, 1);
		AddEdgeFromNodes(g, n0, n2, 2);
		AddEdgeFromNodes(g, n1, n0, 3);
		AddEdgeFromNodes(g, n1, n2, 4);
		AddEdgeFromNodes(g, n2, n0, 5);
		AddEdgeFromNodes(g, n2, n1, 6);

		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, ClearGraph) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		AddEdgeFromNodes(g, n0, n1, distance);

		ClearGraph(g);

		// Release memory for g after use
		DestroyGraph(g);
	}

	TEST(_NodeCInterface, DestroyNodes) {
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		// Use node_vec

		DestroyNodes(node_vec);
	}

	TEST(_NodeCInterface, DestroyEdges) {
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		HF::SpatialStructures::Edge e0(n1); // parent is n0
		HF::SpatialStructures::Edge e1(n3); // parent is n2

		auto edge_vec = new std::vector<HF::SpatialStructures::Edge>{ e0, e1 };

		// Use edge_vec

		DestroyEdges(edge_vec);
	}

	TEST(_NodeCInterface, DestroyGraph) {
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		// use Graph

		// Release memory for g after use
		DestroyGraph(g);
	}
}

namespace CostAlgorithmsTests {
	TEST(_CostAlgorithms, CalculateCrossSlope) {
		Node n0(1, 1, 2);
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

		if (csr.AreValid()) {
			for (int i = 0; i < csr.nnz; i++) {
				std::cout << "child id: " << csr.inner_indices[i] << std::endl;
			}

			std::vector<IntEdge> edge_result = CostAlgorithms::CalculateCrossSlope(g);

			for (IntEdge ie : edge_result) {
				std::cout << ie.child << " has weight " << ie.weight << std::endl;
			}
		}
	}
}

namespace CInterfaceTests {
	TEST(_CostAlgorithmsCInterface, CalculateAndStoreCrossSlope) {

	}
}