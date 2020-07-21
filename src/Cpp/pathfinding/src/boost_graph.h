///
///	\file		boost_graph.h
/// \brief		Contains definitions for the <see cref="HF::Pathfinding::BoostGraph">BoostGraph</see> class
///
///	\author		TBA
///	\date		17 Jun 2020
///

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/adjacency_list.hpp>


// Define a hash function for arrays of 3 floats.
namespace std {
	/// <summary> Combine hashes for multiple floats. </summary>
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
		/// <summary> Template specialization to hash an array of floats. </summary>
		/// <param name="k"> Reference to an array of coordinates (x, y, z) </param>
		/// <returns> Hash for x,y, and z. </returns>
		/*!
			\code{.cpp}
				// Create coordinates, (x, y, z)
				std::array<float, 3> arr = { 123.0, 456.1, 789.2 };
		
				// Using the template specialization for std::hash (which takes a std::array<float,
				// 3>), we retrieve a seed
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

/*!  
	\brief The Boost C++ library. https://www.boost.org/

	\details
	HumanFactors uses a subset of the boost library containing only the essentials for 
	creating graphs and running shortest path algorithms. Use of boost is 
	exclusive to the HF::Pathfinding namespace. 

	\see BoostGraph For the graph implementation used.
	\see path_finder.h for the boost functionality used. 
	
*/
namespace boost
{
#ifdef BOOST_NO_EXCEPTIONS
	inline void throw_exception(std::exception const& e) {
		throw 11; // or whatever
	};
#endif
}


namespace HF {

	// Forward Declares
	namespace SpatialStructures {
		class Graph;
		class Node;		
	}

	namespace Pathfinding {
		// Note: These structs and typedefs are used to simplify creation and usage of the boost
		// graph and serve no purpose outside of that.

		/// <summary> Data stored for every edge in the BoostGraph. </summary>
		/*! \details Each edge in the graph only stores its weight as a float. */
		struct Edge_Cost { 
			float weight; ///< Cost of traversing this edge.
		};

		/// <summary> Data stored for every vertex in the BoostGraph.</summary>
		/*!
			\details 
			Every vertex stores in the graph stores it's index p and a unique double d.
		
			\todo Is the double d needed?
		
		*/
		struct vertex_data {
			boost::graph_traits<
				boost::compressed_sparse_row_graph<boost::directedS>
			>::vertex_descriptor p;			///< The index of a vertex in the CSR.
			double d;						///< Unknown may have been used for the colormap. 
		};

		/*! 
			\brief Type of graph held by the BoostGraph.

			\details
			Set the graph as directed, store vertex_data for each vertex store Edge_Cost for each edge. 

			\remarks
			By changing the contents of this typedef, you can change the type of graph that the BoostGraph holds,
			and the the type of data stored for each vertex or edge.

			\see vertex_data for info on what every vertex in the graph holds.
			\see Edge_Cost for info on what every edge in the graph holds. 
		*/
		typedef boost::compressed_sparse_row_graph<
			boost::directedS,
			vertex_data,
			Edge_Cost
		> graph_t;

		/// <summary> Quick alias to shorten the typename of vertex descriptors for our graph_t
		/// type. /summary>
		typedef boost::graph_traits< graph_t >::vertex_descriptor vertex_descriptor;

		/// <summary> Shorten std::pair to simplify graph construction. </summary>
		typedef std::pair <int, int> pair;

		/*!
			\brief A graph usable with the BoostGraphLibrary.

			\details
			Contains a CSR in boost created from a graph in HumanFactors. This is necessary
			for using any of the BoostGraphLibrary functions.

			\remarks
			None of the memebers of this class should need to be modified after the class is created.
			This is  intended to be a pseudo-wrapper to make the HF::SpatialStructures::Graph usable with boost. 
			You can read more about the boost graph libary in the official boost documentation:
			https://www.boost.org/doc/libs/1_73_0/libs/graph/doc/quick_tour.html

			\remarks
			 Boost's graphs can be difficult to use due to the extensive use of templates. This limits
			 their usage outside of the HF::Pathfinder, so there aren't any functions to interact with the boost graph
			 after creation.

			 \see HF::SpatialStructures::Graph for a graph that's better supported in HumanFactors.
			 \see graph_t for info about the graph in boost this class holds. 
		*/
		class BoostGraph {
		public:
			graph_t g;							///< The underlying graph in boost.
			std::vector<vertex_descriptor> p;	///< Vertex array preallocated to the number of nodes in the graph.
			std::vector<double> d;				///< Distance array preallocated to the number of nodes in the graph.

			/// <summary> Create a boost graph from a HF::SpatialStructures::Graph. </summary>
			/*!
				\param graph Graph to create a graph in boost from. 

				\details 
				Every edge and node in the graph is used to create a graph in Boost. This will also allocate space
				equal to the number of nodes in g for this class's p and d arrays for use in FindPath as an 
				optimization.

				\invariant
				This class will always carry a valid boost graph. 

				\code{.cpp}
					// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h",
					// and #include <vector>

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

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);

					// Passing Graph graph to BoostGraph bg, by reference
					HF::Pathfinding::BoostGraph bg(graph);
				\endcode
			*/
			BoostGraph(const HF::SpatialStructures::Graph& graph);

			/*!
				\brief TODO brief
				\param g TODO g
				\param cost_name TODO cost_name

				\throw HF::Exceptions::NoCost if cost_name does not exist as a cost type in *this
				\note HF::Exceptions::NoCost not implemented yet.

				\code
					// TODO example
				\endcode
			*/
			BoostGraph(const HF::SpatialStructures::Graph& g, const std::string cost_name);

			/// <summary> Explicit Destructor required for BoostGraphDeleter to work in path_finder.h. </summary>
			/*!

				\details Calls the default destruction behavior. 

				\warning 
				Do not modify this function! This class is still using the default destructor, but unique pointer
				requires the destructor to be declared. 


				\code{.cpp}
					// be sure to #include "boost_graph.h", #include "node.h", #include "graph.h",
					// and #include <vector>

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

					// Now you can create a Graph - note that nodes, edges, and distances are passed
					// by reference
					HF::SpatialStructures::Graph graph(edges, distances, nodes);

					// Begin scope
					{
						// Create a BoostGraph bg, from a Graph graph (passing g by reference)
						HF::Pathfinding::BoostGraph bg(graph);
					}
					// End scope

					// When bg goes out of scope, BoostGraph::~BoostGraph is called An explicit call
					// to BoostGraph::~BoostGraph is also made when invoking operator delete on a
					// (BoostGraph *)
				\endcode
			*/
			~BoostGraph();
		};
	}
}
