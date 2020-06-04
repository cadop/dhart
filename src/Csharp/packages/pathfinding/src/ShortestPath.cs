using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using HumanFactors.SpatialStructures;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;

namespace HumanFactors.Pathfinding
{
    /// <summary>
    /// Methods to calculate the shortest path for <see cref="SpatialStructures.Graph" />
    /// </summary>
    public static class ShortestPath{

        /// <summary>
        /// Perform Dijkstra's shortest path algorithm to find a path from <paramref name="start_id"
        /// /> to <paramref name="end_id" />
        /// </summary>
        /// <param name="graph"> The graph to conduct the search on. </param>
        /// <param name="start_id"> The ID of the node to start at. </param>
        /// <param name="end_id"> The ID of the node to path to. </param>
        /// <returns> A path from <paramref name="start_id" /> to <paramref name="end_id" /> </returns>
        /// <exception cref="IndexOutOfRangeException">
        /// <paramref name="start_id" /> or <paramref name="end_id" /> are not the ids of any nodes
        /// in the graph.
        /// </exception>
        /// <remarks>
        /// For searching multiple paths at once efficiently it's recommended to use <see
        /// cref="DijkstraShortestPathMulti(Graph, IEnumerable{Vector3D}, IEnumerable{Vector3D})" />.
        /// </remarks>
        public static Path DijkstraShortestPath(Graph graph, int start_id, int end_id)
        {
            CVectorAndData cvad = NativeMethods.C_CreatePath(graph.Pointer, start_id, end_id);
            if (cvad.size <= 0)
                return null;
            else
                return new Path(cvad);
        }

        /// <summary>
        /// Perform Dijkstra's shortest path algorithm to find a path from <paramref name="start_id"
        /// /> to <paramref name="end_id" />.
        /// </summary>
        /// <param name="graph"> The graph to conduct the search on. </param>
        /// <param name="start_node"> The node to start at. </param>
        /// <param name="end_node"> The node to end at. </param>
        /// <returns>
        /// A path from <paramref name="start_node" /> to <paramref name="end_node" />.
        /// </returns>
        /// <exception cref="IndexOutOfRangeException">
        /// <paramref name="start_id" /> or <paramref name="end_id" /> do not exist in the graph.
        /// </exception>
        /// <remarks>
        /// For searching multiple paths at once efficiently use <see
        /// cref="DijkstraShortestPathMulti(Graph, IEnumerable{Vector3D}, IEnumerable{Vector3D})" />
        /// </remarks>
        /// <remarks>
        /// For searching multiple paths at once efficiently it's recommended to use <see
        /// cref="DijkstraShortestPathMulti(Graph, IEnumerable{Vector3D}, IEnumerable{Vector3D})" />.
        /// </remarks>
        public static Path DijkstraShortestPath(Graph graph, Vector3D start_node, Vector3D end_node)
        {
            int parent_id = graph.GetNodeID(start_node);
            int child_id = graph.GetNodeID(end_node);

            return DijkstraShortestPath(graph, parent_id, child_id);
        }

        /// <summary> Find the shortest paths between each pair of start_id and end_id in order. </summary>
        /// <param name="graph"> The graph to generate paths in. </param>
        /// <param name="start_ids">
        /// Ids for the start points to generate paths from. Length should be equal to that of
        /// <paramref name="end_ids" />.
        /// </param>
        /// <param name="end_ids">
        /// Ids for the end points to generate paths to. Length should be equal to that of <paramref
        /// name="start_ids" />.
        /// </param>
        /// <returns> </returns>
        /// <exception cref="System.ArgumentException">
        /// Length of <paramref name="start_ids"/> didn't equal length of <paramref name="end_ids"/>.
        /// </exception>
        /// <exception cref="IndexOutOfRangeException">
        /// One or more of the start or end ids do not exist in <paramref name="graph" />.
        /// </exception>
        public static Path[] DijkstraShortestPathMulti(Graph graph, int[] start_ids, int[] end_ids)
        {
            if (start_ids.Length != end_ids.Length) 
                throw new ArgumentException("Length of start_ids didn't equal length of end_ids");

            CVectorAndData[] cvads = NativeMethods.C_CreatePaths(graph.Pointer, start_ids, end_ids);
            Path[] paths = new Path[start_ids.Length];
            for(int i = 0; i < cvads.Length; i++)
            {
                if (cvads[i].IsValid())
                    paths[i] = new Path(cvads[i]);
                else
                    paths[i] = null;
            }

            return paths;
        }

        /// <summary> Find the shortest paths between each pair of start_id and end_id in order. </summary>
        /// <param name="graph"> The graph to generate paths in. </param>
        /// <param name="start_nodes">
        /// Start points to generate paths from. Length should be equal to that of <paramref
        /// name="end_nodes" />.
        /// </param>
        /// <param name="end_nodes">
        /// End points to generate paths to. Length should be equal to that of <paramref
        /// name="start_nodes" />.
        /// </param>
        /// <returns> </returns>
        /// <exception cref="System.ArgumentException">
        /// Length of <paramref name="start_nodes"/> didn't equal length of <paramref name="end_nodes"/>.
        /// </exception>
        /// <exception cref="IndexOutOfRangeException">
        /// One or more of the start or end nodes do not exist in <paramref name="graph" />.
        /// </exception>
        public static Path[] DijkstraShortestPathMulti(Graph graph, IEnumerable<Vector3D> start_nodes, IEnumerable<Vector3D> end_nodes)
        {
            if (start_nodes.Count() != end_nodes.Count())
                throw new ArgumentException("Length of start_nodes didn't equal length of end_nodes");

            int size = start_nodes.Count();
            int[] start_ids = new int[size];
            int[] end_ids = new int[size];

            int i = 0;
            foreach (var start_end in start_nodes.Zip(end_nodes, (start, end) => new Tuple<Vector3D, Vector3D>(start, end)))
            {
                start_ids[i] = graph.GetNodeID(start_end.Item1);
                end_ids[i] = graph.GetNodeID(start_end.Item2);
                i++;
            }

            return DijkstraShortestPathMulti(graph, start_ids, end_ids);
        }

    }
}