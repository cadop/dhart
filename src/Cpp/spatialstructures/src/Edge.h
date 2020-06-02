#pragma once
#include <node.h>
#include <vector>
namespace HF {
	namespace SpatialStructures {
		/// <summary>
		/// Describes the type of step an edge conencts to 
		/// </summary>
		enum STEP {
			NOT_CONNECTED = 0,
			NONE = 1,
			UP = 2,
			DOWN = 3,
			OVER = 4
		};

		//TODO: Should this be a private class?
		/// <summary>
		/// Indicates a connection between two nodes
		/// </summary>
		struct Edge {
			Node child;
			STEP step_type;
			float score;

			/// <summary>
			/// Construct a new edge
			/// </summary>
			/// <param name="Child"> the node that this edge connects to </param>
			/// <param name="Score"> the score of this edge</param>
			/// <param name="Step_Type">The type of step </param>
			Edge(const Node& Child, float Score = 0, STEP Step_Type = NONE);
		};


		/// <summary>
		/// Like an edge, but without the node attached since that isn't always relevant.
		/// </summary>
		struct IntEdge {
			int child;
			float weight;
		};
		/// <summary>
		/// A collection of edges for a specific node
		///</summary>
		struct EdgeSet {
			int parent;
			std::vector<IntEdge> children;
		};
	}
}
