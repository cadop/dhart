#pragma once
#include <vector>
#include <iostream>


namespace HF::SpatialStructures
{
	struct PathMember {
		float cost;		///< The cost of a PathMember (weight) ///<
		int node;		///< The node identifier ///<

		/// <summary>
		/// Determines if PathMembers p1 and p2 have equal cost and node (identifier) values
		/// </summary>
		/// <param name="p2">The PathMember to compare with p1</param>
		/// <returns>True if p1 and p2 have equivalent cost and node values, false otherwise
		inline bool operator==(const PathMember &  p2) const {
			return(cost == p2.cost && node == p2.node);
		}

		/// <summary>
		/// See operator==, determines if PathMember p1 and p2 do NOT have equal cost/node values
		/// </summary>
		/// <param name="p2">The PathMember to compare with p1</param>
		/// <returns>True if p1 and p2 do NOT have equivalent cost/node values, false otherwise
		inline bool operator!=(const PathMember& p2) const {
			return (!((*this) == p2));
		}
	};

	/// <summary>
	/// A point in space with an ID
	/// </summary>
	struct Path {
	public:
		std::vector<PathMember> members;		///< The underlying vector of PathMember ///<

		/// <summary>
		/// Default constructor, empty block
		/// </summary>
		Path::Path() {};

		/// <summary>
		/// Construct a path from an ordered list of path members
		/// </summary>
		Path::Path(const std::vector<PathMember> pm);

		/// <summary>
		/// Constructs a PathMember and appends it to the underlying members vector
		/// </summary>
		/// <param name="node">The identifier for the PathMember</param>
		/// <param name="cost">The cost (weight) for the PathMember</param>
		void AddNode(int node, float cost);
		
		/// <summary>
		/// Determines if the underlying members vector is empty, or not
		/// </summary>
		/// <returns>True if the underyling members vector is empty, false otherwise</returns>
		bool empty() const;
		
		/// <summary>
		/// Retrieves the length of the underlying members vector (PathMember count)
		/// </summary>
		/// <returns>The value returned by members.size() - the current PathMember count
		int size() const;

		/// <summary>
		/// Invokes std::reverse to reverse the contents of the underlying members vector, using iterators
		/// </summary>
		void Reverse();

		/// <summary>
		/// Determines if all PathMember in p1 and p2's underlying PathMember vector are equivalent, or not, see operator== for PathMember
		/// </summary>
		/// <param name="p2">The Path to compare against p1 (or rather, the elements of their PathMember vectors)</param>
		/// <returns>True, if all PathMember elements in p2 are equal to all PathMember elements in p1, false otherwise
		bool operator==(const Path & p2) const;

		/// <summary>
		/// Retrieve a PathMember from the underlying PathMember vector, by value
		/// </summary>
		/// <param name="i">The index for the desired PathMember element within members
		/// <returns>A PathMember within the members vector, at index i, by value
		PathMember operator[](int i) const;
		
		/// <summary>
		/// Return the pointer to the underlying path member vector
		/// </summary>
		PathMember* GetPMPointer();
	};	
};

namespace std {
	inline ostream & operator<<(ostream& os, const HF::SpatialStructures::Path p) {
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
