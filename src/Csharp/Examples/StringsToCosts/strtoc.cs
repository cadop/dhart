using HumanFactors.Geometry;
using HumanFactors.GraphGenerator;
using HumanFactors.SpatialStructures;
using HumanFactors;
using System;
using System.Diagnostics;
using HumanFactors.RayTracing;
using HumanFactors.ViewAnalysis;
using HumanFactors.Pathfinding;
using System.Linq;

class StringToEdgeCost
{
	static public void Main(String[] args)
	{
		// Load the OBJ from disk
		var blob = OBJLoader.LoadOBJ("ExampleModels/energy_blob_zup.obj");

		// Generate a BVH from it to use with the raytracer
		EmbreeBVH bvh = new EmbreeBVH(blob, true);

		// Setup graph variables
		Vector3D start_point = new Vector3D(-30, 0, 20); 
		Vector3D spacing = new Vector3D(1, 1, 10);
		int max_nodes = 1000;
		float up_step = 5.0f; float down_step = 5.0f;
		float up_slope = 60.0f; float down_slope = 60.0f;
		int max_step_connections = 1;

		// Generate a graph
		var graph = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(
			bvh,
			start_point,
			spacing,
			max_nodes,
			up_step, down_step,
			up_slope, down_slope,
			max_step_connections
		);

		// Compress the graph so we can use it with pathfinding
		graph.CompressToCSR();

		// Get the nodes in the graph
		var nodes = graph.getNodes();
		int num_nodes = nodes.size;
		Debug.WriteLine(nodes);
		/// >>>[(-30, 0, 1.0688), (-31, -1, 1.0182), (-31, 0, 1.0187), (-31, 1, 1.018), (-30, -1, 1.068), . . . 
		///		(-8, 9, 5.5221), (-8, 10, 5.0938), (-8, 12, 4.2812), (-8, 14, 3.519)]

		Debug.WriteLine("Number of nodes is " + num_nodes);
		// >>> Number of nodes is 1063

		// Conduct view analysis and aggregate the results
		int num_rays = 1000;
		var results = ViewAnalysis.ViewAnalysisAggregate(
			bvh, nodes.ToVector3D(), num_rays, type: ViewAggregateType.AVERAGE
		);
		Debug.WriteLine(results);
		/// >>>[4.242221, 4.569929, 4.656496, 4.495694, 4.25890,. . .
		///		4.540431, 4.452425, 4.266037, 4.1905]

		// Assign these scores as node attributes in the graph
		string attribute_name = "view_analysis_score";
		graph.AddNodeAttribute(attribute_name, results.CopyArray());

		// Convert these attributes to a cost  by calling AttrsToCosts
		string cost_name = "view_analysis_costs";
		graph.AttrsToCosts(attribute_name, cost_name, Direction.INCOMING);

		// Generate A Path using the default cost
		int start_node = 0; int end_node = 100;
		Path distance_path = ShortestPath.DijkstraShortestPath(graph, start_node, end_node);
		Debug.WriteLine(distance_path);
		/// >>> [(0, 1), (4, 1), (12, 1.418), (26, 1.418), (39, 1.419), (50, 1), (63, 1), (79, 1), (100, 0)]

		// Generate a path using the new cost
		Path view_path= ShortestPath.DijkstraShortestPath(graph, start_node, end_node, cost_name);
		Debug.WriteLine(view_path);
		/// >>> [(0, 4.095), (6, 4.024), (20, 4.05), (31, 4.113), (39, 4.149), (50, 4.216), (63, 4.13), (79, 3.897), (100, 0)]
	
		// Note how the cost to next of the nodes in view_path are close to the values we printed out as view analysis results.
	}
}