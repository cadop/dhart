using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using HumanFactors.RayTracing;
using HumanFactors.SpatialStructures;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;

namespace HumanFactors.VisibilityGraph
{
    /// <summary> Contains all methods for generating Visibility Graphs. </summary>
    public static class VisibilityGraph
    {
        /// <summary>
        /// Create a visibility graph from every node to every node in <paramref name="nodes" />.
        /// </summary>
        /// <param name="bvh"> The BVH to intersect with. </param>
        /// <param name="nodes"> The nodes to use in the visibility graph. </param>
        /// <param name="height">
        /// The height to offset each node from the ground. Any nodes that clip into geometry when
        /// offset by height will not be considered for any edge connections.
        /// </param>
        /// <param name="directed">
        /// if set to <c> true </c> generate a directed graph, otherwise generate an undirected graph.
        /// </param>
        /// <returns> A Graph containing the visbility graph. </returns>
        /// <remarks>
        /// <para>
        /// It is suggested in most contexts to set <paramref name="directed" /> to <see
        /// langword="true" />, due to the complexity of generating a visibility graph. An
        /// undirected visibility graph performs N^2 operations, while a directed visibility graph
        /// only performs n(n+1)/2 operations. The Directed graph saves space by only storing
        /// connections between two nodes in the edgelist of a one of the nodes instead of both nodes.
        /// </para>
        /// The results of the visibility graph can be easily summarized per node by using the
        /// Graph's <see cref="Graph.AggregateEdgeCosts(GraphEdgeAggregation, bool)" /> method.
        /// </remarks>
        public static Graph GenerateAllToAll(EmbreeBVH bvh, IEnumerable<Vector3D> nodes, float height = 1.7f, bool directed = true)
        {
            IntPtr potential_graph = NativeMethods.C_AllToAllVisibilityGraph(bvh.Pointer, nodes, height, directed);
            if (potential_graph == IntPtr.Zero) return null;
            else return new Graph(potential_graph);
        }

        /// <summary>
        /// Generate a directed visibility graph from nodes in <paramref name="group_a" /> to the
        /// nodes in <paramref name="group_b" />.
        /// </summary>
        /// <param name="bvh"> The geometry to use for generating the visibility graph. </param>
        /// <param name="group_a"> The nodes to fire rays from. </param>
        /// <param name="group_b"> The nodes to fire rays to. </param>
        /// <param name="height">
        /// The height to offset nodes from the ground. Any nodes that clip into geometry when
        /// offset by height will not be considered for any edge connections.
        /// </param>
        /// <returns>
        /// A graph contanining the visibility graph. Nodes in group_a will be assigned IDs before
        /// those in group_b. <seealso cref="VisibilityGraph.GenerateAllToAll(EmbreeBVH,
        /// IEnumerable{Vector3D}, float, bool)" />
        /// </returns>
        public static Graph GenerateGroupToGroup(
            EmbreeBVH bvh,
            IEnumerable<Vector3D> group_a,
            IEnumerable<Vector3D> group_b,
            float height = 1.7f
        )
        {
            IntPtr potential_graph = NativeMethods.C_GroupToGroupVisibilityGraph(bvh.Pointer, group_a, group_b, height);
            if (potential_graph == IntPtr.Zero) return null;
            else return new Graph(potential_graph);
        }
    }
}