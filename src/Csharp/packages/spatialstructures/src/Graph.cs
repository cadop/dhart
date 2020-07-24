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
        is stored as a Sparse Matrix for space efficency. Nodes are stored
        in a hashmap containing X,Y, and Z coordinates, allowing for quick 
        indexing of specific nodes by location alone. Access to this graph's
        internal CSR is available through Graph.CompressToCSR().

        \par Cost Types
        This Graph is capable of holding multiple cost types for any of it's edges.
        Each cost type has a distinct key as it's name, such as "CrossSlope" or
        "EnergyExpenditure". Upon creation, the graph is assigned a default cost
        type, `Distance` which can be accessed explicitly by the key "Distance" or
        leaving the cost_type field blank. Alternate costs have corresponding edges 
        in the default cost set, but different costs to traverse from the parent 
        to the child node. 
        
        \invariant 1) The CSR maintained by this graph will always be valid. 
        \invariant
        2) Every unique unique node in the graph will be assigned a unique id. A Node
        is considered unique if it has an X,Y,Z location that is not within
        0.0001 units of any other node in the graph.

        \note
        The graph offers some basic functionality to add edges and nodes but it's main use
        is to provide access to the output of the GraphGenerator and VisibilityGraph. If
        adding edges or alternate cost types please make sure to read the documentation
        for these functions and that all preconditions are followed. 

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
            \param cost cost for parent to child 
            \param cost_type The type of cost to add the edge to. 


            \Attention
            This overload is meant for debugging. There are many issues that can occur with
            adding integers to the graph that don't already have nodes assigned. Instead
            use the overload of this function deals with vector 3.

            \pre 1) If adding edges to a new cost type, the graph has been compressed
            by calling CompressToCSR().
            \pre 2) If adding a cost to a type that isn't the default cost type, the edge must
            already exist in the default cost type.
            \pre 3) If the graph has already been compressed and alternate costs exist, then
            both `parent` and `child` already exist in the graph.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then New IDs will be assigned to them. 
            \post 2) Existing representations of the this graph's CSR will be invalidated. 
            \post 3) If `cost_type` is not blank, and does not refer to the default cost type
            or any other cost that already exists in the graph, a new cost type will be created. 
            
            \throws LogicError tried to add an alternate cost to the graph before it was compressed
            \throws InvalidCostOperation Tried to add an alternate cost that doesn't exist in the graph's
            default cost type.

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning 
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.
        */
        public void AddEdge(Vector3D parent, Vector3D child, float cost, string cost_type = "")
            => NativeMethods.C_AddEdge(handle, parent, child, cost, cost_type);
        /*!
            \brief Create a new edge between parent and child with cost.
            
            \param parent_id The ID of the parent node.
            \param child_id The ID of the child node. 
            \param cost cost from parent to child.
            \param cost_type The type of cost to add this edge to. If left blank
            will add the edge to the graph's default cost type.

            \pre 1) If adding edges to a new cost type, the graph must first be compressed
            by calling CompressToCSR()
            \pre 2) If adding a cost to a type that isn't the default cost type, the edge must
            already exist in the default cost type.
            \pre 3) If the graph has already been compressed and alternate costs exist, then
            both `parent` and `child` already exist in the graph.

            \post
            1) If the ID of either parent or child does not exist in the graph
            then New IDs will be assigned to them. 
            \post 2) Existing representations of the this graph's CSR will be invalidated. 
            \post 3) If `cost_type` is not blank, and does not refer to the default cost type
            or any other cost that already exists in the graph, a new cost type will be created. 
            
            \throws LogicError tried to add an alternate cost to the graph before it was compressed
            \throws InvalidCostOperation Tried to add an alternate cost that doesn't exist in the graph's
            default cost type.

            \warning 
            1) If an edge between parent and child already exists, this will overwrite
            that edge.
            \warning
            2) Calling this function will invalidate any existing CSRPtrs
            returned from the graph. Make sure to call CompressToCSR again continuing
            to access it.
        */
        public void AddEdge(int parent_id, int child_id, float cost, string cost_type = "")
            => NativeMethods.C_AddEdge(handle, parent_id, child_id, cost, cost_type);

        /// \brief Garray containing the graph's current nodes.
        /// \returns An array of the graph's current nodes ordered by ID.
        public NodeList getNodes() => new NodeList(NativeMethods.C_GetNodes(handle));

        /*! 
            \brief Compress the graph into a CSR representation, and get pointers to it.

            \param cost_type Change the type of cost that's carried in the CSR's 
            values array. If left blank, will use the graph's default cost type. 

            \remarks 
            The CSR pointers can be mapped to after retrieved from C++ using spans, or
            can be copied out of managed memory.

            \throws KeyNotFoundException `cost_type` is not blank, the name of the graph's
            default cost type, or the name of any already defined cost type in
            the graph.

            \see CSRPtrs for more info on the CSR type and how to access it.
        */
        public CSRInfo CompressToCSR(string cost_type = "")
        {
            this.CSRPointers = NativeMethods.C_GetCSRPointers(handle, cost_type);
            return this.CSRPointers;
        }

        /*! 
            \brief Summarize the edgecosts of every node in the graph. 
            
            \param type The type of aggregation method to use.
            
            \param directed
            Whether or not the graph is directed. If set to true then each nodes's 
            score will only consider incomning edges. Otherwise, each node's score will consider
            both outgoing and incoming edges.
            
            \param cost_type The type of cost to use for aggregating the edges.
            
            \returns
            An array of scores, in which each element corresponds to a node in the graph sorted by ID.

            \pre If not left blank, cost_type must point to a valid cost in the graph. 

            \throws KeyNotFoundException Cost specified didn't match the default cost, or any other
            cost type defined in the graph.

            \warning 
            This will compress the graph if it is not compressed already. If any edges
            were added between lat call to CompressToCSR and now, then any active CSRPtrs
            will be invalidated.

            \remarks
            The order of the scores returned bythis function match the order of the scores returned from
            Graph.getNodes. This can be especially useful for summarizing the results of a VisibilityGraph.
        */
        public ManagedFloatArray AggregateEdgeCosts(
            GraphEdgeAggregation type,
            bool directed = true,
            string cost_type = ""
        ) {
            this.CompressToCSR();
            CVectorAndData cvad = NativeMethods.C_AggregateEdgeCosts(handle, directed, type, cost_type);
            cvad.size = getNodes().size;
            return new ManagedFloatArray(cvad);
        }


        /*!
            \brief Get the cost of traversing between `parent` and `child`

            \param parent Node that's being traversed from
            \param child Node that's being traversed to
            \param cost_type The cost type to retrieve. If blank, the graph's
            default cost will be used.

            \returns The cost of traversing from `parent` to `child`.

			\pre 1) cost_type must be the name of a cost that already exists in the graph,
			or blank. 
            \pre 2) The graph must first have been compressed using Graph.CompressToCSR().
            
            \throws LogicError The graph wasn't compressed before calling this function.
			\throws KeyNotFoundExeption The cost_type specified was not the default cost, blank, or 
			the name of any cost that currently belongs to the graph.

            \note This is not a high performance function, since each index into the CSR requires
            an indexing operation. If multiple values are required, it is suggested to iterate through
            the pointers from Graph.CompressToCSR().

        */
        public float GetCost(int parent, int child, string cost_type = "")
        {
            return NativeMethods.C_GetEdgeCost(this.Pointer, parent, child, cost_type);
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
}