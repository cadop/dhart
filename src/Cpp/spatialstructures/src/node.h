#pragma once

#ifndef HF_NODE
#define HF_NODE

#include <array>
#include <ostream>

namespace HF
{
	namespace SpatialStructures {
		enum NODE_TYPE {
			GRAPH = 0,
			POI = 1,		///< POI is 'point of interest' ///<
			OTHER = 2
		};

		/// <summary>
		/// A point in space with an ID
		/// </summary>
		struct Node {
		public:
			float x, y, z;		///< Cartesian coordinates x, y, z ///<
			short type = GRAPH; ///< Default this field to GRAPH ///<
			int id;				///< Node identifier ///<

			/// <summary>
			/// Default constructor. Constructs everything with NAN
			/// </summary>
			
			/// \code{.cpp}
			/// Node node();	// all fields initialized to NAN
			/// \endcode
			Node();

			// Constructors
			/// <summary>
			/// Create a node without an ID
			/// </summary>
			/// <param name="x"> X coordinate</param>
			/// <param name="y"> Y coordinate</param>
			/// <param name="z"> Z coordinate</param>
			/// <param name="id"> ID of the node </param>
			
			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456)
			/// \endcode
			Node(float x, float y, float z, int ID = -1);

			/// <summary>
			/// Create a node without an ID
			/// </summary>
			/// <param name="position">An array of 3 floats for x,y,z</param>
			
			/// \code{.cpp}
			/// std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
			/// Node node(pos);		// id == -1, type == NODE_TYPE::GRAPH
			/// \endcode
			Node(const std::array<float, 3>& position);

			// Create a point of interest
			
			/// \code{.cpp}
			/// std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
			/// Node node(pos, NODE_TYPE::GRAPH, 456);
			/// \endcode
			Node(const std::array<float, 3>& position, NODE_TYPE t, int id);

			/// <summary>
			/// Calculate the distance between this node and the given node
			/// </summary>
			/// <param name="n2">Note to get distance to </param>
			/// <returns>Distance between this node and n2</returns>
			
			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// float euclidean_distance = node_0.distanceTo(node_1);
			/// \endcode
			float distanceTo(const Node& n2) const;

			/// <summary>
			/// Calculate the angle between node 1 and n2
			/// </summary>
			/// <param name="n2">Node to calculate direction to </param>
			/// <returns>Angle between this node and n2</returns>

			/// \code{.cpp}
			/// TODO code sample: line 40 of node.cpp - 'This needs an actual angle formula'
			/// \endcode
			float angleTo(const Node& n2) const;

			/// <summary>
			/// Get the direction between this node and another node
			/// </summary>
			/// <param name="n2">The node to get the direction to </param>
			/// <returns> an array of 3 floats indicating a direction</returns>

			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// std::array<float, 3> direction_vector = node_0.directionTo(node_1);
			/// \endcode
			std::array<float, 3> directionTo(const Node& n2) const;

			/// <summary>
			/// Returns the x,y,z of this node as an array of 3 floats
			/// </summary>
			/// <param name="n2"></param>
			/// <returns></returns>

			/// \code{.cpp}
			/// Node node(12.0, 23.1, 34.2, 456);
			///
			/// // An array is created from within the getArray() member function
			/// std::array<float, 3> arr = node.getArray();
			///
			/// // ref_arr and ref_node have the same value, but
			/// // refer to different locations in memory -- 
			/// // arr does not consist of the same memory locations as
			/// // that of the coordinate fields within node.
			/// float& ref_arr = arr[0];
			/// float& ref_node = node.x;
			/// \endcode
			std::array<float, 3> getArray() const;

			// Operators

			/// <summary>
			/// Directly access a nodes's position as if it were an array of 3 floats
			/// </summary>
			/// <param name="i"> Index. 0 = x, 1 = y, 2 = z</param>
			/// <returns>a reference to the member float for the requested coordinate</returns>

			/// \code{.cpp}
			/// Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID
			///
			/// float& position = node[1];			// access by reference
			/// position = 93.5						// node.y is now 93.5
			/// \endcode
			float& operator[](int i);

			/// <summary>
			/// Access a nodes's position, by value
			/// </summary>
			/// <param name="i"> Index. 0 = x, 1 = y, 2 = z</param>
			/// <returns>the value (copy) of the member float for the requested coordinate</returns>

			/// \code{.cpp}
			/// Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID
			///
			/// float position = node[1];			// access by reference
			/// position = 93.5						// node.y is still 23.1
			/// \endcode
			float operator[](int i) const;

			/// <summary>
			/// Check if n1 occupies the same space as n2
			/// </summary>
			/// <param name="n2">Node to compare with n1</param>
			/// <returns>True if the distance between n1 and n2 is less than Rounding Precision, false otherwise</returns>

			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// bool same_position = node_0 == node_1;
			///
			/// if (same_position) {
			/// 	std::cout << "Occupies the same space" << std::endl;
			/// } else {
			/// 	std::cout << "Different positions" << std::endl;
			/// }
			///
			/// // same_position evaluates to true
			/// \endcode
			bool operator==(const Node& n2) const;


			/// <summary>
			/// Assigns the values of array n2 to n1
			/// </summary>
			/// <param name="n2">Array whose values will be assigned to n1</param>

			/// \code{.cpp}
			/// Node node(12.0, 23.1, 34.2);
			/// std::array<float, 3> position = { 45.3, 56.4, 67.5 };
			///
			/// node = position;			// assigns node's x, y, z fields to that of position's values
			/// \endcode
			void operator=(const std::array<float, 3>& n2);

			/// <summary>
			/// See operator==, checks if n1 does NOT occupy the same space as n2
			/// <param name="n2">Node to compare with N1</param>
			/// </summary>
			/// <returns>True if the distance between n1 and n2 is greater than or equal to Rounding Precision, false otherwise</returns>
			
			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// // Does the inverse of operator==.
			/// bool different_positions = node_0 != node_1;
			///
			/// if (different_positions) {
			///		std::cout << "Different positions" << std::endl;
			/// } else {
			///		std::cout << "Occupies the same space" << std::endl;
			/// }
			///
			/// // different_positions evaluates to true
			/// \endcode
			bool operator!=(const Node& n2) const;

			/// <summary>
			/// Creates a node from the vector subtraction of n1, n2
			/// </summary>
			/// <param name="n2">Node to subtract from N1</param>
			/// <returns>A node with the values obtained from n1 - n2</returns>

			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// Node node_2 = node_1 - node_0;
			///
			/// // node_2 has values (x = 33.3, y = 33.3, z = 33.3, id = -1, type = NODE_TYPE::GRAPH)
			/// // id and type are given default values as per Node::Node(const std::array<float, 3>& position)
			///
			/// \endcode
			Node operator-(const Node& n2) const;

			/// <summary>
			/// Creates a new node from the vector addition of n1, n2
			/// </summary>
			/// <param name="n2">Node to add to n1</param>
			/// <returns>A node with the values obtained from n1 + n2</returns>

			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// Node node_2 = node_1 + node_0;
			///
			/// // node_2 has values (x = 57.3, y = 79.5, z = 101.7, id = -1, type = NODE_TYPE::GRAPH)
			/// // id and type are given default values as per Node::Node(const std::array<float, 3>& position)
			/// \endcode
			Node operator+(const Node& n2) const;

			/// <summary>
			/// Creates a new node from the dot product of n1, n2
			/// </summary>
			/// <param name="n2">Second factor of dot product, N1 (dot) N2</param>
			/// <returns>A node with the values obtained from the dot product of N1 and N2</returns>

			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// Node node_2 = node_1 * node_0;
			///
			/// // node_2 has values (x = 543.6, y = 1302.84, z = 2308.5, id = -1, type = NODE_TYPE::GRAPH)
			/// // id and type are given default values as per Node::Node(const std::array<float, 3>& position)
			/// \endcode
			Node operator*(const Node& n2) const;

			/// <summary>
			/// Determines if n1's id (an integer) is less than n2's id
			/// </summary>
			/// <param name="n2">Node whose id will be compared with n1</param>
			/// <returns>True if n1's id is less than n2's id, false otherwise</returns>

			/// \code{.cpp}
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// // operator< compares ID fields of node_0 and node_1
			/// bool compare = node_0 < node_1;		// evaluates to true, since 456 < 789
			/// \endcode
			bool operator<(const Node& n2) const;

			/// <summary>
			/// Determines if n1's id (an integer) is less than n2's id - const qualification omitted for std::sort
			/// </summary>
			/// <param name="n2">Node whose id will be compared with n1</param>
			/// <returns>True if n1's id is less than n2's id, false otherwise</returns>

			/// \code{.cpp}
			/// // For this example, we are not concerned about the node coordinates.
			/// Node node_0(0.0, 0.0, 0.0, 3);
			/// Node node_1(0.0, 0.0, 0.0, 1);
			/// Node node_2(0.0, 0.0, 0.0, 2);
			/// Node node_3(0.0, 0.0, 0.0, 0);
			///
			/// std::vector<Node> vec{ node_0, node_1, node_2, node_3 };
			///
			/// // operator< sorts Node by ID, in non-decreasing order
			/// std::sort(vec.begin(), vec.end());	// uses natural ordering through operator<, non-const
			///
			/// std::vector<Node>::iterator it = vec.begin();
			///
			/// while (it != vec.end()) {
			///		std::cout << "Node ID: " << it->id << std::endl;
			///		++it;
			/// } // Node ID will print in order by ID, from smallest to largest
			/// \endcode
			bool operator<(const Node& n2);

			/// <summary>
			/// Determines if n1's id (an integer) is greater than n2's id
			/// </summary>
			/// <param name="n2">Node whose id will be compared with n1</param>
			/// <returns>True if n1's id is greater than n2's id, false otherwise</returns>

			/// \code{.cpp}
			///
			/// Node node_0(12.0, 23.1, 34.2, 456);
			/// Node node_1(45.3, 56.4, 67.5, 789);
			///
			/// // operator< compares ID fields of node_0 and node_1
			/// bool compare = node_0 > node_1;	// evaluates to false, since 456 < 789
			/// \endcode
			bool operator>(const Node& n2) const;
		};
	};
}

/// Hashing code for nodes
namespace std {
	template <typename SizeT>
	inline void hash_combine_impl(SizeT& seed, SizeT value) noexcept
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <>
	struct hash<HF::SpatialStructures::Node>
	{
		inline std::size_t operator()(const HF::SpatialStructures::Node& k) const noexcept
		{
			size_t seed = std::hash<float>()(k.x);
			hash_combine_impl(seed, std::hash<float>()(k.y));
			hash_combine_impl(seed, std::hash<float>()(k.z));
			return seed;
		}
	};

	inline ostream& operator<<(ostream& os, const HF::SpatialStructures::Node n) {
		os << "(" << n.x << ", " << n.y << ", " << n.z << ")";
		return os;
	}

	inline ostream& operator<<(ostream& os, const std::array<float, 3> n) {
		os << "(" << n[0] << "," << n[1] << "," << n[2] << ")";
		return os;
	}
}

#endif
