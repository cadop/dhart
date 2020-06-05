#include <memory>
#include <vector>

namespace HF {
	namespace SpatialStructures {
		class Graph;
		class Path;
		class PathMember;
	}

	namespace Pathfinding {
		class BoostGraph; ///< Forward declared to prevent clients from importing boost ///<
		
		/// <summary>
		/// Just deletes the BoostGraph. This way clients don't need any of the boost stuff
		/// </summary>
		struct BoostGraphDeleter {
		public:
			void operator()(BoostGraph * bg) const;
		};

		/// <summary>
		/// Create a new boost graph from the given HFGraph
		/// <param name="g">The desired Graph used to create a BoostGraph</param>
		/// </summary>
		/// <returns>A smart pointer (unique_ptr) of type BoostGraph, created from the HFGraph
		std::unique_ptr<BoostGraph,BoostGraphDeleter> CreateBoostGraph(const HF::SpatialStructures::Graph & g);
		
		/// <summary>
		/// Find a path between points A and B
		/// </summary>
		/// <param name="bg"> The boost graph containing edges/nodes </param>
		/// <param name="start_id"> ID of the starting node </param>
		/// <param name="end_id"> ID of the ending node </param>
		/// <returns>A Path constructed using Dijkstra's Shortest Path algorithm</returns>
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
		/// <returns>/A std::vector of Path</returns>
		std::vector<HF::SpatialStructures::Path> FindAllPaths(BoostGraph * bg, int start_id, int end_id);
		
		/// <summary>
		/// TODO summary
		/// </summary>
		/// <param name="bg">TODO</param>
		/// <param name="start_points">TODO</param>
		/// <param name="end_points">TODO</param>
		/// <param name="out_paths">TODO</param>
		/// <param name="out_path_members">TODO</param>
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
