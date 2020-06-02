#pragma once
#include <vector>
#include <iostream>


namespace HF::SpatialStructures
{
	struct PathMember {
		float cost;
		int node;
		inline bool operator==(const PathMember &  p2) const {
			return(cost == p2.cost && node == p2.node);
		}

		inline bool operator!=(const PathMember& p2) const {
			return (!((*this) == p2));
		}
	};

	/// <summary>
	/// A point in space with an ID
	/// </summary>
	struct Path {
	public:
		std::vector<PathMember> members;
		Path::Path() {};

		/// <summary>
		/// Construct a path from an ordered list of path members
		/// </summary>
		Path::Path(const std::vector<PathMember> pm);

		void AddNode(int node, float cost);
		bool empty() const;
		int size() const;
		void Reverse();

		bool operator ==(const Path & p2) const;
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
