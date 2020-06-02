#include <memory>
#include <vector>

namespace HF {
	namespace SpatialStructures {
		class Graph;
		class Path;
		class PathMember;
	}
	namespace Pathfinding {
		class BoostGraph; // Forward declared to prevent clients from importing boost
		
		/// <summary>
		/// Just deletes the BoostGraph. This way clients don't need any of the boost stuff
		/// </summary>
		struct BoostGraphDeleter {
		public:
			void operator()(BoostGraph * bg) const;
		};

		/// <summary>
		/// Create a new boost graph from the given HFGraph
		/// </summary>
		std::unique_ptr<BoostGraph,BoostGraphDeleter> CreateBoostGraph(const HF::SpatialStructures::Graph & g);
		
		/// <summary>
		/// Find a path between points A and B
		/// </summary>
		/// <param name="bg"> The boost graph containing edges/nodes </param>
		/// <param name="start_id"> ID of the starting node </param>
		/// <param name="end_id"> ID of the ending node </param>
		HF::SpatialStructures::Path FindPath(BoostGraph * bg, int start_id, int end_id);
		
		/// <summary>
		/// More efficient than calling FindPath manually. Organises paths by starting point,
		/// calculates only one predecessor matrix per unique starting point, then finds a 
		/// path for every pair.
		/// </summary>
		/// <param name="graph"> The boost graph containing edges/nodes </param>
		/// <param name=start_points> Ordered list of starting points </param>
		/// <param name="end_points"> Ordered list of ending points </param>
		std::vector<HF::SpatialStructures::Path> FindPaths(
			BoostGraph * bg,
			const std::vector<int> & start_points,
			const std::vector<int> & end_points
		);

		/// <summary>
		/// Find a path from every node to every node
		/// </summary>
		std::vector<HF::SpatialStructures::Path> FindAllPaths(BoostGraph * bg, int start_id, int end_id);
		
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


