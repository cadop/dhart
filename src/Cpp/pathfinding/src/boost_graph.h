
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace std {
	template <typename SizeT>
	inline void array_hash_combine_impl(SizeT& seed, SizeT value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <>
	struct hash<std::array<float, 3>>
	{
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
}// 
namespace HF {
	namespace SpatialStructures {
		class Graph;
		class Node;
	}

	namespace Pathfinding {

		using pair = std::pair<int, int>;
		struct Edge_Cost { float weight; };

		struct vertex_data {
			boost::graph_traits<
				boost::compressed_sparse_row_graph<boost::directedS>
			>::vertex_descriptor p;
			double d;
		};
		typedef boost::compressed_sparse_row_graph<
			boost::directedS,
			vertex_data,
			Edge_Cost
		> graph_t;
		typedef boost::graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
		typedef std::pair <int, int> pair;

		/// <summary>
		/// Holds the necessary state for the boost pathfinding algorithm
		/// </summary>
		class BoostGraph {
		public:
			graph_t g;
			std::vector<vertex_descriptor> p;
			std::vector<double> d;

			/// <summary>
			/// Create a new boost graph from the given HF Graph
			/// </summary>
			BoostGraph(const HF::SpatialStructures::Graph & graph);
			~BoostGraph();
		};
	}
}
