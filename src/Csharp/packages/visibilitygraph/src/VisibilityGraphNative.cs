
using HumanFactors.NativeUtils;
using HumanFactors.Exceptions;
using System;
using System.Runtime.InteropServices;
using HumanFactors.SpatialStructures;
using System.Runtime.Serialization.Json;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace HumanFactors.VisibilityGraph
{

	/*! \brief Native Functions for the VisibilityGraph */
	internal static class NativeMethods
	{
		private const string dllpath = NativeConstants.DLLPath;

		/*! 
            
            \brief Generate a visibility graph in C++
            
            \param bvh A pointer to the bvh to use in C++.
            \param nodes The nodes to use in the visibility graph.
            \param height The height to offset each node from the ground. 
            \param directed  if set to true generate a directed graph, otherwise generate an undirected graph.

            \returns a pointer to the newly generated VisibilityGraph in C++
        */
		internal static IntPtr C_AllToAllVisibilityGraph(IntPtr bvh, IEnumerable<Vector3D> nodes, float height, bool directed = false)
		{
			// Setup out_ptr to be updated when we call this function
			IntPtr out_ptr = new IntPtr();

			// Flatten array of nodes
			var flat_nodes = HelperFunctions.FlattenVectorArray(nodes);

			// Get Generate the visibility graph in C++, using the undirected function if it's undirected
			// or using the direceted function if it's directed
			HF_STATUS res;
			if (directed == true)
				res = CreateVisibilityGraphAllToAll(bvh, flat_nodes, nodes.Count(), ref out_ptr, height);
			else
				res = CreateVisibilityGraphAllToAllUndirected(bvh, flat_nodes, nodes.Count(), ref out_ptr, height);

			// If no graph could be generated, return null
			if (res == HF_STATUS.NO_GRAPH) return IntPtr.Zero;

			// otherwise, out_ptr will contain a pointer to the new graph
			else return out_ptr;
		}

		/*!
            \brief Generate a directed visibility graph from nodes in `group_a` to the nodes in `group_b.`

            \param bvh A pointer to the raytracer to use in C++.
            \param group_a The nodes to cast rays from.
            \param group_b The nodes to cast rays to.
            \param height The height to offset nodes from the ground. 

            \returns A pointer to the newly generated visibity graph in C++
        */
		internal static IntPtr C_GroupToGroupVisibilityGraph(IntPtr bvh, IEnumerable<Vector3D> group_a, IEnumerable<Vector3D> group_b, float height)
		{
			// Convert group_a and group_b to arrays
			var flat_group_a = HelperFunctions.FlattenVectorArray(group_a);
			var flat_group_b = HelperFunctions.FlattenVectorArray(group_b);

			// Create output parameter
			IntPtr out_ptr = new IntPtr();

			// Call the visibility graph function in C++, updating out-ptr
			HF_STATUS res = CreateVisibilityGraphGroupToGroup(bvh, flat_group_a, flat_group_a.Length / 3, flat_group_b, flat_group_b.Length / 3, ref out_ptr, height);

			// If no graph could be generated, return null
			if (res == HF_STATUS.NO_GRAPH) return IntPtr.Zero;

			// Otherwise, out_ptr will contain a poitner to the new graph.
			else return out_ptr;
		}

		[DllImport(dllpath)]
		private static extern HF_STATUS CreateVisibilityGraphAllToAll(
			IntPtr ert,
			[In] float[] nodes,
			int num_nodes,
			ref IntPtr out_graph,
			float height
		);

		[DllImport(dllpath)]
		private static extern HF_STATUS CreateVisibilityGraphAllToAllUndirected(
			IntPtr ert,
			[In] float[] nodes,
			int num_nodes,
			ref IntPtr out_graph,
			float height
		);

		[DllImport(dllpath)]
		private static extern HF_STATUS CreateVisibilityGraphGroupToGroup(
			IntPtr ert,
			[In] float[] group_a,
			int size_a,
			[In] float[] group_b,
			int size_b,
			ref IntPtr out_graph,
			float height
		);




	}

}
