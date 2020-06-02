#include <Edge.h>

namespace HF {
	namespace SpatialStructures {
		Edge::Edge(const Node& Child, float Score, STEP Step_Type) :child(Child), step_type(Step_Type), score(Score) {};
	}
}