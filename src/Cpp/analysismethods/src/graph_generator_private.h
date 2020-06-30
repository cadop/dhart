///
/// \file		graph_generator_private.h
/// \brief		Contains definitions for the <see cref="HF::AnalysisMethods::GraphGeneratorPrivate">GraphGeneratorPrivate</cref> class
///
///	\author		TBA
///	\date		26 Jun 2020

#include <node.h>
#include <edge.h>
#include <graph.h>
#include <graph_generator.h>
#include <unique_queue.h>
#include <set>
namespace HF{
	namespace AnalysisMethods {
		/// <summary>
		/// Contains the private implementation of the graph generator so clients aren't forced to recompile
		/// </summary>
		class GraphGeneratorPrivate {
		
		private:
			HF::SpatialStructures::Graph G;			  ///< An internal graph that is iterated upon as the program is executed ///<
			HF::AnalysisMethods::GraphGenerator & GG; ///< The underlying graph generator; ///<

			/// <summary>
			/// Invokes CheckRay using a temporary instance of v3 (named testpos), constructed from position
			/// </summary>
			/// <param name="position">The node to use for CheckRay</param>
			/// <returns>See CheckRay, returns value from CheckRay(testpos, down, FLOORS)</returns>
			bool WalkableCheck(const SpatialStructures::Node& position);

			/// <summary>
			/// Calculate positions for every direction pair in directions, then deposit in out children
			/// </summary>
			/// <param name="parent">The parent node</param>
			/// <param name="directions">A reference to a direction pair (coordinates)</param>
			/// <param name="out_children">A reference to a vector of Node where children will be deposited</param>
			void Graphdirecs(const SpatialStructures::Node& parent, const std::vector<std::pair<int, int>>& directions, std::vector<SpatialStructures::Node>& out_children);

			///<summary>
			/// Push point to the ground if there is ground beneath it and return true. Return false if it isn't on any walkable terrain.
			///</summary>
			/// <param name="start">Reference to v3 instance</param>
			/// <returns>Returns true if CheckRay(start, down) is true, false otherwise</returns>
			bool CheckStart(v3& start);

			/// <summary>
			/// Populate out_children with a potential child position for every direction in directions.
			/// </summary>
			/// <param name="parent">parent to start from</param>
			/// <param name="directions">An array of pairs for the x and y direction</param>
			/// <param name="out_children">Output array</param>
			void GeneratePotentialChildren(const HF::SpatialStructures::Node& parent, const std::vector<std::pair<int, int>>& directions, std::vector<HF::SpatialStructures::Node>& out_children);

			
			/// <summary>
			/// Populate the graph with with nodes and edges
			/// </summary>
			/// <param name="todo"> A unique queue with a single start point</param>
			void CrawlGeom(UniqueQueue & todo);

			/// <summary>
			/// A parallel version of CrawlGeom
			/// </summary>
			/// <param name="todo"></param>
			/// <returns></returns>
			void CrawlGeomParallel(UniqueQueue& todo);

			/// <summary>
			/// Ranks children then stores relations?
			/// </summary>
			/// <param name="parent">The desired parent node</param>
			/// <param name="directions">A reference to a vector of integer pairs, representing directions</param>
			/// <param name="out_relations">A reference to a vector of Edge, representing relations</param>
			inline void ComputerParent(const HF::SpatialStructures::Node& parent, const std::vector<std::pair<int, int>>& directions, std::vector<HF::SpatialStructures::Edge>& out_relations);

			/// <summary>
			/// Fire a ray using the Embree Ray Tracer
			/// </summary>
			/// <param name="position">Node to use as the origin. On Hit, this node's position will be overridden</param>
			/// <param name="direction">Direction to fire the ray in</param>
			/// <param name="flag">Fire at floors or obstacles</param>
			/// <returns>True on hit, false otherwise</returns>
			bool CheckRay(HF::SpatialStructures::Node& position, const v3& direction, HIT_FLAG flag = HIT_FLAG::BOTH);
			
			/// <summary>
			/// Fire a ray using the Embree Ray Tracer
			/// </summary>
			/// <param name="position">Node to use as the origin. On Hit, this node's position will be overridden</param>
			/// <param name="direction">Direction to fire the ray in</param>
			/// <param name="flag">Fire at floors or obstacles</param>
			/// <returns>True on hit, false otherwise</returns>
			bool CheckRay(v3& position, const v3& direction, HIT_FLAG flag = HIT_FLAG::BOTH);

			/// <summary>
			/// Check if parent has a line of sight to child within max_dist
			/// </summary>
			/// <param name="parent"> Node to shoot from</param>
			/// <param name="child">Node to shoot to</param>
			/// <returns>True if there is an unobstructed path between parent and child</returns>
			bool OcclusionCheck(const SpatialStructures::Node & parent, const SpatialStructures::Node & child);
			
			/// <summary>
			/// Check the floor
			/// </summary>
			/// <param name="parent">The desired parent</param>
			/// <param name="child">The desired child</param>
			/// <returns>True if the child isn't lower than the maximum z coordinate offset, false otherwise</returns>
			bool CheckFloor(const HF::SpatialStructures::Node& parent, HF::SpatialStructures::Node& child);

			/// <summary>
			/// Returns true if the slope is up, and false otherwise
			/// </summary>
			/// <param name="n1">The first node to compare</param>
			/// <param name="n2">The second node to compare</param>
			/// <returns>True, if the slope of n1 and n2 is positive (upward), false otherwise</returns>
			bool isUpSlope(const HF::SpatialStructures::Node& n1, const HF::SpatialStructures::Node& n2);

			/// <summary>
			/// Calculates children for parent and places them in out_children
			/// </summary>
			/// <param name="parent">Reference to parent node</param>
			/// <param name="possible_children">Reference to vector of node, representing possible children</param>
			/// <param name="out_children">Reference to vector of Edge, representing edges of children</param>
			void GetChildren(const HF::SpatialStructures::Node & parent, const std::vector<SpatialStructures::Node>& possible_children, std::vector<HF::SpatialStructures::Edge>& out_children);

			/// <summary>
			/// Fire a ray from the parent to each child node to see if it satisfies step requirements
			/// </summary>
			/// <param name="parent">The parent node to fire the ray from</param>
			/// <param name="children">A list of child nodes to check</param>
			/// <returns>A list of valid nodes</returns>
			std::vector<HF::SpatialStructures::Node> CheckChildren(const HF::SpatialStructures::Node& parent, std::vector<HF::SpatialStructures::Node> Children);

			/// <summary>
			/// Check if there is a connection between each node
			/// </summary>
			/// <param name="parent">The desired parent node</param>
			/// <param name="child">The desired child node</param>
			/// <returns>True if parent and child are connected, false otherwise</returns>
			bool IsConnected(const HF::SpatialStructures::Node& parent, const HF::SpatialStructures::Node& child);
			
			/// <summary>
			/// Get the distance between the node and where the ray hits. If the ray doesn't hit return 0
			/// </summary>
			/// <param name="p1">The desired node to assess</param>
			/// <param name="direction">The direction of the ray</param>
			/// <param name="flag">Ray collision status, 1 = FLOORS, 2 = OBSTACLES, 3 = BOTH (the default)</param>
			/// <returns>Distance between the node and the ray collision -- 0.0f if ray does not hit</returns>
			float RayDist(const HF::SpatialStructures::Node& p1, const v3& direction, HIT_FLAG flag = HIT_FLAG::BOTH);

			///<summary>
			/// Check if the parent and child have a connection between them. May move nodes around to fit on ground mesh.
			///</summary>
			HF::SpatialStructures::STEP CheckConnection(const HF::SpatialStructures::Node& parent, const HF::SpatialStructures::Node& child);

		public:

			/// <summary>
			/// Default constructor
			/// </summary>
			/// <param name="GG">The GraphGenerator used to create this instance</param>
			GraphGeneratorPrivate(GraphGenerator & GG);

			/// <summary>
			/// TODO description
			/// </summary>
			/// <returns>TODO returns</returns>
			HF::SpatialStructures::Graph BuildNetwork();
		};
	}
}
