
using DHARTAPI.NativeUtils;
using DHARTAPI.Exceptions;
using System;
using System.Runtime.InteropServices;

namespace DHARTAPI.GraphGenerator
{

	/*! \brief Native methods for the GraphGenerator */
	internal static class NativeMethods
	{
		const string dllpath = NativeConstants.DLLPath;

		/*!
			\brief Generate a graph using the C++ graph generator.  
		
			\param bvh A pointer to a valid EmbreeBVH in native memory. 
			\param start_point The starting point for the graph generator. If this isn't above solid ground, no nodes will
			be generated.
			\param spacing Space between nodes.Lower values will yield more nodes for a higher resolution graph.
			\param max_nodes The maximum amount of nodes to generate.
			\param up_step Maximum height of a step the graph can traverse.Any steps higher this will be considered inaccessible. 
			\param up_slope Maximum upward slope the graph can traverse in degrees.Any slopes steeper than this
			will be considered inaccessible.
			\param down_step Maximum step down the graph can traverse.Any steps steeper than this will be considered inaccessible.
			\param down_slope
			The maximum downward slope the graph can traverse. Any slopes steeper than this will be
			considered inaccessible.
			\param max_step_connections Multiplier for number of children to generate for each node. Increasing this value will
			increase the number of edges in the graph, and as a result the amount of memory the
			algorithm requires.
			\param min_connections The required out-degree for a node to be valid and stored.
			 This must be greater than 0 and equal or less than the total connections created from max_step_connections.
			 Default is 1. A value of 8 when max_step_connections=1 would be a grid.
			\param core_count Number of cores to use. -1 will use all available cores, and 0 will run a serialized version of the algorithm.
			\param walkable_id IDs of geometry to be considered as obstacles
			\param obstacle_id IDs of geometry to be considered as walkable surfaces

			\returns 
			A pointer to a graph generated with the given parameters or a null pointer if no graph could be generated with the 
			given parameters.
		*/
		internal static IntPtr C_GenerateGraph(
			IntPtr bvh,
			Vector3D start_point,
			Vector3D spacing,
			int max_nodes = -1,
			float up_step = 0.2f,
			float up_slope = 20,
			float down_step = 0.2f,
			float down_slope = 20,
			int max_step_connections = 1,
			int min_connections =1,
			int core_count = -1,
			int[] obstacle_ids = null,
			int[] walkable_ids = null)
		{
			IntPtr out_graph = new IntPtr();
			float[] start_arr = start_point.ToArray();
			float[] spacing_arr = spacing.ToArray();

			// If obstacles/IDs are null, conider their links as 0
			int num_obstacles = (obstacle_ids != null) ? obstacle_ids.Length : 0;
			int num_walkables = (walkable_ids != null) ? walkable_ids.Length : 0;

			// Call the basic graph enerator if there's no obstacle geometry, otherwise call
			// the obstacle version of the graph generator
			HF_STATUS res = HF_STATUS.GENERIC_ERROR;
			if (num_obstacles == 0)
			{
				res = GenerateGraph(
					bvh,
					start_arr,
					spacing_arr,
					max_nodes,
					up_step,
					up_slope,
					down_step,
					down_slope,
					max_step_connections,
					min_connections,
					core_count,
					ref out_graph
				);
			}
			else
			{
				res = GenerateGraphObstacles(
					bvh,
					start_arr,
					spacing_arr,
					max_nodes,
					up_step,
					up_slope,
					down_step,
					down_slope,
					max_step_connections,
					min_connections,
					core_count,
					obstacle_ids,
					walkable_ids,
					num_obstacles,
					num_walkables,
					ref out_graph
				);
			}

			// Return a null pointer if no graph could be generated
			if (res == HF_STATUS.NO_GRAPH)
				return IntPtr.Zero;
			else
				return out_graph;
		}

		[DllImport(dllpath)]
		private static extern HF_STATUS GenerateGraph(
			IntPtr ray_tracer,
			[In] float[] start_point,
			[In] float[] end_point,
			int MaxNodes,
			float UpStep,
			float UpSlope,
			float DownStep,
			float DownSlope,
			int max_step_connection,
			int min_connections,
			int core_count,
			ref IntPtr out_graph
		);

		[DllImport(dllpath)]
		private static extern HF_STATUS GenerateGraphObstacles(
			IntPtr ray_tracer,
			[In] float[] start_point,
			[In] float[] end_point,
			int MaxNodes,
			float UpStep,
			float UpSlope,
			float DownStep,
			float DownSlope,
			int max_step_connection,
			int min_connections,
			int core_count,
			int[] obstacle_ids,
			int[] walkable_ids,
			int num_obstacles,
			int num_walkables,
			ref IntPtr out_graph
		);

	}
}
