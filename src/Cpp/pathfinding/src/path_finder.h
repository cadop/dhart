///
///	\file		path_finder.h
/// \brief		Contains definition for the <see cref="HF::Pathfinding::BoostGraphDeleter">BoostGraphDeleter</see> structure
///
///	\author		TBA
///	\date		17 Jun 2020
///

#include <memory>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <cmath>

namespace HF {
	// Forward declares so we don't need to include these in the header.
	namespace SpatialStructures {
		class Graph;
		class Path;
		class PathMember;
	}

	/*! 
		\brief Algorithms to find the shortest path between nodes in a HF::SpatialStructures::Graph. 
	
		\details
		This namespace utilizes pathfinding algorithms from Boost internally.To make use of the algorithms
		in this namespace, first you must create a BoostGraph from the HF::SpatialStructures::Graph you 
		want to find paths in.		

		\see HF::SpatialStructures::Path for a description of the path datatype.
		\see BoostGraph for details on generating a BoostGraph from a HF::SpatialStructures::Graph.
	*/
	namespace Pathfinding {
		class BoostGraph; // Forward declared to prevent clients from importing boost. 	

		/*!
			\brief Deleter for the BoostGraph. 

			\remarks
			 BoostGraphDeleter is needed by std::unique_ptr to destroy a  BoostGraph; it is not meant to be called directly. 
		
			\remarks Implementation based on standard library reference for unique pointer
			https://en.cppreference.com/w/cpp/memory/unique_ptr

			\warning BoostGraphDeleter is only for use with std::unique_ptr. You do not want to pass the address of a 
			stack-allocated BoostGraph to BoostGraphDeleter.operator()(). BoostGraphDeleter.operator() calls operator 
			delete on the (BoostGraph *) argument, and passing the address of a stack-allocated BoostGraph for said 
			argument will result in undefined behavior.

			\see CreateBoostGraph for BoostGraphDeleter's intended use.
		*/
		struct BoostGraphDeleter {
		public:

			/// <summary> Delete a boost graph. </summary>
			/// <param name="bg"> Pointer to the boost graph to delete. </param>
			/*!
				\code
					// be sure to #include "path_finder.h", #include "boost_graph.h", #include
					// "node.h", #include "graph.h", and #include <vector>

					// For this example, we must have a BoostGraph instance to use with
					// BoostGraphDeleter. In order to create a BoostGraph, we must first create a
					// Graph instance first. We must prepare the nodes, their edges, and the weights
					// (distances) of each edge.

					// Create the nodes
					HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
					HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
					HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

					// Create a container (vector) of nodes
					std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

					// Create matrices for edges and distances, edges.size() == distances().size()
					std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
					std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);

					// Passing Graph graph to BoostGraph bg, by reference
					HF::Pathfinding::BoostGraph bg(graph);

					// Read below on BoostGraphDeleter -- this is important.

					// BoostGraphDeleter is needed by std::unique_ptr to destroy a BoostGraph; it is
					// not meant to be called directly. You do not want to pass the address of a
					// stack-allocated BoostGraph to BoostGraphDeleter::operator().

					// BoostGraphDeleter's operator() calls operator delete on the (BoostGraph *)
					// argument, and passing the address of a stack-allocated BoostGraph for said
					// argument will result in undefined behavior.

					// In other words, do __not__ do the following:

					HF::Pathfinding::BoostGraphDeleter bg_deleter;

					// Create a stack-allocated BoostGraph from a HF::SpatialStructures::Graph graph
					HF::Pathfinding::BoostGraph boostGraph(graph);	// not created using operator new

					// Using a BoostGraphDeleter on the (address of a) stack-allocated BoostGraph
					bg_deleter(&boostGraph);						// calls operator delete for a (BoostGraph *)
				 \endcode
			*/
			void operator()(BoostGraph * bg) const;
		};

		/*! 
			\brief Create a new boost graph from a HF::SpatialStructures:Graph 
		
			\param g The graph to create a BoostGraph from
			\param cost_type The name of the cost set in `graph` that will be used to construct this boost graph
							 Leaving this as blank will use the cost type the graph was constructed with.

			\returns A unique_ptr to point to the new BoostGraph created from the HFGraph.
			
			\pre 1) If `cost_type` is specified, and is not an empty string, then `cost_type` must be the key
			of an already created cost in `graph`.
			\pre 2) `graph` must be compressed.
		
			\remarks
			This returns a pointer since it insulates the caller from needing to import Boost, which
			is extremely useful for the C_Interface since its clients will not need to use boost at
			all and it doesn't need to go through the trouble of compiling all of the boost library
			(again). A unique pointer was chosen here so the caller doesn't need to worry about
			manually deleting the returned boost graph at a later point. 
			
			\throws HF::Exceptions::NoCost if `cost_type` was not left blank and no cost type with the key `cost_type`
					exists in `graph`.
			
			\code
				// be sure to #include "path_finder.h", #include "boost_graph.h", #include "node.h",
				// #include "graph.h", and #include <vector>

				// For this example, we must have a BoostGraph instance to use with
				// BoostGraphDeleter. In order to create a BoostGraph, we must first create a Graph
				// instance first. We must prepare the nodes, their edges, and the weights
				// (distances) of each edge.

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

				// Now we can create a smart pointer to a BoostGraph Note the type of boostGraph -
				// it is a std::unique_ptr<HF::Pathfinding::BoostGraph,
				// HF::Pathfinding::BoostGraphDeleter>. Use the auto keyword for type inference, or
				// your choice of using statements/typedef to make the use of the type described
				// above easier.
				auto boostGraph = CreateBoostGraph(graph);
			\endcode
		*/
		std::unique_ptr<BoostGraph,BoostGraphDeleter> CreateBoostGraph(
			const HF::SpatialStructures::Graph & g,
			const std::string & cost_type = ""
		);
		
		/// <summary> Find a path between points A and B using Dijkstra's Shortest Path algorithm. </summary>
		/// <param name="bg"> The boost graph containing edges/nodes. </param>
		/// <param name="start_id"> ID of the starting node. </param>
		/// <param name="end_id"> ID of the ending node. </param>
		/// <returns> The shortest path between A and B. </returns>
		/*!
			\details

			To find the path, A row of the predecessor matrix is generated for node a, then followed until
			node B is reached. This algorithm is implemented using dijkstra_shortest_path from the BoostGraphLibrary
			https://www.boost.org/doc/libs/1_73_0/libs/graph/doc/dijkstra_shortest_paths_no_color_map.html

			\note 
			Use FindPaths for multiple paths as it's able to reuse a lot of work compared to running this in a
			loop.

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
		
		/*! 
			\brief Find a path from every id in start_ids to the matching end node in end_ids. 

			\param bg The boost graph containing edges/nodes.
			\param start_points Ordered list of starting points.
			\param end_points Ordered list of ending points.
			
			\returns 
			An ordered array of paths matching the order of the pairs of start_id and end_id.
			Paths that could not be generated will be returned as paths with no nodes.

			\details
			More efficient than calling FindPath manually in a loop. Sorts paths by starting point,
			calculates only one predecessor matrix per unique starting point, then finds a
			path for every pair. 

			\pre Length of start_points must match that of end_points.
			
			\todo This isn't in parallel! Can be implemented using a similar approach to InsertPathsIntoArray. 
			
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

				// Prepare the parents and children vectors -- We will be searching for the shortest
				// path from node 0 to node 3, as well as the shortest path from node 0 to node 4.
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

		/*!
			\details
			Calculate a permutation for every combination of nodes in the graph,
			then call FindPaths, OR Write a unique function inspired by FindPaths
			that just runs through a range of paths. 

			\remarks
			May benefit from using an algorithm optimized for this like
			https://www.boost.org/doc/libs/1_73_0/libs/graph/doc/floyd_warshall_shortest.html.

			\exception std::out_of_memory if the memory required is too large.

			\warning This will allocate a large amount of memory. Ensure that this won't run out
			of memory when called, or be prepared to catch an out of memory exception.
			\todo Implement this function to the specifications outlined here. 
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
		std::vector<HF::SpatialStructures::Path> FindAllPaths(BoostGraph * bg);
		
		/*!
			\brief A special version of FindPaths optimized for the C_Interface.

			\param bg Boost graph to generate paths in
			\param start_points An ordered list of start points to generate paths from.
			\param end_points An ordered list of end points to generate paths to.
			\param out_paths Location for the the path pointer array to be created. Paths that could not be
			generated will be left as null pointers.
			\param out_path_members Location for the pathmember pointer array will be created. All path member
			pointers will point to the PathMembers of the Path in paths at the same location. Paths that could not
			be generated will be left as null pointers. 
			\param out_sizes Output raw_array of integers that will contain the length of every path in path_members.
			Paths that could not be generated will be left with a length of zero.
			
			\pre
			The length of start_ids must match the length of end_ids.

			\post 1) out_path_members will point to a vector of pointers to vectors of PathMembers with an element for every path.
			\post 2) out_paths will point to a vector of pointers to paths with an element for every path.
			\post 3) out_sizes will point to an array of integers containing the size of every path in out_paths

			\remarks 
			Usually the C-Interface is able to simply wrap existing functions with minimal code to make them accessible to exernal
			callers, however in this specific situation there were real performance gains to be found by implementing this function
			directly in HF::PathFinding itself. It's efficent and safe for that purpose, but \link FindPaths \endlink should be 
			preferred outside of that context since this function can be quite dangerous if not handled properly. 

			\warning
			The caller is responsible for freeing all of the memory allocated in out_paths and out_sizes. The contents of
			out_path_members will automatically be deleted when the path they belong to is deleted. Do not try
			to manually delete out_path_members or the path that owns it will throw a null pointer exception
			when it is deleted.

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

				// Prepare the parents and children vectors -- We will be searching for the shortest
				// path from node 0 to node 3, as well as the shortest path from node 0 to node 4.
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

		/*! \brief Holds and maintains a distance and predecessor matrix
		
			\remarks
			This is mostly used for the implementation of GenerateDistanceAndPredecessor. 

			\warning
			This object does NOT delete it's internal arrays upon destruction. It's the owner's
			responsibility to do this!
		*/
		struct DistanceAndPredecessor{
			std::vector<float>* dist;
			std::vector<int>* pred;

			int size = 0; ///< Number of rows and colums

			/*! 
				\brief Create and allocate a pair of distance and predecessor arrays. 
				
				\param size Number of rows and columns. Size of each array will be this to the power of 2

				All arrays will be initialized with NAN as their default values

			*/
			inline DistanceAndPredecessor(int size) {
				
				// Preallocate enough space in both arrays to hold n^2 values
				int arr_count = pow(size, 2);
				dist = new std::vector<float>(arr_count, -1);
				pred = new std::vector<int>(arr_count, -1);

				this->size = size;
			}

			/*! \brief Get a pointer to the beginning of the ith row of the distance array
				
				\param i Index of the row to get. 

				\returns a pointer to the beginning of row i.
			*/
			inline float * GetRowOfDist(int i) {
				// Check bounds in debug mode
				assert(i >= 0 && i < size);

				return dist->data() + (i * size);
			}


			/*! \brief Get a pointer to the beginning of the ith row of the predecessor array
				
				\param i Index of the row to get. 

				\returns a pointer to the beginning of row i.
			*/
			inline int * GetRowOfPred(int i) {
				// Check bounds in debug mode
				assert(i >= 0 && i < size);

				return pred->data() + (i * size);
			}
		};
			
		/*!
			\brief Generate the distance and predecessor matricies for a specific boost graph.

			\param bg Boost graph to generate the matricies from

			\returns A DistanceAndPredecessor containing pointers to the new distance and predecessor matricies 

			\warning 
			It is up to the caller to deallocate both arrays. This is mostly for the C_Interface, and as such ignores
			the safety that most other functions adhere to. It is the caller's responsibility to deallocate both arrays
			from this function. 

			\par Example
			\snippet tests\src\Pathfinding.cpp EX_DistPred
			\snippet tests\src\Pathfinding.cpp EX_DistPred_2
			`[0.000000, 10.000000, 5.000000, 10.000000, 0.000000, 15.000000, -1.000000, -1.000000, 0.000000]`\n
			`[0, 0, 0, 1, 1, 0, -1, -1, 2]`
		
		*/
		DistanceAndPredecessor GenerateDistanceAndPred(const BoostGraph& bg);
			
		inline DistanceAndPredecessor GenerateDistanceAndPredFast(const BoostGraph& bg);
		/*!
			\brief A special version of FindPaths optimized for the C_Interface, such that all paths possible
				   from each node to every other node are generated.

			\param bg Boost graph to generate paths in
			\param out_paths Location for the the path pointer array to be created. Paths that could not be
			generated will be left as null pointers.
			\param out_path_members Location for the pathmember pointer array will be created. All path member
			pointers will point to the PathMembers of the Path in paths at the same location. Paths that could not
			be generated will be left as null pointers.
			\param out_sizes Output raw_array of integers that will cntain the length of every path in path_members.
			Paths that could not be generated will be left with a length of zero.

			\pre
			The length of start_ids must match the length of end_ids.

			\post 1) out_path_members will point to a vector of pointers to vectors of PathMembers with an element for every path.
			\post 2) out_paths will point to a vector of pointers to paths with an element for every path.
			\post 3) out_sizes will point to an array of integers containing the size of every path in out_paths

			\remarks
			Usually the C-Interface is able to simply wrap existing functions with minimal code to make them accessible to exernal
			callers, however in this specific situation there were real performance gains to be found by implementing this function
			directly in HF::PathFinding itself. It's efficent and safe for that purpose, but \link FindPaths \endlink should be
			preferred outside of that context since this function can be quite dangerous if not handled properly.

			\warning
			The caller is responsible for freeing all of the memory allocated in out_paths and out_sizes. The contents of
			out_path_members will automatically be deleted when the path they belong to is deleted. Do not try
			to manually delete out_path_members or the path that owns it will throw a null pointer exception
			when it is deleted.


			\code
				HF::SpatialStructures::Graph g;

				// Add the edges
				g.addEdge(0, 1, 1);
				g.addEdge(0, 2, 2);
				g.addEdge(1, 3, 3);
				g.addEdge(1, 4, 4);
				g.addEdge(2, 4, 4);
				g.addEdge(3, 5, 5);
				g.addEdge(4, 6, 3);
				g.addEdge(5, 6, 1);

				// Always compress the graph after adding edges
				g.Compress();

				// Create a BoostGraph (std::unique_ptr)
				auto bg = CreateBoostGraph(g);

				// Total paths is node_count ^ 2
				size_t node_count = g.Nodes().size();
				size_t path_count = node_count * node_count;

				// Pointer to buffer of (Path *)
				Path** out_paths = new Path* [path_count];
				// out_paths[i...path_count - 1] will be alloc'ed by InsertPathsIntoArray

				// Pointer to buffer of (PathMember *)
				PathMember** out_path_member = new PathMember* [path_count];
				// out_path_member[i...path_count - 1] points to out_paths[i...path_count - 1]->GetPMPointer();

				// Pointer to buffer of (int)
				int* sizes = new int[path_count];

				//
				// The two loops for start_points and end_points
				// are just for the output.
				//
				int curr_id = 0;
				std::vector<int> start_points(path_count);
				// Populate the start points,
				// size will be (node_count)^2
				for (int i = 0; i < node_count; i++) {
					for (int k = 0; k < node_count; k++) {
						start_points[curr_id++] = i;
					}
				}

				curr_id = 0;

				std::vector<int> end_points(path_count);
				// Populate the end points,
				// size will be (node_count)^2
				for (int i = 0; i < node_count; i++) {
					for (int k = 0; k < node_count; k++) {
						end_points[curr_id++] = k;
					}
				}

				InsertAllToAllPathsIntoArray(bg.get(), out_paths, out_path_member, sizes);

				for (int i = 0; i < path_count; i++) {
					if (out_paths[i]) {
						// Always check if out_paths[i] is nonnull!
						int total_cost = 0;
						std::cout << "Path from " << start_points[i] << " to " << end_points[i] << std::endl;

						Path p = *out_paths[i];
						for (auto m : p.members) {
							total_cost += m.cost;
							std::cout << "node ID: " << m.node << "\tcost " << m.cost << std::endl;
						}

						std::cout << "Total cost: " << total_cost << std::endl;
						std::cout << "--------------------------" << std::endl;
					}
				}

				//
				// Resource cleanup
				//
				if (sizes) {
					delete[] sizes;
					sizes = nullptr;
				}

				if (out_path_member) {
					delete[] out_path_member;
					out_path_member = nullptr;
				}

				if (out_paths) {
					for (int i = 0; i < path_count; i++) {
						if (out_paths[i]) {
							delete out_paths[i];
							out_paths[i] = nullptr;
						}
					}
				}
			\endcode
		*/
		void InsertAllToAllPathsIntoArray(BoostGraph* bg, HF::SpatialStructures::Path** out_paths, HF::SpatialStructures::PathMember** out_path_members, int* out_sizes);

		std::vector<std::vector<int>> FindAPSP(BoostGraph& bg);

		//inline std::vector<int> ConstructShortestPathNodesFromPred(int start, int end, int* pred);
	}
}

/*! \brief An overload to print HF::Pathfinding::DistanceAndPredecessor when passed to cout .*/
inline std::ostream & operator<<(std::ostream& os, HF::Pathfinding::DistanceAndPredecessor & dist_pred) {
	const int num_nodes = dist_pred.size;

	// create strings to add to
	std::string dist_string = "[";
	std::string pred_string = "[";


	// Iterate through every row
	for (int row = 0; row < num_nodes; row++) {

		// Get pointers to beginning and end of both matricies
		const float* dist_row = dist_pred.GetRowOfDist(row);
		const int* pred_row = dist_pred.GetRowOfPred(row);

		// Iterate through every column in this row
		for (int col = 0; col < num_nodes; col++) {
			
			// Get values from matricies
			const float dist_value = dist_row[col];
			const int pred_value = pred_row[col];

			// Convert values to strings
			dist_string += std::to_string(dist_value);
			pred_string += std::to_string(pred_value);

			// Don't print a comma if this is the last element
			if (!(row == num_nodes-1 && col == num_nodes-1)) {
				dist_string += ", ";
				pred_string += ", ";
			}
		}
	}

	// cap off both strings
	dist_string += "]";
	pred_string += "]";

	// Print dist then predecessor
	os << dist_string << std::endl << pred_string;
	return os;
}
