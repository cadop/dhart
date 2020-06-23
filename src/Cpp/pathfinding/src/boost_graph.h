///
///	\file		boost_graph.h
///	\brief		Header file for BoostGraph
///
///	\author		TBA
///	\date		17 Jun 2020
///
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace std {
	/// <summary>
	/// Used to create a seed
	/// </summary>
	/// @tparam SizeT Integral type, such as size_t (implementation defined)

	/*!
		\code{.cpp}
			std::size_t seed_in = 0;
			std::size_t value_in = VALUE_USER_DEFINED;	// user-defined value for seed
	
			// seed_in is passed by reference
			std::array_hash_combine_impl<size_t>(seed_in, value_in);
	
			// seed_in has now been given a value by array_hash_combine_impl
		\endcode
	*/
	template <typename SizeT>
	inline void array_hash_combine_impl(SizeT& seed, SizeT value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <>
	struct hash<std::array<float, 3>>
	{
		/// <summary>
		/// Returns a seed
		/// </summary>
		/// <param name="k">Reference to an array of coordinates (x, y, z)</param>
		/// <returns>A seed, using std::array_has_combine_impl</returns>

		/*!
			\code{.cpp}
				// Create coordinates, (x, y, z)
				std::array<float, 3> arr = { 123.0, 456.1, 789.2 };
		
				// Using the template specialization for std::hash (which takes a std::array<float, 3>),
				// we retrieve a seed
				std::size_t arr_hash = std::hash<std::array<float, 3>>(arr);
			\endcode
		*/
		inline std::size_t operator()(const std::array<float, 3>& k) const
		{
			size_t seed = std::hash<float>()(k[0]);
			array_hash_combine_impl(seed, std::hash<float>()(k[1]));
			array_hash_combine_impl(seed, std::hash<float>()(k[2]));
			return seed;
		}
	};
}

namespace boost
{
#ifdef BOOST_NO_EXCEPTIONS
	inline void throw_exception(std::exception const& e) {
		throw 11; // or whatever
	};
#endif
}

namespace HF {
	namespace SpatialStructures {
		class Graph;	///< see graph.h in spatialstructures
		class Node;		///< see node.h in spatialstructures
	}

	namespace Pathfinding {
		using pair = std::pair<int, int>;

		/// <summary>
		/// Represents the weight of an edge within a graph, used by graph_t
		/// </summary>
		struct Edge_Cost { 
			float weight;	///< weight/cost of an edge
		};

		/// <summary>
		/// Represents a vertex descriptor, used by graph_t
		/// </summary>
		struct vertex_data {
			boost::graph_traits<
				boost::compressed_sparse_row_graph<boost::directedS>
			>::vertex_descriptor p;			///< A vertex_descriptor for a directed Boost CSR graph
			double d;
		};

		/// <summary>
		/// Alias for a graph type, a directed Compressed Sparse Row (CSR) graph
		/// See struct vertex_data and struct Edge_Cost
		/// </summary>
		typedef boost::compressed_sparse_row_graph<
			boost::directedS,
			vertex_data,
			Edge_Cost
		> graph_t;

		/// <summary>
		/// Alias for a vertex_descriptor
		/// See graph_t
		/// </summary>
		typedef boost::graph_traits< graph_t >::vertex_descriptor vertex_descriptor;

		/// <summary>
		/// Alias for std::pair<int, int>
		/// </summary>
		typedef std::pair <int, int> pair;

		/// <summary>
		/// Holds the necessary state for the Boost pathfinding algorithm
		/// </summary>
		class BoostGraph {
		public:
			graph_t g;							///< Directed CSR graph (Boost)
			std::vector<vertex_descriptor> p;	///< Container of vertex_descriptor
			std::vector<double> d;				///< Container of double

			/// <summary>
			/// Create a new Boost graph from the given HF Graph
			/// </summary>
			
			/*!
				\code{.cpp}
					// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

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
				\endcode
			*/
			BoostGraph(const HF::SpatialStructures::Graph& graph);

			/// <summary>
			/// Destroy the current Boost graph (uses compiler-generated implementation via 'default')
			/// </summary>
			
			/*!
				\code{.cpp}
					// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h", and #include <vector>

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

					// Begin scope
					{
						// Create a BoostGraph bg, from a Graph graph (passing g by reference)
						HF::Pathfinding::BoostGraph bg(graph);
					}
					// End scope

					// When bg goes out of scope, BoostGraph::~BoostGraph is called
					// An explicit call to BoostGraph::~BoostGraph is also made when
					// invoking operator delete on a (BoostGraph *)
				\endcode
			*/
			~BoostGraph();
		};
	}
}
