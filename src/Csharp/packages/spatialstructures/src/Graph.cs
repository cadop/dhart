using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using System;

/*!
    \brief Standard fundamental data structures for representing space used throughout HumanFactors. 

    \remarks
    The datatypes in the SpatialStructures namespace are used throughout the HumanFactors.
    For example, the GraphGenerator and VisibilityGraph both produce a Graph as output,
    allowing for the code to manage the Graph's internal CSR to be centralized in a single 
    location. 
*/
namespace HumanFactors.SpatialStructures
{
    /// \brief  Methods for aggregating edge costs per node from the graph.
    /// \see Graph.AggregateEdgeCosts for usage of this enum.
    public enum GraphEdgeAggregation
    {
        SUM = 0, ///< Add the cost of all edges
        AVERAGE = 1,///< Average the cost of all edges
        COUNT = 2 ///< Count the number of edges.
    }

    /*!
        \brief A graph representing connections between points in space.

        \details
        Every Node in the graph contains an X,Y,Z coordinate which can be
        used to represent a specific point in space. This graph internally
        is stored as a CSR for space efficency. Nodes are stored in a hashmap
        containing X,Y, and Z coordinates, allowing for quick indexing of s
        specific nodes by location alone. 
        
        \invariant 1) The CSR maintained by this graph will always be valid. 
        \invariant
        2) Every unique unique node in the graph will be assigned a unique id. A Node
        is considered unique if it has an X,Y,Z location that is not within
        0.0001 units of any other node in the graph.


        \internal
            \todo Functions to access edges in the graph like numpy. Users shouldn't have
            to use unsafe functions to get the edges of a node from the CSR.
        \endinternal

        \see CompressToCSR to get a CSR representation of the graph.
    */
    public class Graph : NativeObject
    {
        /// \brief This graph's CSR pointers.
        private CSRInfo CSRPointers;

        /// \brief Wrap a graph that already exists in unmanaged memory.
        /// <param name="GraphPtr"> Pointer to the grpah in unmanaged memory </param>
        internal Graph(IntPtr GraphPtr) : base(GraphPtr, -1) { }

        /// <summary> Construct a new empty graph in unmanaged memory. </summary>
        public Graph() : base(NativeMethods.C_CreateGraph(), -1) { }

        /*! 
            \brief Create a new edge between parent and child with cost.
            
            \param parent The X,Y,Z location for the parent node.
            \param child x,y,z location for the child 
            \param cost cost for parent to child </param>

            \post
            1) If the X,Y,Z position of either parent or child does not exist in the graph
            then a ID location will be assigned to it.
            \post 2) Existing representations of the this graph's CSR will be invalidated. 

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning 
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.
        */
        public void AddEdge(Vector3D parent, Vector3D child, float cost)
            => NativeMethods.C_AddEdge(handle, parent, child, cost);
        /*!
            \brief Create a new edge between parent and child with cost.
            
            \param parent_id The ID of the parent node.
            \param child_id The ID of the child node. 
            \param cost cost from parent to child.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then it will be created.
            \post 2) Existing representations of the this graph's CSR will be invalidated. 

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.
        */
        public void AddEdge(int parent_id, int child_id, float cost)
            => NativeMethods.C_AddEdge(handle, parent_id, child_id, cost);

        /// \brief Garray containing the graph's current nodes.
        /// \returns An array of the graph's current nodes ordered by ID.
        public NodeList getNodes() => new NodeList(NativeMethods.C_GetNodes(handle));

        /*! 
            \brief Compress the graph, and get pointers to a CSR representation of it.

            \remarks 
            The CSR pointers can be mapped to after retrieved from C++ using spans.

            \see CSRPtrs for more info on the CSR type and how to access it.
        */
        public void CompressToCSR()
        {
            this.CSRPointers = NativeMethods.C_GetCSRPointers(handle);
        }

        /*! 
            \brief Summarize the edgecosts of every node in the graph. 
            
            \param type The type of aggregation method to use.
            
            \param directed
            Whether or not the graph is directed. If set to true then each nodes's 
            score will only consider incomning edges. Otherwise, each node's score will consider
            both outgoing and incoming edges.
            
            \returns
            An array of scores, in which each element corresponds to a node in the graph sorted by ID.

            \warning 
            This will compress the graph if it is not compressed already. If any edges
            were added between lat call to CompressToCSR and now, then any active CSRPtrs
            will be invalidated.

            \remarks
            The order of the scores returned bythis function match the order of the scores returned from
            Graph.getNodes. This can be especially useful for summarizing the results of a VisibilityGraph.
        */
        public ManagedFloatArray AggregateEdgeCosts(GraphEdgeAggregation type, bool directed = true)
        {
            this.CompressToCSR();
            CVectorAndData cvad = NativeMethods.C_AggregateEdgeCosts(handle, directed, type);
            cvad.size = getNodes().size;
            return new ManagedFloatArray(cvad);
        }

        /*! 
            \brief Gets the ID of a node in the graph.
            
            \param node The X,Y,Z position of a node to get the ID for.

            \returns The ID of the node, or -1 if the node isn't in the graph.
        */
        public int GetNodeID(Vector3D node) => NativeMethods.C_GetNodeID(handle, node.x, node.y, node.z);

	    /*!
		 \brief Free the native memory managed by this class. 
		 \note the garbage collector will handle this automatically
		 \warning Do not attempt to use this class after freeing it!
		 \returns True. This is guaranteed to execute properly.  
		*/
        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyGraph(handle);
            return true;
        }
    }

    public static class CostAlgorithms
    {
        public static float FloatFromC()
        {
            return NativeMethods.C_FloatFromC();
        }


        /*!
            \brief  Calculates and stores cross slope for all subgraphs in g
            \param  g   The operand Graph

            \code
                // Create the graph
                Graph g = new Graph();

                // Create 7 nodes
                Vector3D n0 = new Vector3D(2, 6, 6);
                Vector3D n1 = new Vector3D(0, 0, 0);
                Vector3D n2 = new Vector3D(-5, 5, 4);
                Vector3D n3 = new Vector3D(-1, 1, 1);
                Vector3D n4 = new Vector3D(2, 2, 2);
                Vector3D n5 = new Vector3D(5, 3, 2);
                Vector3D n6 = new Vector3D(-2, -5, 1);

                // Add 9 edges
                g.AddEdge(n0, n1, 0); // [ -2, -6, -6 ]
                g.AddEdge(n1, n2, 0); // [ -5,  5,  4 ]
                g.AddEdge(n1, n3, 0); // [ -1,  1,  1 ]
                g.AddEdge(n1, n4, 0); // [  2,  2,  2 ]
                g.AddEdge(n2, n4, 0); // [ -9, -3, -2 ]
                g.AddEdge(n3, n5, 0); // [ -6,  2,  1 ]
                g.AddEdge(n5, n6, 0); // [ -7, -8, -1 ]
                g.AddEdge(n4, n6, 0); // [ -6, -7, -1 ]

                // Compress the graph after adding edges
                g.CompressToCSR();

                // Calculate and store edge type in g: cross slope
                CostAlgorithms.CalculateAndStoreCrossSlope(g);
            \endcode
         */
        public static void CalculateAndSioreCrossSlope(Graph g)
        {
            NativeMethods.C_CalculateAndStoreCrossSlope(g.Pointer);
        }
 
    }
}