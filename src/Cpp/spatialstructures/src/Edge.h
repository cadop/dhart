///
/// \file		Edge.h
///	\brief		Header file for an Edge data type, used by a graph ADT
///
/// \author		TBA
/// \date		06 Jun 2020
///
#pragma once

#include <node.h>
#include <vector>

namespace HF {
	namespace SpatialStructures {
		/// <summary>
		/// Describes the type of step an edge connects to 
		/// </summary>
		enum STEP {
			NOT_CONNECTED = 0,
			NONE = 1,
			UP = 2,
			DOWN = 3,
			OVER = 4
		};

		// TODO: Should this be a private class?
		/// <summary>
		/// Indicates a connection between two nodes
		/// </summary>
		struct Edge {
			Node child;			///< The child node for this edge
			STEP step_type;		///< see enum STEP, valid range is [0, 4]
			float score;

			/// <summary>
			/// Construct a new edge
			/// </summary>
			/// <param name="Child"> the node that this edge connects to </param>
			/// <param name="Score"> the score of this edge</param>
			/// <param name="Step_Type">The type of step </param>

			/// \code{.cpp}
			/// Node node(12.0, 23.1, 34.2, 456);
			/// float score = 4.3f;
			///
			/// Edge edge(node, score, STEP::NOT_CONNECTED);
			/// \endcode
			Edge(const Node& Child, float Score = 0, STEP Step_Type = NONE);
		};


		/// <summary>
		/// Like an edge, but without the node attached since that isn't always relevant.
		/// </summary>
		struct IntEdge {
			int child;      ///< Identifier of child node
			float weight;	///< Weight of edge
		};

		/// <summary>
		/// A collection of edges for a specific node
		///</summary>
		struct EdgeSet {
			int parent;						///< Identifier of parent node
			std::vector<IntEdge> children;	///< vector of IntEdge (children)
		};
	}
}
