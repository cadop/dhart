///
/// \file		Edge.h
/// \brief		Contains definitions for the <see cref="HF::SpatialStructures::Edge">Edge</see> structure
///
///	\author		TBA
///	\date		26 Jun 2020
#pragma once

#include <node.h>
#include <vector>

namespace HF {
	namespace SpatialStructures {
		/// <summary> Describes the type of step an edge connects to. </summary>
		enum STEP {
			NOT_CONNECTED = 0, ///< No connection between parent and child.
			NONE = 1,		 ///< Parent and child are on the same plane and no step is required.
			UP = 2,			///< A step up is required to get from parent to child.
			DOWN = 3,		///< A step down is required to get from parent to child.
			OVER = 4		///< A step over something is required to get from parent to child.
		};

		/// <summary> A connection to a child node. </summary>
		/*!
			\remarks To save memory, edge contains no parent node. This is usually because edges can be
			placed in arrays to match the order of parent nodes.
		
		*/
		struct Edge {
			Node child;			///< The child node for this edge.
			STEP step_type;		///< Step required to traverse this edge \see STEP for info on every step type. 
			float score;		///< The cost required to traverse this edge.

			/// <summary> Construct a new edge to child. </summary>
			/// <param name="Child"> The node being traversed to. </param>
			/// <param name="Score"> The cost to traverse from parent to child. </param>
			/// <param name="Step_Type">
			/// The type of step required to traverse from parent to child.
			/// </param>
			/*!
				\code
					// be sure to #include "Edge.h"
			
					HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);
					float score = 4.3f;
			
					HF::SpatialStructures::Edge edge(node, score, STEP::NOT_CONNECTED);
				\endcode
			*/
			Edge(const Node& Child, float Score = 0, STEP Step_Type = NONE);
		};


		/// <summary> A lighter version of Edge that contains an ID instead of a full node object.. </summary>
		struct IntEdge {
			int child;      ///< Identifier of child node
			float weight;	///< Cost to traverse to child. 
		};

		/// <summary> A collection of edges and a parent. </summary>
		/*!
			\remarks
			This can to represent a node and its edges without repeating any information such
			as parent_id or node objects.
		*/
		struct EdgeSet {
			int parent;						///< Identifier of parent node
			std::vector<IntEdge> children;	///< vector of IntEdge (children)
		};
	}
}
