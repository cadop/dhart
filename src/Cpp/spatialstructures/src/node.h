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
			Node();
			// Constructors
			/// <summary>
			/// Create a node without an ID
			/// </summary>
			/// <param name="x"> X coordinate</param>
			/// <param name="y"> Y coordinate</param>
			/// <param name="z"> Z coordinate</param>
			/// <param name="id"> ID of the node </param>
			Node(float x, float y, float z, int ID = -1);

			/// <summary>
			/// Create a node without an ID
			/// </summary>
			/// <param name="position">An array of 3 floats for x,y,z</param>
			Node(const std::array<float, 3>& position);

			// Create a point of interest
			Node(const std::array<float, 3>& position, NODE_TYPE t, int id);

			/// <summary>
			/// Calculate the distance between this node and the given node
			/// </summary>
			/// <param name="n2">Note to get distance to </param>
			/// <returns>Distance between this node and n2</returns>
			float distanceTo(const Node& n2) const;

			/// <summary>
			/// Calculate the angle between node 1 and n2
			/// </summary>
			/// <param name="n2">Node to calculate direction to </param>
			/// <returns>Angle between this node and n2</returns>
			float angleTo(const Node& n2) const;

			/// <summary>
			/// Get the direction between this node and another node
			/// </summary>
			/// <param name="n2">The node to get the direction to </param>
			/// <returns> an array of 3 floats indicating a direction</returns>
			std::array<float, 3> directionTo(const Node& n2) const;

			/// <summary>
			/// Returns the x,y,z of this node as an array of 3 floats
			/// </summary>
			/// <param name="n2"></param>
			/// <returns></returns>
			std::array<float, 3> getArray() const;

			/// <summary>
			/// Directly access a nodes's position as if it were an array of 3 floats
			/// </summary>
			/// <param name="i"> Index. 0 = x, 1 = y, 2 = z</param>
			/// <returns>a reference to the member float for the requested coordinate</returns>
			float& operator[](int i);

			/// <summary>
			/// Access a nodes's position, by value
			/// </summary>
			/// <param name="i"> Index. 0 = x, 1 = y, 2 = z</param>
			/// <returns>the value (copy) of the member float for the requested coordinate</returns>
			float operator[](int i) const;

			// Operators

			/// <summary>
			/// Check if n1 occupies the same space as n2
			/// </summary>
			/// <param name="n2">Node to compare with n1</param>
			/// <returns>True if the distance between n1 and n2 is less than Rounding Precision, false otherwise</returns>
			bool operator==(const Node& n2) const;

			/// <summary>
			/// Assigns the values of array n2 to n1
			/// </summary>
			/// <param name="n2">Array whose values will be assigned to n1</param>
			void operator=(const std::array<float, 3>& n2);

			/// <summary>
			/// See operator==, checks if n1 does NOT occupy the same space as n2
			/// <param name="n2">Node to compare with N1</param>
			/// </summary>
			/// <returns>True if the distance between n1 and n2 is greater than or equal to Rounding Precision, false otherwise</returns>
			bool operator!=(const Node& n2) const;

			/// <summary>
			/// Creates a node from the vector subtraction of n1, n2
			/// </summary>
			/// <param name="n2">Node to subtract from N1</param>
			/// <returns>A node with the values obtained from n1 - n2</returns>
			Node operator-(const Node& n2) const;

			/// <summary>
			/// Creates a new node from the vector addition of n1, n2
			/// </summary>
			/// <param name="n2">Node to add to n1</param>
			/// <returns>A node with the values obtained from n1 + n2</returns>
			Node operator+(const Node& n2) const;

			/// <summary>
			/// Creates a new node from the dot product of n1, n2
			/// </summary>
			/// <param name="n2">Second factor of dot product, N1 (dot) N2</param>
			/// <returns>A node with the values obtained from the dot product of N1 and N2
			Node operator*(const Node& n2) const;

			/// <summary>
			/// Determines if n1's id (an integer) is less than n2's id
			/// </summary>
			/// <param name="n2">Node whose id will be compared with n1</param>
			/// <returns>True if n1's id is less than n2's id, false otherwise
			bool operator<(const Node& n2) const;

			/// <summary>
			/// Determines if n1's id (an integer) is less than n2's id - const qualification omitted for std::sort
			/// </summary>
			/// <param name="n2">Node whose id will be compared with n1</param>
			/// <returns>True if n1's id is less than n2's id, false otherwise
			bool operator<(const Node& n2);

			/// <summary>
			/// Determines if n1's id (an integer) is greater than n2's id
			/// </summary>
			/// <param name="n2">Node whose id will be compared with n1</param>
			/// <returns>True if n1's id is greater than n2's id, false otherwise
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
