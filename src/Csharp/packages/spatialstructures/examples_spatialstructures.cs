using HumanFactors.GraphGenerator;
using HumanFactors.SpatialStructures;
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using HumanFactors.Pathfinding;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors;
using System.Diagnostics;
using System;

namespace Humanfctors.Examples
{
	[TestClass]
	public class SpatialExamples
	{
		public Graph GenerateExampleGraph()
		{
			//! [EX_PathFinding_Graph]
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
			
			//! [EX_PathFinding_Graph]

			return graph;
		}

		[TestMethod]
		public void GeneratePathsWithCostAlgorithms()
		{
			var graph = GenerateExampleGraph();
			
			//! [EX_Pathfinding_Setup]

			// Predeclare paths
			Path energy_path, distance_path;

			// Get the unique key of this cost type. We'll use this to tell the pathfinder to generate
			// a path using the cost set that we just generated.
			string energy_key = CostAlgorithmNames.ENERGY_EXPENDITURE;

			//! [EX_Pathfinding_Setup]
			
			//! [EX_Pathfinding_IDS]

			// Generate a graph using the alternate cost type, then generate one using the graph's
			// default cost type, distance.
			int start_id = 1;
			int end_id = 105;
			energy_path = ShortestPath.DijkstraShortestPath(graph, start_id, end_id, energy_key);
			distance_path = ShortestPath.DijkstraShortestPath(graph, start_id, end_id);

			//! [EX_Pathfinding_IDS]

			//! [EX_Pathfinding_Nodes]
			
			// Get the nodes from the graph
			var nodes = graph.getNodes();

			// Convert these to Vector3D for use in pathfinding
			var start_node = nodes[1].ToVector3D();
			var end_node = nodes[105].ToVector3D();

			// Generate paths using these nodes.
			energy_path = ShortestPath.DijkstraShortestPath(graph, start_node, end_node, energy_key);
			distance_path = ShortestPath.DijkstraShortestPath(graph, end_node, start_node);

			//! [EX_Pathfinding_Nodes]

			//! [EX_Pathfinding_Print]
			
			// Print paths to output. 
			Debug.WriteLine(distance_path);
			Debug.WriteLine(energy_path);
			
			//! [EX_Pathfinding_Print]
		}

		[TestMethod]
		public void DijkstraShortestPathMulti()
		{
			var graph = GenerateExampleGraph();

			//! [EX_MultiPathFinding]

			// Get the unique key of this cost type. We'll use this to tell the pathfinder to generate
			// a path using the cost set that we just generated.
			string energy_key = CostAlgorithmNames.ENERGY_EXPENDITURE;

			// Generate a graph using the alternate cost type, then generate one using the graph's
			// default cost type, distance.
			int[] start_ids = {1, 2, 3, 4};
			int[] end_ids = {101, 102, 103, 104};
			Path[] energy_path = ShortestPath.DijkstraShortestPathMulti(graph, start_ids, end_ids, energy_key);
			Path[] distance_path = ShortestPath.DijkstraShortestPathMulti(graph, start_ids, end_ids);

			// Print out every pair of paths
			for (int i = 0; i < start_ids.Length; i++)
			{
				Debug.WriteLine(
					String.Format("{0} to {1} Energy  : {2}", start_ids[i], end_ids[i], energy_path[i])
				);
				Debug.WriteLine(
					String.Format("{0} to {1} Distance: {2}", start_ids[i], end_ids[i], distance_path[i])
				);
			}

			//! [EX_MultiPathFinding]

		}

		[TestMethod]
		public void DijkstraShortestPathMultiNodes()
		{
			var graph = GenerateExampleGraph();

			//! [EX_MultiPathFinding_Nodes]

			// Get the unique key of this cost type. We'll use this to tell the pathfinder to generate
			// a path using the cost set that we just generated.
			string energy_key = CostAlgorithmNames.ENERGY_EXPENDITURE;

			// Get nodes from the graph
			var nodes = graph.getNodes();

			// Generate a graph using the alternate cost type, then generate one using the graph's
			// default cost type, distance.
			Vector3D[] start_nodes = {
				nodes[0].ToVector3D(),
				nodes[1].ToVector3D(),
				nodes[2].ToVector3D(),
				nodes[3].ToVector3D()
			};
			Vector3D[] end_nodes = {
				nodes[101].ToVector3D(),
				nodes[102].ToVector3D(),
				nodes[103].ToVector3D(),
				nodes[104].ToVector3D()
			};
			Path[] energy_path = ShortestPath.DijkstraShortestPathMulti(graph, start_nodes, end_nodes, energy_key);
			Path[] distance_path = ShortestPath.DijkstraShortestPathMulti(graph, start_nodes, end_nodes);

			// Print out every pair of paths
			for (int i = 0; i < start_nodes.Length; i++)
			{
				Debug.WriteLine(
					String.Format("{0} to {1} Energy  : {2}", start_nodes[i], end_nodes[i], energy_path[i])
				);
				Debug.WriteLine(
					String.Format("{0} to {1} Distance: {2}", start_nodes[i], end_nodes[i], distance_path[i])
				);
			}

			//! [EX_MultiPathFinding_Nodes]

		}

		[TestMethod]
		public void GenerateAllToAll()
		{
			//! [EX_Pathfinding_AllToAll]
			
			// Create a graph and add some edges
			Graph g = new Graph();

			Vector3D[] Nodes =
			{
				new Vector3D(0,0,1),
				new Vector3D(0,0,2),
				new Vector3D(0,0,3),
				new Vector3D(0,0,4)
			}; 

			g.AddEdge(Nodes[0], Nodes[1], 10);
			g.AddEdge(Nodes[0], Nodes[2], 30);
			g.AddEdge(Nodes[0], Nodes[3], 30);
			g.AddEdge(Nodes[1], Nodes[2], 15);
			g.AddEdge(Nodes[3], Nodes[1], 15);
			g.AddEdge(Nodes[2], Nodes[3], 5);

			// Compress the graph
			g.CompressToCSR();
			
			// Generate all paths for this graph
			var paths = ShortestPath.DijkstraAllToAll(g);

			// Iterate through the return values to print the path from every node to every other node
			int num_nodes = g.NumNodes();
			for (int start_id = 0; start_id < num_nodes; start_id++)
			{
				for (int end_id = 0; end_id < num_nodes; end_id++)
				{
					// If this is a node to itself then continue
					if (start_id == end_id) continue;

					// get the path from parent id to child id
					int path_index = (start_id * num_nodes) + end_id;
					Path start_to_end = paths[path_index];

					// Only print the path if it was able to be found
					if (start_to_end != null)
						Debug.WriteLine(String.Format("{0} -> {1} : {2}", start_id, end_id, start_to_end));
					else
						Debug.WriteLine(String.Format("{0} -> {1} : {2}", start_id, end_id, "[None]"));
				}
			}
			
			//! [EX_Pathfinding_AllToAll]
		}

		[TestMethod]
		public void GetEdgeCostExample()
		{
			//! [Example_GetCost]

			// Create a graph and add an edge
			Graph g = new Graph();
			g.AddEdge(0, 1, 100);

			// Compress the graph so we can read the edge
			g.CompressToCSR();

			// Retrieve the cost that we just added using GetCost
			float cost_in_graph = g.GetCost(0, 1);
			Debug.WriteLine(cost_in_graph);

			//! [Example_GetCost]

			Assert.AreEqual(100, cost_in_graph);
		}


	}
}