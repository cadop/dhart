///
/// \file		path.h
///	\brief		Contains definitions for the <see cref="HF::SpatialStructures::Path">Path</cref> structure
///
/// \author		TBA
/// \date		06 Jun 2020
///

#pragma once

#include <vector>
#include <iostream>

#include "Constants.h"

namespace HF::SpatialStructures
{
	/*! \brief The ID of a node, and the cost cost to the node after it.*/
	struct PathMember {
		float cost;		///< Cost of traversing to the next path member in the path.
		int node;		///< ID of the node this represents in the graph.

		/// <summary> Compare the id and cost of this node with p2. </summary>
		/// <param name="p2"> The PathMember to compare with this node. </param>
		/// <returns> True if p1 and p2 have equivalent cost and node values, false otherwise. </returns>
		/*!
			\code{.cpp}
				// be sure to #include "path.h"

				HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
				HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
				HF::SpatialStructures::PathMember p2 = { 2.78f, 2 };

				bool is_true = p1 == p2;
				bool is_false = p0 == p1;

				// cost and node fields of p1 and p2 are identical, so is_true evaluates true
				// is_false evaluates false, since fields of p0 and p1 do not have matching values
			\endcode
		*/
		inline bool operator==(const PathMember& p2) const {
			// unsafe to test floating point values for equality using == return(cost == p2.cost &&
			// node == p2.node);

			// Take absolute value of cost - p2.cost, compare to ROUNDING_PRECISION If true, cost
			// and p2.cost are approximately equal
			bool approx_equal = std::abs(cost - p2.cost) < HF::SpatialStructures::ROUNDING_PRECISION;
			return approx_equal && node == p2.node;
		}

		/// <summary> Determines if this pathmember amd p2 do NOT have equal cost/node values. </summary>
		/// <param name="p2"> The PathMember to compare with this pathmember. </param>
		/// <returns> True if p1 and p2 do NOT have equivalent cost/node values, false otherwise. </returns>

		/*!
			\code{.cpp}
				// be sure to #include "path.h"

				HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
				HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
				HF::SpatialStructures::PathMember p2 = { 2.78f, 2 };

				bool is_true = p1 != p2;
				bool is_false = p0 != p1;

				// applies the inverse of operator== to evaluate operator!= since fields of p1 and
				// p2 have different values, is_true is in fact, true is_false evaluates to true.
			\endcode
		*/
		inline bool operator!=(const PathMember& p2) const {
			return (!((*this) == p2));
		}
	};

	/// <summary> A collection of nodes that form a path. </summary>
	struct Path {
	public:
		std::vector<PathMember> members; ///< Ordered array of PathMembers that comprise the path

		/// <summary> Construct an empty path.</summary>
		/*!
			\code{.cpp}
				// be sure to #include "path.h"

				HF::SpatialStructures::Path path;
			\endcode
		*/
		Path::Path() {};

		/// <summary> Construct a path from an ordered list of PathMembers. </summary>
		/*!
			\code{.cpp}
				// be sure to #include "path.h"

				// Create the PathMembers
				HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
				HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
				HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
				HF::SpatialStructures::PathMember p3 = { 9.35, 7 };

				// Create the container of PathMembers
				std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

				// Create the path, using the container of PathMembers
				HF::SpatialStructures::Path path(members);
			\endcode
		*/
		Path::Path(const std::vector<PathMember> pm);

		/// \brief Add a new node to the path.
		/// \details Constructs a PathMember and appends it to the underlying members vector.
		/// <param name="node"> The identifier for the PathMember </param>
		/// <param name="cost"> The cost (weight) for the PathMember </param>

			/*!
				\code{.cpp}
					// be sure to #include "path.h"

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
													// and is then appended to the underlying
													// members vector (via push_back)
				\endcode
		*/
		void AddNode(int node, float cost);

		/// \brief Determine if this path has any nodes in it.
		/// \details Checks if the size of members is zero.
		/// <returns> True if the underyling members vector is empty, false otherwise </returns>

		/*!
			\code{.cpp}
				// be sure to #include "path.h"

				HF::SpatialStructures::Path path;

				// There are no PathMembers in path's members container.

				// if empty() returns true, that means the underlying members vector is of size 0
				// (no members) otherwise, empty returns false. In this case, path.empty() returns true.
				std::string result = path.empty() ? "is empty" : "has at least one member";

				std::cout << "The Path object " << result << std::endl;
			\endcode
		*/
		bool empty() const;

		/// <summary> Determine how many nodes are in this path. </summary>
		/// <returns> The length of this path. </returns>

		/*!
			\code{.cpp}
				// be sure to #include "path.h"

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

				std::cout << "The Path object has " << result << std::endl;
			\endcode
		*/
		int size() const;

		/// \brief Reverse the direction of this path.
		/// \details Invokes std::reverse to reverse the contents of the underlying members vector using iterators
		/*!
			\code{.cpp}
				// be sure to #include "path.h"

				// Create the PathMembers
				HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
				HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
				HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
				HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

				// Create the container of PathMembers
				std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

				HF::SpatialStructures::Path path(members);			// Create the Path object, path
				path.AddNode(278, 3.14f);							// Append one more PathMember to path

				path.Reverse();				// The order of the PathMembers within members is now that of
											// p3, p2, p1, p0
			\endcode
		*/
		void Reverse();

		/// <summary> Determine if this path is identical to p2. </summary>
		/// <param name="p2"> The Path to compare against this path </param>
		/// <returns>
		/// True, if all PathMember elements in p2 are equal to all PathMember elements in this
		/// path, false otherwise
		/// </returns>
		/*!
			\see PathMember.operator==()

			\code{.cpp}
			// be sure to #include "path.h"

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

			// path_0 and path_1 share the same PathMember values, with an identical permutation, so
			// they are equivalent.

			// path_0 and path_2 are not equivalent, because they have completely different
			// PathMember values.

			// path_0 and path_3 are not equivalent, because although they have PathMember objects
			// of the same values, the order in which path_0 and path_3 have their member vectors
			// arranged are different.
			\endcode
		*/
		bool operator==(const Path& p2) const;

		/// <summary> Retrieve the node and cost in this path at index i. </summary>
		/// <param name="i"> The index for the desired PathMember element within members. </param>
		/// <returns> A PathMember within the members vector, at index i, by value. </returns>
		/*!
			\exception std::out_of_range i extended past the number of nodes in the path.

			\code{.cpp}
				// be sure to #include "path.h"

				// Create the PathMembers
				HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
				HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
				HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
				HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

				// Create the container of PathMembers
				std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

				HF::SpatialStructures::Path path(members);						// Create the Path object, path

				const int desired_index = 2;
				HF::SpatialStructures::PathMember result = path[desired_index];	// a copy of the element at desired_index
																				// within the
																				// internal members
																				// vector is
																				// assigned to result
			\endcode
		*/
		PathMember operator[](int i) const;

		/// <summary> Get a pointer to the path's underlying path members vector. </summary>
		/*!
			\code{.cpp}
				// be sure to #include "path.h"

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
			\endcode
		*/
		PathMember* GetPMPointer();
	};
};

namespace std {
	/*!
		\brief String representation of all the members contained by path p.

		\code{.cpp}
			// be sure to #include "path.h"

			// Create the PathMembers
			HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
			HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
			HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
			HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

			// Create the container of PathMembers
			std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

			HF::SpatialStructures::Path path(members);							// Create the Path object, path

			std::cout << path << std::endl;				// Output path to an ostream, like cout
			// Output is: (3) -3.14-> (2) -2.78-> (1) -1.64-> (7) -9.35->
		\endcode
	*/
	inline ostream& operator<<(ostream& os, const HF::SpatialStructures::Path p) {
		for (int i = 0; i < p.size(); i++) {
			const auto& pm = p[i];

			os << "(" << pm.node << ")";
			if (i != p.size()) {
				os << " -" << pm.cost << "-> ";
			}
		}
		os << std::endl;
		return os;
	}
}
