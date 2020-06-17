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

			/// \code{.cpp}
			///	// BoostGraphDeleter is needed by std::unique_ptr to destroy a BoostGraph; it is not meant to be called directly.
			/// // You do not want to pass the address of a stack-allocated BoostGraph to BoostGraphDeleter::operator().
			///
			/// // BoostGraphDeleter's operator() calls operator delete on the (BoostGraph *) argument,
			/// // and passing the address of a stack-allocated BoostGraph for said argument
			/// // will result in undefined behavior.
			///
			///	// In other words, don't do this:
			///
			///	BoostGraphDeleter bg_deleter;
			///	
			///	// Create a stack-allocated BoostGraph from a HF::SpatialStructures::Graph g
			///	BoostGraph boostGraph(g);		// not created using operator new
			///
			///	// Using a BoostGraphDeleter on the (address of a) stack-allocated BoostGraph
			/// bg_deleter(&boostGraph);		// calls operator delete for a (BoostGraph *)
			///
			/// // The type BoostGraphDeleter is only for use with std::unique_ptr.
			/// // See BoostGraph::CreateBoostGraph for BoostGraphDeleter's intended use.
			/// \endcode
			void operator()(BoostGraph * bg) const;
		};

		/// <summary>
		/// Create a new boost graph from the given HFGraph
		/// <param name="g">The desired Graph used to create a BoostGraph</param>
		/// </summary>
		/// <returns>A smart pointer (unique_ptr) of type BoostGraph, created from the HFGraph

		/// \code{.cpp}
		/// We must first have a HF::SpatialStructures::Graph instance first.
		///
		/// using HF::SpatialStructures::Node;			// for Node
		/// using HF::SpatialStructures::Graph;			// for Graph
		///
		/// // Create the nodes
		/// Node node_0(12.0, 23.1, 34.2, 456);
		/// Node node_1(45.3, 56.4, 67.5, 789);
		/// Node node_2(55.5, 25.1, 85.2, 940);
		///
		///	// Create a container (vector) of nodes
		/// std::vector<Node> nodes = { node_0, node_1, node_2 };
		///
		/// // Create matrices for edges and distances, edges.size() == distances().size()
		///	std::vector<std::vector<int>> edges = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9 } };
		/// std::vector<std::vector<float>> distances = { {12.0, 23.1, 45.2}, {67.3, 89.4, 98.5}, {76.6, 54.7, 32.8} };
		///
		///	// Create a Graph - note that nodes, edges, and distances are passed by reference
		/// Graph g(edges, distances, nodes);
		///
		/// // Now we can create a BoostGraph - note that Graph g is passed by reference
		/// std::unique_ptr<BoostGraph, BoostGraphDeleter> boostGraph = CreateBoostGraph(g);
		/// \endcode
		std::unique_ptr<BoostGraph,BoostGraphDeleter> CreateBoostGraph(const HF::SpatialStructures::Graph & g);
		
		/// <summary>
		/// Find a path between points A and B
		/// </summary>
		/// <param name="bg"> The boost graph containing edges/nodes </param>
		/// <param name="start_id"> ID of the starting node </param>
		/// <param name="end_id"> ID of the ending node </param>
		/// <returns>A Path constructed using Dijkstra's Shortest Path algorithm</returns>

		/// \code{.cpp}
		/// // We must first have a HF::SpatialStructures::Graph instance first.
		///
		/// using HF::SpatialStructures::Node;			// for Node
		/// using HF::SpatialStructures::Graph;			// for Graph
		///	using HF::SpatialStructures::Path;			// for Path
		///
		/// // Create the nodes
		/// Node node_0(12.0, 23.1, 34.2, 456);
		/// Node node_1(45.3, 56.4, 67.5, 789);
		/// Node node_2(55.5, 25.1, 85.2, 940);
		///
		///	// Create a container (vector) of nodes
		/// std::vector<Node> nodes = { node_0, node_1, node_2 };
		///
		/// // Create matrices for edges and distances, edges.size() == distances().size()
		///	std::vector<std::vector<int>> edges = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9 } };
		/// std::vector<std::vector<float>> distances = { {12.0, 23.1, 45.2}, {67.3, 89.4, 98.5}, {76.6, 54.7, 32.8} };
		///
		///	// Create a Graph - note that nodes, edges, and distances are passed by reference
		/// Graph g(edges, distances, nodes);
		///
		/// // Now we can create a BoostGraph - note that Graph g is passed by reference
		/// std::unique_ptr<BoostGraph, BoostGraphDeleter> boostGraph = CreateBoostGraph(g);
		///
		/// // Prepare the range of IDs (from Nodes)
		/// int id_begin = node_0.id;		// id_begin == 456
		/// int id_end = node_2.id;			// id_end == 940
		/// 
		///	// Use the std::unique_ptr::get function to retrieve a raw pointer for FindPath
		///	Path found_path = FindPath(boostGraph.get(), id_begin, id_end);
		/// \endcode
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

		/// \code{.cpp}
		/// using HF::SpatialStructures::Path;		// for Path
		/// using HF::SpatialStructures::Graph;		// for Graph
		///
		///	// Build a Graph g with the following edges, then compress g
		/// Graph g;
		/// g.addEdge(0, 1, 100);
		/// g.addEdge(0, 2, 50);
		/// g.addEdge(1, 3, 10);
		/// g.addEdge(2, 3, 10);
		/// g.Compress();
		///
		/// // Create a std::unique_ptr<BoostGraph, BoostGraphDeleter> from an HF::SpatialStructures::Graph g
		///	std::unique_ptr<BoostGraph, BoostGraphDeleter> boostGraph = CreateBoostGraph(g);
		///	
		///	// Create a vector of 100 int, initialized to value 0
		///	std::vector<int> start_vec(100, 0);
		///
		/// // Create a vector of 100 int, initialized to value 3
		///	std::vector<int> end_vec(100, 3);
		///
		///	// Use std::unique_ptr's get member function to retrieve a raw pointer
		/// std::vector<Path> found_paths = FindPaths(boostGraph.get(), start_vec, end_vec);
		/// \endcode
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

		/// \code{.cpp}
		/// using HF::SpatialStructures::Path;		// for Path
		/// using HF::SpatialStructures::Graph;		// for Graph
		///
		///	// Build a Graph g with the following edges, then compress g
		/// Graph g;
		/// g.addEdge(0, 1, 100);
		/// g.addEdge(0, 2, 50);
		/// g.addEdge(1, 3, 10);
		/// g.addEdge(2, 3, 10);
		/// g.Compress();
		///
		/// // Create a std::unique_ptr<BoostGraph, BoostGraphDeleter> from an HF::SpatialStructures::Graph g
		///	std::unique_ptr<BoostGraph, BoostGraphDeleter> boostGraph = CreateBoostGraph(g);
		///	
		///	// Prepare range of IDs
		///	int begin_id = 0;
		/// int end_id = 3;
		///
		///	// Use std::unique_ptr's get member function to retrieve a raw pointer
		/// std::vector<Path> found_paths = FindAllPaths(boostGraph.get(), begin_id, end_id);
		/// \endcode
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

		/// \code{.cpp}
		/// using HF::SpatialStructures::Path;			// for Path
		/// using HF::SpatialStructures::PathMember;	// for PathMember
		/// using HF::SpatialStructures::Graph;			// for Graph
		///
		///	// Build a Graph g with the following edges, then compress g
		/// Graph g;
		/// g.addEdge(0, 1, 100);
		/// g.addEdge(0, 2, 50);
		/// g.addEdge(1, 3, 10);
		/// g.addEdge(2, 3, 10);
		/// g.Compress();
		///
		/// // Create a std::unique_ptr<BoostGraph, BoostGraphDeleter> from an HF::SpatialStructures::Graph g
		///	std::unique_ptr<BoostGraph, BoostGraphDeleter> boostGraph = CreateBoostGraph(g);
		///	
		///	// Create a vector of 100 int, initialized to value 0
		///	std::vector<int> start_vec(100, 0);
		///
		/// // Create a vector of 100 int, initialized to value 3
		///	std::vector<int> end_vec(100, 3);
		///
		///	// Create smart pointers to hold Paths, PathMembers, and the sizes
		/// std::unique_ptr<Path[]> result_paths(new Path[MAX_SIZE]);
		/// std::unique_ptr<PathMember[]> result_path_members(new PathMember[MAX_SIZE]);
		/// std::unique_ptr<int[]> result_sizes(new int[MAX_SIZE]);
		///
		///	// Retrieve raw pointers so their addresses can be passed to InsertPathsIntoArray
		///	Path* ppath = result_paths.get();
		///	PathMember* pmembers = result_path_members.get();
		/// int* psizes = result_sizes.get();
		///
		///	// Call InsertPathsIntoArray
		///	InsertPathsIntoArray(boostGraph.get(), start_vec, end_vec, &ppath, &pmembers, psizes);
		///
		/// // No need to call operator delete on result_paths, result_path_members, or result_sizes,
		/// // as their memory resources are handled by std::unique_ptr
		/// \endcode
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
