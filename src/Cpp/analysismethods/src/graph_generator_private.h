///
/// \file		graph_generator_private.h
/// \brief		Contains definitions for the <see cref="HF::GraphGenerator::GraphGeneratorPrivate">GraphGeneratorPrivate</see> class
///
///	\author		TBA
///	\date		26 Jun 2020

#include <node.h>
#include <edge.h>
#include <graph.h>
#include <graph_generator.h>
#include <unique_queue.h>
#include <set>
namespace HF::GraphGenerator {
	/// <summary>
	/// Contains the private implementation of the GraphGenerator to insulate the clients from the implementation.
	/// </summary>
	/// 
	/*!
		\todo Combine this with the graphgenerator public and create a single class. 
	*/
	class GraphGeneratorPrivate {
	
	private:
		HF::SpatialStructures::Graph G;			  ///< An internal graph that is iterated upon as the program is executed
		HF::GraphGenerator::GraphGenerator & GG; ///< The underlying graph generator to hold all the settings

		/// <summary> Determine if a node is above a valid floor. </summary>
		/// <param name="position"> Node to check. </param>
		/// <returns> True if the node is above a valid floor, false othewise. </returns>
		/*! \details Identical to CheckRay, but without any side effects. (the value of position won't change).*/
		bool WalkableCheck(const SpatialStructures::Node& position);

		/// <summary>
		/// Push point to the ground if there is ground beneath it.
		/// </summary>
		/// <param name="start"> Start point to check and modify. </param>
		/// <returns> Returns true if CheckRay(start, down) is true, false otherwise </returns>
		/// 
		/*!
			\post 
			If true is returned, the start point is placed directly on top of the geometry below it.
			If false, the start point is unmodified.
			
			\todo Is this needed? This is identical to CheckRay.
		*/
		bool CheckStart(v3& start);

		/// <summary>
		/// Populate out_children with a potential child position for every direction in directions.
		/// </summary>
		/// <param name="parent"> Parent to generate children for. </param>
		/// <param name="directions">
		/// X,Y pairs to use for generating children. I.E. A pair of `{1,2}`would create a child
		/// at `{parent.x + 1*spacing.x, parent.y + 2*spacing.y, parent.z + spacing.z}.`
		/// </param>
		/// <param name="out_children"> Output parameter for children. </param>
		/// 
		/*!
			\post out_children is filled with children generated from parent and directions.

			\todo Potential speed gain here by using indexing instead of emplace_back.
		*/
		void GeneratePotentialChildren(
			const HF::SpatialStructures::Node& parent,
			const std::vector<std::pair<int, int>>& directions,
			std::vector<HF::SpatialStructures::Node>& out_children
		);
		
		/// <summary> Begin breadth first search to populate the graph with with nodes and edges. </summary>
		/// <param name="todo">
		/// Todo list to hold unchecked nodes. Must atleast contain a single start point.
		/// </param>
		/*! 
			\pre todo contains the starting point for the graph.
			
			\todo Extract the set logic for directions to another function.
			
			\see CrawlGeomParallel for a parallel version.
		*/
		void CrawlGeom(UniqueQueue & todo);

		/// <summary> A parallel version of CrawlGeom. </summary>
		/// <param name="todo">
		/// Todo list to use for execution. Must atleast contain a single start point.
		/// </param>
		/*! 
			\pre todo contains the start point for the graph.
			
			\todo Extract the set logic for directions to another function.
			
			\see CrawlGeom for a serial version.
		*/
		void CrawlGeomParallel(UniqueQueue& todo);

		/// <summary> Generate edges and children for parent in relations. </summary>
		/// <param name="parent"> Parent node to generate children from. </param>
		/// <param name="directions"> Directions to generate children in. </param>
		/// <param name="out_relations"> Output parameter for relations. </param>
		/*! 
			\post out_relation contains all valid edges between parent and its children.
		
			\see GeneratePotentialChildren for info on how directions are used to generate children. 
		*/
		inline void ComputerParent(
			const HF::SpatialStructures::Node& parent,
			const std::vector<std::pair<int, int>>& directions,
			std::vector<HF::SpatialStructures::Edge>& out_relations
		);

		/// <summary> Cast a ray and overwrite position with the result. </summary>
		/// <param name="position">
		/// Node to use as the origin. On Hit, this node's position will be overridden
		/// </param>
		/// <param name="direction"> Direction to fire the ray in </param>
		/// <param name="flag"> The type of geometry to intersect </param>
		/// <returns> True if the ray intersected any geometry. False otherwise. </returns>
		/*!
			\post If true is returned, position contains the point where the casted ray intersected geometry.
		*/
		bool CheckRay(
			HF::SpatialStructures::Node& position,
			const v3& direction,
			HIT_FLAG flag = HIT_FLAG::BOTH
		);
		
		/// <summary> Cast a ray and overwrite position with the point of intersection. </summary>
		/// <param name="position">
		/// Node to use as the origin. On intersection, this node's position will be overridden
		/// with the point of intersection.
		/// </param>
		/// <param name="direction"> Direction to cast the ray in. </param>
		/// <param name="flag">
		/// The type of geometry to collide with. See HIT_FLAG for more details.
		/// </param>
		/// <returns> True on hit, false otherwise. </returns>
		/*!
			\post If true is returned, position contains the point where the casted ray intersected geometry.

			\exception std::exception if HIT_FLAG isn't a valid value.
		*/
		bool CheckRay(
			v3& position,
			const v3& direction,
			HIT_FLAG flag = HIT_FLAG::BOTH
		);

		/// <summary> Check if there is an obstruction between parent and child. </summary>
		/// <param name="parent"> Parent node to cast the ray from. </param>
		/// <param name="child"> Child node to cast the ray to. </param>
		/// <returns> True if there is an obstacle blocking line of sight between between parent and child. </returns>
		/*! This function casts an occlusion ray from parent to child with a maximum distance
			equal to the distance between both nodes.
		*/
		bool OcclusionCheck
		(
			const SpatialStructures::Node & parent,
			const SpatialStructures::Node & child
		);
		
		/// <summary> Determine if the graph can traverse the slope from n1 to n2. </summary>
		/// <param name="n1"> Node that's being traversed from. </param>
		/// <param name="n2"> Node that's being traversed to. </param>
		/// <returns> True if the slope between n1 and n2 is within the limits in our settings. 
		/// False otherwise. </returns>
		/// \todo Rename this function to SlopeCheck, or SlopeCalc like the function it's based on in 
		/// the original python code. 
		bool isUpSlope(
			const HF::SpatialStructures::Node& n1,
			const HF::SpatialStructures::Node& n2
		);

		/// <summary> Create all possible edges from parent to possible_children. </summary>
		/// <param name="parent"> Parent node used to generate contents of possible_children. </param>
		/// <param name="possible_children">
		/// Possible children for parent. Usually generated from GeneratePotentialChildren.
		/// </param>
		/// <param name="out_children">
		/// An output array for valid edges in after they've been evaluated.
		/// </param>
		/*!
			\details 
			All children are offset to the ground using \link CheckChildren \endlink, then a connection to parent
			is checked for using CheckConnection. If CheckChildren or CheckConnection is not successful
			for a child node, then it will not have an edge created in out_children.

			\post 
			out_children contains a set of all valid edges between parent and the children in possible_children.
		*/
		void GetChildren(
			const HF::SpatialStructures::Node & parent,
			const std::vector<SpatialStructures::Node>& possible_children,
			std::vector<HF::SpatialStructures::Edge>& out_children
		);

		/// <summary>
		/// Offset all children to the ground, then check if traversing from parent would
		/// require a step outside of step height limits.
		/// </summary>
		/// <param name="parent"> Parent of all children in children. </param>
		/// <param name="children"> Child nodes to check. </param>
		/// <returns>
		/// All children that are above a valid floor and can be traversed to by parent without
		/// requiring a step outside of step height limits.
		/// </returns>
		/*!
			\todo Why is Children not const?
		*/
		std::vector<HF::SpatialStructures::Node> CheckChildren(
			const HF::SpatialStructures::Node& parent,
			std::vector<HF::SpatialStructures::Node> Children
		);


		/*!
			\brief Check if parent can traverse to child.  
			
			\param parent Parent of child.
			\param child Child generated from parent.

			\returns The type of STEP from parent to child.

			\details
			Specifically, this function checks for a direct line of sight. If the line of sight check succeeds, then
			it checks to see if the slope between parent and child is within the defined limits. If not, then
			it will offset the child node upward by upstep if it's higher than the parent, or downward by downstep
			if it's lower than parent and try the occlusion check again. 

			\see STEP for details on each type of step.
		*/
		HF::SpatialStructures::STEP CheckConnection(
			const HF::SpatialStructures::Node& parent,
			const HF::SpatialStructures::Node& child
		);

	public:
		/// <summary> Construct a graphgeneratorprivate with the settings of GG. </summary>
		/// <param name="GG"> GraphGenerator containing settings to use when BuildNetwork is called. </param>
		GraphGeneratorPrivate(GraphGenerator & GG);

		/// <summary> Conduct a breadth first search of traversable space. </summary>
		/// <returns> The Graph generated using the settings from GG. </returns>
		/// \see GraphGenerator::BuildNetwork
		HF::SpatialStructures::Graph BuildNetwork();
	};
}
