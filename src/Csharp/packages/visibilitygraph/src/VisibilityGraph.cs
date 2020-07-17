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

/*!
    \brief Evaluate visibility between a set of points. 

    \details
    A Visibility Graph is a graph of points of space locations that have a
    clear line of sight to each other. Edges in a Visibility Graph are bidirectional,
    since if one node can see another node, the inverse is true. Generating a Visibility
    Graph from a set of nodes consists of performing a line of sight check between each
    combination of nodes, and creating an edge if the line of sight check passes. In this
    implementation, the cost of every edge in the visibility graph is equal to the distance
    between the parent and child nodes. 

    \see SpatialStructures.Graph for more information about the type returned from this function.
    \see RayTracing.EmbreeBVH for info on the BVH type required to call these functions. 

*/
namespace HumanFactors.VisibilityGraph
{
    /// <summary> Contains all methods for generating Visibility Graphs.
    public static class VisibilityGraph
    {
        /*!
            \brief Create a visibility graph from every node to every node in nodes.

            \param bvh The BVH to intersect with.
            \param nodes The nodes to use in the visibility graph.
            \param height The height to offset each node from the ground. 
            \param directed  if set to true generate a directed graph, otherwise generate an undirected graph.

            \returns
            A Graph where each node is a node in nodes, and each edge represents a clear line of sight between the parent and
            child node. Any nodes that clip into geometry when offset by height will not be considered 
            for any edge connections and will always have no outgoing or incoming edges.
            
            \attention
            When directed is true. Edges between nodes will only be stored in the edges of the node with the 
            lower ID. To store the edge in the edge array of both nodes use false but be warned, it will
            take longer to execute and take up more memory. Read the following section for more info.

            \par Directed vs Undirected 
            For the sake of this function, an undirected visibility graph describes a graph
            where edge is only stored in the graph for the node with the lowest ID. For example, 
            The first node will check for edges from itself to every node in the set, node 2 will
            check for edges from itself to every other node except for the first node, node 3
            will check every node except for the first and second. In contrast, a directed graph
            will check every node against every other node in the graph regardless of ID. In most cases
            you will want to use the undirected algorithm, as it only performs  n(n+1)/2 operations vs
            the directed's n^2 operations.
			
            \warning Graphs output by this function CANNOT be modified.

            \see SpatialStructures.Graph.AggregateEdgeCosts for a way to easily summarize the results of a visibility graph.
       */
        public static Graph GenerateAllToAll(EmbreeBVH bvh, IEnumerable<Vector3D> nodes, float height = 1.7f, bool directed = true)
        {
            IntPtr potential_graph = NativeMethods.C_AllToAllVisibilityGraph(bvh.Pointer, nodes, height, directed);
            if (potential_graph == IntPtr.Zero) return null;
            else return new Graph(potential_graph);
        }

        /*!
            \brief Generate a directed visibility graph from nodes in `group_a` to the nodes in `group_b.`

            \param bvh The geometry to use for generating the visibility graph.
            \param group_a The nodes to cast rays from.
            \param group_b The nodes to cast rays to.
            \param height The height to offset nodes from the ground. 
            
            \returns
            A graph contanining the visibility graph. Since a new graph is created, all nodes will
            be assigned ids for access within it. First every node in group_a will be assigned
            ids in order, then all nodes in `group_b` will be assigned ids in order. Nodes in `group_a` will
            be assigned IDs before those in `group_b`. All edges will be from nodes in `group_a` to nodes in
            `group_b`. Any nodes that clip into geometry when offset by height will not be considered for
            any edge connections and will always have no outgoing or incoming edges.
            
            \pre No nodes in `group_a` also exist in `group_b` and vice versa.

            \attention
            Any nodes that clip into geometry when offset by `height` will not be considered for any edge connections.
            
            \see GenerateAllToAll to generate a graph from every node to every other node. 
        */
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
