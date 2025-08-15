///
/// \file		edge.cpp
/// \brief		Contains implementation for the <see cref = "HF::SpatialStructures::Edge">Edge</see> structure
///
/// \author		TBA
/// \date		06 Jun 2020

#include <edge.h>

namespace HF {
	namespace SpatialStructures {
		Edge::Edge(const Node& Child, float Score, STEP Step_Type) :child(Child), step_type(Step_Type), score(Score) {};
	}
}