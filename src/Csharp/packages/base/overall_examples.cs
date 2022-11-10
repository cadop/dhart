using DHARTAPI.GraphGenerator;
using DHARTAPI.SpatialStructures;
using DHARTAPI.Geometry;
using DHARTAPI.RayTracing;
using DHARTAPI.Pathfinding;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using DHARTAPI;
using System.Diagnostics;
using System;
using System.Text.RegularExpressions;
using System.Collections;
using System.Collections.Generic;

namespace Humanfctors.Examples
{
	[TestClass]
	public class OverallExamples
	{
		/*! \brief Compares two strings ignoring whitespace. */
		public void CompareOutputToExpected(string expected, string actual)
		{
			string normalized_expected = Regex.Replace(expected, @"\s", "");
			string normalized_actual = Regex.Replace(actual, @"\s", "");

			Assert.AreEqual(normalized_expected, normalized_actual);
		}

		public Graph GenerateExampleGraph()
		{
			//! [EX_PathFinding_Graph]
			// Load the OBJ from ddisk
			var blob = OBJLoader.LoadOBJ("ExampleModels/energy_blob_zup.obj");

			// Generate a BVH from it 
			EmbreeBVH bvh = new EmbreeBVH(blob, true);

			// Setup graph variables
			Vector3D start_point = new Vector3D(-30, 0, 20);
			Vector3D spacing = new Vector3D(1, 1, 10);
			int max_nodes = 10000;
			float up_step = 5.0f;
			float down_step = 5.0f;
			float up_slope = 60.0f;
			float down_slope = 60.0f;
			int max_step_connections = 1;
			int min_connections = 1;

			// Generate a graph
			var graph = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(
				bvh,
				start_point,
				spacing,
				max_nodes,
				up_step,
				down_step,
				up_slope,
				down_slope,
				max_step_connections,
				min_connections,
				-1
			);

			// Compress the graph so we can use it with pathfinding
			graph.CompressToCSR();

			// Calculate an alternate set of costs for the graph based on Energy Expenditure
			CostAlgorithms.CalculateAndStoreEnergyExpenditure(graph);
			
			//! [EX_PathFinding_Graph]

			return graph;
		}

		public void AssertEquality(PathMember[] Expected, PathMember[] Actual)
		{
			for (int i = 0; i < Expected.Length; i++)
			{
				Assert.AreEqual(Expected[i].id, Actual[i].id);
				Assert.AreEqual(Expected[i].cost_to_next, Actual[i].cost_to_next,0.001);
			}
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

			PathMember[] expected_distance = { new PathMember(105, 1.415f), new PathMember(84, 1.0f),
				new PathMember(66, 1.0f), new PathMember(52, 1.0f), new PathMember(41, 1.0f), new PathMember(28, 1.0f),
				new PathMember(15, 1.0f), new PathMember(3, 1.0f), new PathMember(2, 1.0f), new PathMember(1, 0f)};

			PathMember[] expected_energy ={new PathMember(1, 2.51f), new PathMember(2, 2.486f),
				new PathMember(3, 2.488f), new PathMember(15, 2.5f), new PathMember(28, 2.5f),
				new PathMember(41, 2.484f), new PathMember(52, 2.421f), new PathMember(66, 2.43f),
				new PathMember(84, 2.741f), new PathMember(105, 0)
			};
			
			AssertEquality(expected_distance, distance_path.CopyArray());
			AssertEquality(expected_energy, energy_path.CopyArray());
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

			string output = "";

			// Print out every pair of paths
			for (int i = 0; i < start_ids.Length; i++)
			{
				output += String.Format("{0} to {1} Energy  : {2}", start_ids[i], end_ids[i], energy_path[i]) + "\n";
				output += String.Format("{0} to {1} Distance: {2}", start_ids[i], end_ids[i], distance_path[i]) + "\n";
			}

			Debug.WriteLine(output);

			//! [EX_MultiPathFinding]

			string expected_output = @"1 to 101 Energy  : [(1, 4.571), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 1.937), (80, 5.065), (101, 0)]
1 to 101 Distance: [(1, 1.415), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1), (80, 1.416), (101, 0)]
2 to 102 Energy  : [(2, 2.49), (1, 4.571), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 3.388), (81, 3.271), (102, 0)]
2 to 102 Distance: [(2, 1), (1, 1.415), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1.001), (81, 1.001), (102, 0)]
3 to 103 Energy  : [(3, 2.514), (2, 2.49), (1, 4.571), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 5.858), (82, 3.837), (103, 0)]
3 to 103 Distance: [(3, 1), (2, 1), (1, 1.415), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1.418), (82, 1.002), (103, 0)]
4 to 104 Energy  : [(4, 2.475), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 5.858), (82, 6.109), (104, 0)]
4 to 104 Distance: [(4, 1), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1.418), (82, 1.419), (104, 0)]";

			CompareOutputToExpected(expected_output, output);

		}

		[TestMethod]
		public void DijkstraShortestPathMultiNodes()
		{
			var graph = GenerateExampleGraph();

			String output = "";

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
				output += String.Format("{0} to {1} Energy  : {2}", start_nodes[i], end_nodes[i], energy_path[i]) + "\n";
				output +=
					String.Format("{0} to {1} Distance: {2}", start_nodes[i], end_nodes[i], distance_path[i]) + "\n";
			}

			Debug.WriteLine(output);
			//! [EX_MultiPathFinding_Nodes]

			// Compare this output to the expected output

			string expected_output = @"(-30, 0, 1.069) to (-25, -8, 1.556) Energy  : [(0, 2.484), (4, 2.475), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 1.937), (80, 5.065), (101, 0)]
(-30, 0, 1.069) to (-25, -8, 1.556) Distance: [(0, 1), (4, 1), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1), (80, 1.416), (101, 0)]
(-31, -1, 1.018) to (-26, -4, 1.596) Energy  : [(1, 4.571), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 3.388), (81, 3.271), (102, 0)]
(-31, -1, 1.018) to (-26, -4, 1.596) Distance: [(1, 1.415), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1.001), (81, 1.001), (102, 0)]
(-31, 0, 1.019) to (-25, -6, 1.679) Energy  : [(2, 2.49), (1, 4.571), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 5.858), (82, 3.837), (103, 0)]
(-31, 0, 1.019) to (-25, -6, 1.679) Distance: [(2, 1), (1, 1.415), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1.418), (82, 1.002), (103, 0)]
(-31, 1, 1.018) to (-24, -8, 1.73) Energy  : [(3, 2.514), (2, 2.49), (1, 4.571), (12, 5.759), (26, 5.871), (39, 6.117), (50, 7.005), (64, 5.858), (82, 6.109), (104, 0)]
(-31, 1, 1.018) to (-24, -8, 1.73) Distance: [(3, 1), (2, 1), (1, 1.415), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1.421), (64, 1.418), (82, 1.419), (104, 0)]";
			
			CompareOutputToExpected(expected_output, output);
		}

		[TestMethod]
		public void GenerateAllToAll()
		{
			//! [EX_Pathfinding_AllToAll]
			
			// Create a graph and add some edges
			Graph g = new Graph();

			// Create nodes and add edges to the graph
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

			string output = "";

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
						output += String.Format("{0} -> {1} : {2}", start_id, end_id, start_to_end) + "\n";
					else
						output += String.Format("{0} -> {1} : {2}", start_id, end_id, "[None]") + "\n";
				}
			}

			// Print output
			Debug.WriteLine(output);

			//! [EX_Pathfinding_AllToAll]

			string expected_output = @"0 -> 1 : [(0, 10), (1, 0)]
			0 -> 2 : [(0, 10), (1, 15), (2, 0)]
			0 -> 3 : [(0, 30), (3, 0)]
			1 -> 0 : [None]
			1 -> 2 : [(1, 15), (2, 0)]
			1 -> 3 : [(1, 15), (2, 5), (3, 0)]
			2 -> 0 : [None]
			2 -> 1 : [(2, 5), (3, 15), (1, 0)]
			2 -> 3 : [(2, 5), (3, 0)]
			3 -> 0 : [None]
			3 -> 1 : [(3, 15), (1, 0)]
			3 -> 2 : [(3, 15), (1, 15), (2, 0)]";

			// Compare output to expected
			CompareOutputToExpected(expected_output, output);
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

		[TestMethod]
		public void EnergyBlobExample()
		{
			MeshInfo MI = OBJLoader.LoadOBJ("ExampleModels/energy_blob_zup.obj");
			EmbreeBVH bvh = new EmbreeBVH(MI, true);
			int max_nodes = 5000;
			float up_step = 5;
			float up_slope = 60;
			float down_step = 5;
			float down_slope = 60;
			int max_step_connections = 1;
			int min_connections = 1;
			int cores = -1;


			Graph g = GraphGenerator.GenerateGraph(
				bvh,
				new Vector3D(-30.0f, 0.0f, 20.0f),
				new Vector3D (1.0f, 1.0f, 10.0f),
				max_nodes,
				up_step,
				up_slope,
				down_step,
				down_slope,
				max_step_connections,
				min_connections,
				cores
			);

			g.CompressToCSR();
			Assert.AreEqual(3450, g.NumNodes());
			Debug.WriteLine(g);

		}
	}
}