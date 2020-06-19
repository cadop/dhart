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
		
		/// // operator< compares ID fields of node_0 and node_1
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
		HF::SpatialStructures::Path path();
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

}

///
///	The following are tests for the code samples for HF::SpatialStructures::Constants
///

namespace ConstantsExampleTests {

}

