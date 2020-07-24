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

			/*! \brief Default Constructor*/
			Edge() {};
		};


		/// <summary> A lighter version of Edge that contains an ID instead of a full node object.. </summary>
		struct IntEdge {
			int child;      ///< Identifier of child node
			float weight;	///< Cost to traverse to child. 


			inline bool operator==(const IntEdge& IE2) const {
				return (
					this->child == IE2.child
					&& abs(this->weight - IE2.weight) < 0.0001
				);
			}

			inline bool operator!=(const IntEdge& IE2) const { return !(this->operator==(IE2)); }
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

			/*! \brief Empty Constructor*/
			EdgeSet() { parent = -1; };


			/*!\brief Construct an edge set with a list of int edges and a parent*/
			inline EdgeSet(int parent, const std::vector<IntEdge>& edges) {
				this->AddEdges(edges);
				this->parent = parent;
			}

			/*! \brief Get the number of children in this edgeset. */
			inline int size() const { return children.size(); }

			/*! \brief Add a set of edges to the array of children. */
			inline void AddEdges(const std::vector<IntEdge> & edges) {
				// If we have no edges, just overwrite our existing array
				if (children.size() == 0)
					children = edges;
				// Otherwise resize and copy in
				else {
					children.resize(children.size() + edges.size());
					std::copy(edges.begin(), edges.end(), children.begin() + children.size());
				}
			}

			/*! \brief Check the equality of two edge sets.*/
			inline bool operator==(const EdgeSet & ES2) const {
				if (this->parent != ES2.parent) return false;
				if (this->children.size() != ES2.children.size()) return false;
				
				for (int i = 0; i < this->children.size(); i++)
					if (this->children[i] != ES2.children[i])
						return false;
				return true;
			}
		};
	}
}
