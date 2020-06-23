///
///	\file		path_finder.h
///	\brief		Header file for BoostGraphDeleter
///
///	\author		TBA
///	\date		17 Jun 2020
///
#include <memory>
#include <vector>

namespace HF {
	namespace SpatialStructures {
		class Graph;			///< see graph.h in spatialstructures
		class Path;				///< see path.h in spatialstructures
		class PathMember;		///< see path.h in spatialstructures
	}

	namespace Pathfinding {
		class BoostGraph; ///< Forward declared to prevent clients from importing boost
		
		/// <summary>
		/// Just deletes the BoostGraph. This way clients don't need any of the boost stuff
		/// (for std::unique_ptr<T, Deleter>, T == typename/class, Deleter == user-supplied struct which contains an operator() for destroying the pointee)
		/// </summary>
		struct BoostGraphDeleter {
		public:

			/// <summary>
			/// Used to destroy a BoostGraph
			/// </summary>
			/// <param name="bg">The address of a BoostGraph (pointer to BoostGraph)</param>

			/*!
				\code
					// be sure to #include "path_finder.h", #include "boost_graph.h",
					// #include "node.h", #include "graph.h", and #include <vector>

					// For this example, we must have a BoostGraph instance to use with BoostGraphDeleter.
					// In order to create a BoostGraph, we must first create a Graph instance first.
					// We must prepare the nodes, their edges, and the weights (distances) of each edge.

					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

					// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);

					// Passing Graph graph to BoostGraph bg, by reference
					HF::Pathfinding::BoostGraph bg(graph);

					//
					// Read below on BoostGraphDeleter -- this is important.
					//

					// BoostGraphDeleter is needed by std::unique_ptr to destroy a BoostGraph; it is not meant to be called directly.
					// You do not want to pass the address of a stack-allocated BoostGraph to BoostGraphDeleter::operator().

					// BoostGraphDeleter's operator() calls operator delete on the (BoostGraph *) argument,
					// and passing the address of a stack-allocated BoostGraph for said argument
					// will result in undefined behavior.

					//
					// In other words, do __not__ do the following:
					//

					HF::Pathfinding::BoostGraphDeleter bg_deleter;

					// Create a stack-allocated BoostGraph from a HF::SpatialStructures::Graph graph
					HF::Pathfinding::BoostGraph boostGraph(graph);	// not created using operator new

					// Using a BoostGraphDeleter on the (address of a) stack-allocated BoostGraph
					bg_deleter(&boostGraph);						// calls operator delete for a (BoostGraph *)

					// The type BoostGraphDeleter is only for use with std::unique_ptr.
					// See BoostGraph::CreateBoostGraph for BoostGraphDeleter's intended use.
				 \endcode
			*/
			void operator()(BoostGraph * bg) const;
		};

		/// <summary>
		/// Create a new boost graph from the given HFGraph
		/// <param name="g">The desired Graph used to create a BoostGraph</param>
		/// </summary>
		/// <returns>A smart pointer (unique_ptr) of type BoostGraph, created from the HFGraph
		
		/*!
			\code
				// be sure to #include "path_finder.h", #include "boost_graph.h",
				// #include "node.h", #include "graph.h", and #include <vector>

				// For this example, we must have a BoostGraph instance to use with BoostGraphDeleter.
				// In order to create a BoostGraph, we must first create a Graph instance first.
				// We must prepare the nodes, their edges, and the weights (distances) of each edge.

				// Create the nodes
				HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
				HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
				HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

				// Create a container (vector) of nodes
				std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

				// Create matrices for edges and distances, edges.size() == distances().size()
				std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
				std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

				// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
				HF::SpatialStructures::Graph graph(edges, distances, nodes);

				// Now we can create a smart pointer to a BoostGraph
				// Note the type of boostGraph - it is a
				//		std::unique_ptr<HF::Pathfinding::BoostGraph, HF::Pathfinding::BoostGraphDeleter>.
				// Use the auto keyword for type inference, or your choice of using statements/typedef to make
				// the use of the type described above easier.
				auto boostGraph = CreateBoostGraph(graph);
			\endcode
		*/
		std::unique_ptr<BoostGraph,BoostGraphDeleter> CreateBoostGraph(const HF::SpatialStructures::Graph & g);
		
		/// <summary>
		/// Find a path between points A and B
		/// </summary>
		/// <param name="bg"> The boost graph containing edges/nodes </param>
		/// <param name="start_id"> ID of the starting node </param>
		/// <param name="end_id"> ID of the ending node </param>
		/// <returns>A Path constructed using Dijkstra's Shortest Path algorithm</returns>
		
		/*!
			\code
				// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"

				// Create a Graph g, and compress it.
				HF::SpatialStructures::Graph g;
				g.addEdge(0, 1, 1);
				g.addEdge(0, 2, 2);
				g.addEdge(1, 3, 3);
				g.addEdge(2, 4, 1);
				g.addEdge(3, 4, 5);
				g.Compress();

				// Create a boostGraph from g
				auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

				// Get the path from node (id 0) to node (id 3)
				HF::SpatialStructures::Path path = HF::Pathfinding::FindPath(boostGraph.get(), 0, 3);

				// Print the nodes along the shortest path
				std::cout << "Shortest path from node id 0 to node id 3:" << std::endl;
				for (auto p : path.members) {
					std::cout << p.node << std::endl;
				}
			\endcode
		*/
		HF::SpatialStructures::Path FindPath(BoostGraph * bg, int start_id, int end_id);
		
		/// <summary>
		/// More efficient than calling FindPath manually. Organises paths by starting point,
		/// calculates only one predecessor matrix per unique starting point, then finds a 
		/// path for every pair.
		/// </summary>
		/// <param name="graph"> The boost graph containing edges/nodes </param>
		/// <param name=start_points> Ordered list of starting points </param>
		/// <param name="end_points"> Ordered list of ending points </param>
		/// <returns>a std::vector of Path, constructed using Dijkstra's Shortest Path algorithm</returns>

		/*!
			\code
				// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"

				// Create a Graph g, and compress it.
				HF::SpatialStructures::Graph g;
				g.addEdge(0, 1, 1);
				g.addEdge(0, 2, 2);
				g.addEdge(1, 3, 3);
				g.addEdge(2, 4, 1);
				g.addEdge(3, 4, 5);
				g.Compress();

				// Create a boostGraph from g
				auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

				// Prepare the parents and children vectors --
				// We will be searching for the shortest path from node 0 to node 3,
				// as well as the shortest path from node 0 to node 4.
				std::vector<int> parents = { 0, 0 };
				std::vector<int> children = { 3, 4 };

				std::vector<HF::SpatialStructures::Path> paths
					= HF::Pathfinding::FindPaths(boostGraph.get(), parents, children);

				// Get the shortest paths, which are already stored in paths
				auto path_0_3 = paths[0];
				auto path_0_4 = paths[1];

				// Print the shortest path from node 0 to node 3
				std::cout << "Shortest path from node id 0 to node id 3:" << std::endl;
				for (auto p : path_0_3.members) {
					std::cout << p.node << std::endl;
				}

				// Print the shortest path from node 0 to node 4
				std::cout << "Shortest path from node id 0 to node id 4:" << std::endl;
				for (auto p : path_0_4.members) {
					std::cout << p.node << std::endl;
				}
			\endcode
		*/
		std::vector<HF::SpatialStructures::Path> FindPaths(
			BoostGraph * bg,
			const std::vector<int> & start_points,
			const std::vector<int> & end_points
		);

		/// <summary>
		/// Find a path from every node to every node (NOTE: Not implemented yet.)
		/// </summary>
		/// <param name="bg">A BoostGraph for the desired traversal</param>
		/// <param name="start_id">Identifier for the starting node (start point)</param>
		/// <param name="end_id">Identifier for the ending node (end point)</param>
		/// <returns>A std::vector of Path</returns>

		/*!
			 \code
				// NOTE: HF::Pathfinding::FindAllPaths is not implemented yet.

				// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"

				// Create a Graph g, and compress it.
				HF::SpatialStructures::Graph g;
				g.addEdge(0, 1, 1);
				g.addEdge(0, 2, 2);
				g.addEdge(1, 3, 3);
				g.addEdge(2, 4, 1);
				g.addEdge(3, 4, 5);
				g.Compress();

				// Create a boostGraph from g
				auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

				// Get the path from node (id 0) to node (id 4)
				auto all_paths = HF::Pathfinding::FindAllPaths(boostGraph.get(), 0, 4);

				// all_paths will contain all shortest paths for [node 0, node 4]
			 \endcode
		*/
		std::vector<HF::SpatialStructures::Path> FindAllPaths(BoostGraph * bg, int start_id, int end_id);
		
		/// <summary>
		/// Runs Dijkstra's Shortest Path Algorithm on a (BoostGraph *) bg,
		///	and inserts the constructed paths into an array of (HF::SpatialStructures::Path *),
		/// as well as the PathMembers into an array of (HF::SpatialStructures::PathMember *).
		/// </summary>
		/// <param name="bg">Address of a BoostGraph</param>
		/// <param name="start_points">Vector of starting points</param>
		/// <param name="end_points">Vector of ending points</param>
		/// <param name="out_paths">Pointer to dynamically allocated array of HF::SpatialStructures::Path</param>
		/// <param name="out_path_members">Pointer to dynamically allocated array of HF::SpatialStructures::PathMember</param>

		/*!
			\code
				// be sure to #include "path_finder.h", #include "boost_graph.h", and #include "graph.h"

				// Create a Graph g, and compress it.
				HF::SpatialStructures::Graph g;
				g.addEdge(0, 1, 1);
				g.addEdge(0, 2, 2);
				g.addEdge(1, 3, 3);
				g.addEdge(2, 4, 1);
				g.addEdge(3, 4, 5);
				g.Compress();

				// Create a boostGraph from g
				auto boostGraph = HF::Pathfinding::CreateBoostGraph(g);

				// Prepare the parents and children vectors --
				// We will be searching for the shortest path from node 0 to node 3,
				// as well as the shortest path from node 0 to node 4.
				std::vector<int> parents = { 0, 0 };
				std::vector<int> children = { 3, 4 };

				// Create smart pointers to hold Path, PathMember and sizes
				const int MAX_SIZE = 4;
				std::unique_ptr<HF::SpatialStructures::Path[]> result_paths(new Path[MAX_SIZE]);
				std::unique_ptr<HF::SpatialStructures::PathMember[]> result_path_members(new PathMember[MAX_SIZE]);
				std::unique_ptr<int[]> result_sizes(new int[MAX_SIZE]);

				// Retrieve raw pointers so their addresses can be passed to InsertPathsIntoArray
				HF::SpatialStructures::Path* ppath = result_paths.get();
				HF::SpatialStructures::PathMember* pmembers = result_path_members.get();
				int* psizes = result_sizes.get();

				// Use InsertPathsIntoArray
				HF::Pathfinding::InsertPathsIntoArray(boostGraph.get(), parents, children, &ppath, &pmembers, psizes);
			\endcode
		*/
		void InsertPathsIntoArray(
			const BoostGraph* bg,
			const std::vector<int>& start_points,
			const std::vector<int>& end_points,
			HF::SpatialStructures::Path** out_paths,
			HF::SpatialStructures::PathMember** out_path_members,
			int* out_sizes
		);
	}
}
