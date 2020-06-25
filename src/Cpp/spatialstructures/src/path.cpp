///
/// \file		path.cpp
///	\brief		Source file for Path/PathMember data types
///
/// \author		TBA
/// \date		06 Jun 2020
///
#include <path.h>
#include <algorithm>

namespace HF::SpatialStructures {
	void Path::AddNode(int node, float cost) {
		members.push_back(PathMember{ cost, node });
	}

	bool Path::empty() const {
		return members.size() == 0;
	}

	int Path::size() const {
		return members.size();
	}

	void Path::Reverse() {
		std::reverse(members.begin(), members.end());
	}

	bool Path::operator==(const Path& P2) const {

		// Return early if the sizes of this and P2 don't match
		if (P2.size() != this->size()) return false;

		// Otherwise, comapre elements one by one
		for (int i = 0; i < this->size(); i++)
		{
			auto our_pm = (*this)[i];
			auto their_pm = P2[i];

			if (our_pm != their_pm)
				return false;
		}

		// If we've gotten this far, then all path members
		// must be equal.
		return true;
	}

	PathMember Path::operator[](int i) const {
		return this->members[i];
	}

	Path::Path(const std::vector<PathMember> pm) {
		this->members = pm;
	}

	PathMember * Path::GetPMPointer() {
		return this->members.data();
	}
}