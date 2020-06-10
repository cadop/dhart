///
/// \file		edge.cpp
///	\brief		Source file for an edge data type, used by a graph ADT
///
/// \author		TBA
/// \date		06 Jun 2020
///
#include <Edge.h>

namespace HF {
	namespace SpatialStructures {
		Edge::Edge(const Node& Child, float Score, STEP Step_Type) :child(Child), step_type(Step_Type), score(Score) {};
	}
}