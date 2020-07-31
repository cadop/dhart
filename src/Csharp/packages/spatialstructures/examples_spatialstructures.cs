using HumanFactors.GraphGenerator;
using HumanFactors.SpatialStructures;
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using HumanFactors.Pathfinding;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors;
using System.Diagnostics;

namespace Humanfctors.Examples
{
	[TestClass]
	public class SpatialExamples
	{

		[TestMethod]
		public void GeneratePathsWithCostAlgorithms()
		{
			// Load the OBJ from ddisk
			var blob = OBJLoader.LoadOBJ("ExampleModels/energy_blob_zup.obj");

			// Generate a BVH from it 
			EmbreeBVH bvh = new EmbreeBVH(blob);

			// Setup graph variables
			Vector3D start_point = new Vector3D(-30, 0, 20);
			Vector3D spacing = new Vector3D(1, 1, 10);
			int max_nodes = 10000;
			float up_step = 5.0f;
			float down_step = 5.0f;
			float up_slope = 60.0f;
			float down_slope = 60.0f;
			int max_step_connections = 1;

			// Generate a graph
			var graph = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(
				bvh,
				start_point,
				spacing,
				max_nodes,
				up_step,
				down_step,
				up_slope,
				down_slope,
				max_step_connections,
				-1
			);

			// Compress the graph so we can use it with pathfinding
			graph.CompressToCSR();

			// Calculate an alternate set of costs for the graph based on Energy Expenditure
			CostAlgorithms.CalculateAndStoreEnergyExpenditure(graph);

			// Get the unique key of this cost type. We'll use this to tell the pathfinder to generate
			// a path using the cost set that we just generated.
			string energy_key = CostAlgorithmNames.ENERGY_EXPENDITURE;

			// Generate a graph using the alternate cost type, then generate one using the graph's
			// defgault cost type, distance. 
			int start_id = 1;
			int end_id = 105;
			var energy_path = ShortestPath.DijkstraShortestPath(graph, start_id, end_id, energy_key);
			var distance_path = ShortestPath.DijkstraShortestPath(graph, start_id, end_id);

			// Print paths to output. 
			Debug.WriteLine(distance_path);
			Debug.WriteLine(energy_path);
		}

	}
}