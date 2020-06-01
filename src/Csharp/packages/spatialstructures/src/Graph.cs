using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using System;

namespace HumanFactors.SpatialStructures
{
    /// <summary> Methods for aggregating edge costs per node from the graph </summary>
    public enum GraphEdgeAggregation
    {
        SUM = 0,
        AVERAGE = 1,
        COUNT = 2
    }

    /// <summary> Holds a graph in unmanaged memory as a CSR. </summary>
    public class Graph : NativeObject
    {
        /// <summary> This graph's CSR pointers. </summary>
        public CSRInfo CSRPointers;

        /// <summary> Wrap a graph that already exists in unmanaged memory. </summary>
        /// <param name="GraphPtr"> Pointer to the grpah in unmanaged memory </param>
        internal Graph(IntPtr GraphPtr) : base(GraphPtr, -1) { }

        /// <summary> Construct a new empty graph in unmanaged memory. </summary>
        public Graph() : base(NativeMethods.C_CreateGraph(), -1) { }

        /// <summary> Add a new edge to the graph </summary>
        /// <param name="parent"> x,y,z location for the parent </param>
        /// <param name="child"> x,y,z location for the child </param>
        /// <param name="cost"> cost for parent to child </param>
        public void AddEdge(Vector3D parent, Vector3D child, float cost)
            => NativeMethods.C_AddEdge(handle, parent, child, cost);

        /// <summary> Add a new edge to the graph </summary>
        /// <param name="parent_id"> ID of the parent node </param>
        /// <param name="child_id"> ID of the child node </param>
        /// <param name="cost"> cost for parent to child </param>
        public void AddEdge(int parent_id, int child_id, float cost)
            => NativeMethods.C_AddEdge(handle, parent_id, child_id, cost);

        /// <summary> Get an array of of the graph's current nodes </summary>
        /// <returns> An array of the graph's nodes. </returns>
        public NodeList getNodes() => new NodeList(NativeMethods.C_GetNodes(handle));

        /// <summary> Compress the graph, and get pointers to a CSR representation of it. </summary>
        /// <remarks>
        /// The CSR pointers can be mapped to after retrieved from C++ using <see cref="Span{T}" />.
        /// </remarks>
        public void CompressToCSR()
        {
            this.CSRPointers = NativeMethods.C_GetCSRPointers(handle);
        }

        /// <summary>
        /// Summarize the edgecosts for every node in the graph. This will compress the graph if not
        /// compressed already.
        /// </summary>
        /// <param name="type"> </param>
        /// <param name="directed">
        /// Whether or not the graph is directed. If set to <see langword="true" /> each nodes's
        /// score will only consider incomning edges. Otherwise, each node's score will consider
        /// outgoing and incoming edges.
        /// </param>
        /// <returns>
        /// An array of scores, in which each element corresponds to a node in the graph sorted by ID.
        /// </returns>
        public ManagedFloatArray AggregateEdgeCosts(GraphEdgeAggregation type, bool directed = true)
        {
            this.CompressToCSR();
            CVectorAndData cvad = NativeMethods.C_AggregateEdgeCosts(handle, directed, type);
            cvad.size = getNodes().size;
            return new ManagedFloatArray(cvad);
        }

        /// <summary>
        /// Gets the ID of a node in the graph.
        /// </summary>
        /// <param name="node">The node to get the ID for.</param>
        /// <returns>The ID of the node, or -1 if the node isn't in the graph.</returns>
        public int GetNodeID(Vector3D node) => NativeMethods.C_GetNodeID(handle, node.x, node.y, node.z);

        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyGraph(handle);
            return true;
        }
    }
}