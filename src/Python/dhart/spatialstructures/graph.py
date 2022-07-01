import ctypes
import numpy
from numpy.lib import recfunctions as rfn
from scipy.sparse import csr_matrix
from scipy.spatial.distance import cdist
from ctypes import c_float, c_int, c_void_p
from typing import *
from enum import IntEnum

from dhart.native_numpy_like import NativeNumpyLike
from .node import NodeStruct, NodeList
from . import spatial_structures_native_functions

__all__ = ['CostAggregationType','EdgeSumArray','Graph', 'Direction']

class CostAggregationType(IntEnum):
    SUM = 0
    AVERAGE = 1
    COUNT = 2
    
class Direction(IntEnum):
    INCOMING = 0
    OUTGOING = 1
    BOTH = 2

class EdgeSumArray(NativeNumpyLike):
    """ Contains the scores for every node based on edges """

    native_type = c_float 
    delete_fp = spatial_structures_native_functions.C_DeleteFloatArray

    def __init__(
        self, vector_ptr: ctypes.c_void_p, data_ptr: ctypes.c_void_p, size: int
    ):
        super().__init__(vector_ptr, data_ptr, size)


class Graph:
    """ A graph representing connections between points in space.

    Every Node in the graph contains a set of  X,Y,Z coordinates which can be
    used to represent a specific point in space. This graph internally is 
    stored as a Sparse Matrix for space efficency. Nodes are stored in a 
    hashmap containing X,Y, and Z coordinates, allowing for quick indexing
    of specific nodes by location alone. Access to this graph's internal
    CSR is available through Graph.CompressToCSR().

    Cost Types:
        This Graph is capable of holding multiple cost types for any of it's 
        edges. Each cost type has a distinct key as it's name, such as 
        "CrossSlope" or "EnergyExpenditure". Upon creation, the graph is 
        assigned a default cost type, Distance which can be accessed explicitly 
        by the key "Distance" or leaving the cost_type field blank. Alternate 
        costs have corresponding edges in the default cost set, but different 
        costs to traverse from the parent to the child node.

    Node Attributes:
        The graph is able to store an arbitrary amount of information about the 
        nodes it contains as strings. Similar to alternate cost types, node 
        attributes are each have a distinct key as their name, but instead of 
        conatining information about edges in the graph, node attributes contain 
        information about nodes. Unlike the cost algorithms in edgecosts, right 
        now there is no functionality within HumanFactors that populates the
        node attributes of the graph with any kind of metric, however the 
        methods to add and clear node attributes are made available so you are 
        free to add your own node attributes.

    Attributes:
        csr: csr pointing to the underlying graph in C++.
    """

    graph_ptr: Union[ctypes.c_void_p, None] = None  # A pointer to the graph in C++
    __data_array: numpy.array
    __outer_indices: numpy.array
    __inner_indices: numpy.array
    csr: csr_matrix

    def __init__(self, graph_ptr: Union[c_void_p, None] = None):
        """ Create a new graph wrapper object from a graph pointer """
        if not graph_ptr:
            self.graph_ptr = spatial_structures_native_functions.C_CreateGraph(None)
        else:
            self.graph_ptr = graph_ptr

    def CompressToCSR(self, cost_type : str = "") -> csr_matrix:
        """ Compress the graph if needed then return a reference as a CSR matrix

        If the graph is already compressed, this does no work in native code,
        and just returns the CSR.

        Parameters:
            cost_type : str, optional
                The cost type to use for constructing the CSR. Defaults to empty string. If left as default, the graph's default cost type will 
                be used

        Returns:
            A scipy csr matrix mapped to the underlying CSR of this graph
            in native code.

        Pre Condition:
            If cost_type isn't the default, it must be the key of an already
            existing cost type in the graph
        
        Raises:
            KeyError: 
            cost_type wasn't the default cost and didn't match the name of any
            existing cost in the graph.
        """

        # Get the CSR matrix from native code
        (
            nnz,
            row_count,
            col_count,
            data_ptr,
            inner_indices_ptr,
            outer_indices_ptr,
        ) = spatial_structures_native_functions.C_GetCSRPtrs(self.graph_ptr, cost_type)

        # Update this class's internal attributes
        self.nnz = nnz
        self.row_count = row_count
        self.col_count = col_count

        data_nparr_pointer = ctypes.cast(data_ptr, ctypes.POINTER(c_float))
        outer_nparr_pointer = ctypes.cast(outer_indices_ptr, ctypes.POINTER(c_int))
        inner_nparr_pointer = ctypes.cast(inner_indices_ptr, ctypes.POINTER(c_int))

        # Map numpy arrays to pointers in native memory
        self.__data_array = numpy.ctypeslib.as_array(data_nparr_pointer, shape=(nnz,))
        self.__outer__indices_array = numpy.ctypeslib.as_array(
            outer_nparr_pointer, shape=(row_count + 1,)
        )
        self.__inner__indices_array = numpy.ctypeslib.as_array(
            inner_nparr_pointer, shape=(nnz,)
        )

        # Map a numpy CSR to this CSR
        self.csr = csr_matrix(
            (
                self.__data_array,
                self.__inner__indices_array,
                self.__outer__indices_array,
            ),
            (row_count, col_count),
            copy=False,
        )

        # Return a reference to the caller
        return self.csr

    def GetNodeID(self, node: Tuple[float, float, float]) -> int:
        """ Get the id of a node in the graph

        Returns:
            int: the id of node or -1 if the node doesn't exist in the graph
        """
        return spatial_structures_native_functions.C_GetNodeID(self.graph_ptr, node)

    def AddEdgeToGraph(
        self,
        parent: Union[Tuple[float, float, float], NodeStruct, int],
        child: Union[Tuple[float, float, float], NodeStruct, int],
        cost: float,
        cost_type: str = ""
    ) -> None:
        """ Add a new edge to the graph from parent to child with the given cost

        Args:
            parent : Either a node with values for x,y, and z, or an integer representing a node ID
            child : Where the edge from parent is going to. Must be the same type of parent i.e. if parent is an int, then the child must also be an int
            cost : the cost from parent to child
            cost_type : The type of cost to add this edge to. If left blank or as the empty string then the edge will be added to the graph's default cost set.
        
        Pre Conditions:
            1) If cost_type is not left blank, then the edge from parent to 
               child must already exist in the graph for its default cost type.
            2) If adding costs to an alternate cost type, then the graph must
               already be compressed

        Post Conditions:
            1) If parent or child doesn't exist in the graph as nodes, they will 
               be added
            2) If cost_type doesn't already exist in the graph it will be 
               createda
        
        Raises:
            dhart.Exceptions.LogicError:
                Tried to add an alternate cost type to an uncompressed graph
            dhart.Exceptions.InvalidCostOperation:
                Tried to add an alternate cost tot he graph with an edge
                that didn't first exist in the graph's default cost set

        Note:
          The graph offers some basic functionality to add edges and nodes but 
          it's main use is to provide access to the output of the GraphGenerator
          and VisibilityGraph. If adding edges or alternate cost types please
          make sure to read the documentation for these functions and that 
          all preconditions are followed.

        Warning:
            1) Once any edges have been added to the graph as an alternate cost 
               type, new edges are no longer able to be created.
            2) While this function can be called with integers for both paren
               and child id, doing so is not recommended unless you are
               sure that both ids already exist in the graph. If an ID is added
               that does not already exist in the graph, it will cause certain
               functions to behave incorrectly. This was mostly implemented
               for the sake of testing.
        """

        if isinstance(parent, int) and isinstance(child, int):
            spatial_structures_native_functions.C_AddEdgeFromNodeIDs(
                self.graph_ptr, parent, child, cost, cost_type
            )
        else:
            spatial_structures_native_functions.C_AddEdgeFromNodes(
                self.graph_ptr, parent, child, cost, cost_type
            )

    def getNodes(self) -> NodeList:
        """ Get a list of nodes from the graph as a nodelist 

        The graph generator guarantees the order of nodes in the array to correspond 
        with the id. However, if the graph used to call this method is post-modified
        for example, by adding edges manually, this may not hold true. 

        """

        ret = spatial_structures_native_functions.GetNodesFromGraph(self.graph_ptr)
        return NodeList(ret[0], ret[1])

    def get_closest_nodes(self, p_desired, x=True, y=True, z=True):
        """ Get the closet nodes to the input set of points

        Parameters
        ----------

        p_desired : ndarray
            n x m shape where n is the number of points an m is equal to the axis set to True
        x : bool, optional
            Include x axis in the distance comparison, Default True
        y : bool, optional
            Include y axis in the distance comparison, Default True
        z : bool, optional
            Include z axis in the distance comparison, Default True

        Returns
        -------

        int or ndarray[int]
            int if a single point is passed, ndarray if multiple points are passed,
            where ndarray is size of number of points passed in p_desired

        Warning
        -------

        Size of points in `p_desired` must be same as the number of x,y,z axis set to True 

        Note
        ----

        In future this should be calling a method in C interface instead of implemented in Python

        Examples
        --------

        >>> import numpy as np

        >>> import dhart
        >>> from dhart.geometry import LoadOBJ
        >>> from dhart.graphgenerator import GenerateGraph
        >>> from dhart.raytracer import EmbreeBVH

        >>> # Load BVH
        >>> obj_path = dhart.get_sample_model("energy_blob_zup.obj")
        >>> loaded_obj = LoadOBJ(obj_path)
        >>> embree_bvh = EmbreeBVH(loaded_obj)

        >>> # Set graph parameters 
        >>> start_point, spacing, max_nodes = (-30, 0, 20), (1, 1, 10), 100000

        >>> # Generate the graph
        >>> graph = GenerateGraph(embree_bvh, start_point, spacing, max_nodes)

        Check closest nodes using single x,y point

        >>> closest_node = graph.get_closest_nodes(np.array([30,0]), z=False)
        >>> print("Closest Node: ", closest_node)
        Closest Node:  2421

        Check closest nodes using multiple x,y points

        >>> closest_node = graph.get_closest_nodes(np.array([[30,0],[20,20]]), z=False)
        >>> print("Closest Node: ", closest_node)
        Closest Node:  [2421 2450]

        Check closest nodes using single x,y,z point

        >>> closest_node = graph.get_closest_nodes(np.array([30,0,0]))
        >>> print("Closest Node: ", closest_node)
        Closest Node:  2422

        Check closest nodes using multiple x,y,z points

        >>> closest_node = graph.get_closest_nodes(np.array([[30,0,0],[20,20,0]]))
        >>> print("Closest Node: ", closest_node)
        Closest Node:  [2422 2450]

        """

        # if this is one point, wrap in a list
        if len(p_desired.shape) == 1:
            p_desired=[p_desired]
            # Get the closest node to the point 
            closest_nodes = cdist(p_desired, self.get_node_points(x=x, y=y, z=z)).argmin()

        else:
            # Get the closest node to the point and return array
            closest_nodes = cdist(p_desired, self.get_node_points(x=x, y=y, z=z)).argmin(axis=1)

        return closest_nodes

    def get_closest_points():
        """ 
            Get the closest point in the graph to the input set of points

            to bne done Implement this

        """

        return 

    def get_node_points(self, x=True, y=True, z=True):
        """
        Get the nodes of the graph as xyz values in a numpy array by defining the array name
        view to be used as the x,y,z values. It uses the `structured_to_unstructured` 
        method of numpy to perform the conversion.

        Parameters
        ----------

        x : bool, optional
        y : bool, optional
        z : bool, optional

        Returns
        -------

        ndarray[n x 3]
            where n is the number of nodes in the graph and 3 is an x,y,z point

        Notes
        -----

        Copys? (TODO check this) which means the array will not change if the graph is modified

        Examples
        --------

        >>> 
        >>> 

        """

        # first get the nodes
        nodes = self.getNodes()

        # Create list of point names to return, e.g, ['x','y','z']
        point_names = []
        if x: point_names.append('x')
        if y: point_names.append('y')
        if z: point_names.append('z')

        # extract the array names of the xyz values of interest
        node_xyz = nodes.array[point_names]
        # convert to an unstructured array to be seen like normal numpy array
        node_point_array = rfn.structured_to_unstructured(node_xyz)

        return node_point_array

    def Clear(self, cost_type : str = ""):
        """ Clear all edges/nodes from the graph or a specific cost type
            
        Args:
            cost_type : str, optional
                If left as the empty string, clear all nodes/edges from the
                graph. Otherwise, clear edges from the specified cost type.

        Raises:
            KeyError:
                Tried to clear a cost type that didn't exist in the graph.
        
        """
        spatial_structures_native_functions.C_ClearGraph(self.graph_ptr, cost_type)

    def ConvertToLists(self, just_nodes = False) -> Tuple[List[Tuple], List[Tuple]]:
        """ Convert the CSR to a list of nodes and tuples, useful for serialization """
        
        node_array = self.getNodes().array.tolist()
        nodes = [(n[0], n[1], n[2], n[4]) for n in node_array]
        if just_nodes:
            return nodes

        csr = self.CompressToCSR()
        ascoo = csr.asformat("coo")
        edges = list(zip(ascoo.row.tolist(), ascoo.col.tolist(), ascoo.data.tolist()))

        return nodes, edges

    def AggregateEdgeCosts(
            self,
            ct: CostAggregationType,
            directed: bool,
            cost_type: str = "") -> EdgeSumArray:
        """ Get an aggregated score for every node in the graph based on
            its edges.

        Parameters:
            ct : CostAggregationType
                Aggregation method to use for summarizing edge costs. See
                CostAggregationType for more information
            directed : bool
                If false, only consider outgoing edges of each node. If true,
                consider outgoing and incoming edges.
            cost_type : str
                Name of the cost set to use for aggregation. If left as blank,
                will use the default cost of the graph.

        Returns:
            An array of floats containing  aggregated cost score for each node 
            in the graph ordered by node ID.
        
        Raises:
            KeyError:
                cost_type wasn't set to an empty string or the key of any
                cost type in the graph.
         """

        vector_ptr, data_ptr = spatial_structures_native_functions.C_AggregateEdgeCosts(
            self.graph_ptr, ct, directed, cost_type)
        return EdgeSumArray(vector_ptr, data_ptr, len(self.getNodes()))

    def __del__(self):
        if self.graph_ptr:
            spatial_structures_native_functions.DestroyGraph(self.graph_ptr)

    def GetEdgeCost(self, parent: int, child: int, cost_type: str = "") -> float:
        """ Get the cost from parent to child for a specific cost type

        Args:
            parent (int): Node the edge is from
            child (int):  Node the edge is to 
            cost_type (str, optional): Cost type to get the cost from. If left blank will use the graph's default cost type. 

        Returns:
            float : The cost of traversing from parent to child. Will be -1 if no edge exists
        """

        return spatial_structures_native_functions.C_GetEdgeCost(
            self.graph_ptr,
            parent,
            child,
            cost_type
            )

    def NumNodes(self) -> int:
        """Get the number of nodes in the graph."""
        return spatial_structures_native_functions.C_NumNodes(self.graph_ptr)

    def add_node_attributes(
        self, attribute: str, ids: Union[int, List[int]], scores: Union[str, List[Any]],
        ) -> None:
        """ Add attributes to one or more nodes
        
        Args:
            attribute : Unique key of the attribute to assign scores for
            ids : Node IDS in the graph to assign attributes to
            scores : Scores to assign to the ids at the same index
        
        Preconditions:
            1) IDs in ids must already belong to nodes in the graph
            2) The length of scores and ids must match

        Raises:
            ValueError : the length of ids and scores did not match
        
        Example:
            Add node attributes to a graph

           >>> from dhart.spatialstructures import Graph
           >>> # Create a simple graph with 3 nodes
           >>> g = Graph()
           >>> g.AddEdgeToGraph(0, 1, 100)
           >>> g.AddEdgeToGraph(0, 2, 50)
           >>> g.AddEdgeToGraph(1, 2, 20)
           >>> csr = g.CompressToCSR()

           >>> # Add node attributes to the simple graph
           >>> attr = "Test"
           >>> ids = [0, 1, 2]
           >>> scores = ["zero", "one", "two"]
           >>> g.add_node_attributes(attr, ids, scores)

           >>> # To ensure that they've been added properly we will call
           >>> # get_node_attributes.
           >>> g.get_node_attributes(attr)
           ['zero', 'one', 'two']

        """
        # Just send it to C++
        spatial_structures_native_functions.c_add_node_attributes(
            self.graph_ptr, attribute, ids, scores
        )

    def get_node_attributes(self, attribute: str) -> List[str]:
        """ Get scores of every node for a specific attribute

        Args:
            attribute : The unique key of the attribute to get scores for

        Returns:
            A list of strings representing the score of every node in the
            graph for attribute in order of ID. If attribute does not exist
            in the graph, then None is returned. For nodes that have never
            been assigned a score for a specific attribute, the score at
            the index of their ID will be None.


        Example:
            Create a graph, add some nodes, assign some attributes to its nodes
            then read them from the graph
           
           >>> from dhart.spatialstructures import Graph
           >>> # Create a simple graph with 3 nodes
           >>> g = Graph()
           >>> g.AddEdgeToGraph(0, 1, 100)
           >>> g.AddEdgeToGraph(0, 2, 50)
           >>> g.AddEdgeToGraph(1, 2, 20)
           >>> csr = g.CompressToCSR()

           >>> # Add node attributes to the simple graph
           >>> attr = "Test"
           >>> ids = [0, 1, 2]
           >>> scores = ["zero", "one", "two"]
           >>> g.add_node_attributes(attr, ids, scores)
           
           
           >>> # Get attribute scores from the graph
           >>> g.get_node_attributes(attr)
           ['zero', 'one', 'two']
        """

        return spatial_structures_native_functions.c_get_node_attributes(
            self.graph_ptr, attribute, self.NumNodes()
        )

    def clear_node_attribute(self, attribute: str):
        """ Clear a node attribute and all of its scores from the graph

        Args:
            attribute : The unique key of the attribute to delete from the graph
        
        Example:
            Adding an attribute, then clearing it from the graph
           >>> from dhart.spatialstructures import Graph
           >>> # Create a simple graph with 3 nodes
           >>> g = Graph()
           >>> g.AddEdgeToGraph(0, 1, 100)
           >>> g.AddEdgeToGraph(0, 2, 50)
           >>> g.AddEdgeToGraph(1, 2, 20)
           >>> csr = g.CompressToCSR()

           >>> # Add node attributes to the simple graph
           >>> attr = "Test"
           >>> ids = [0, 1, 2]
           >>> scores = ["zero", "one", "two"]
           >>> g.add_node_attributes(attr, ids, scores)
           
           >>> # Clear the attribute
           >>> g.clear_node_attribute(attr)

           >>> # Get attribute scores from the graph
           >>> g.get_node_attributes(attr)
           []
        
        """
        spatial_structures_native_functions.c_clear_node_attribute(
            self.graph_ptr, attribute
        )

    def attrs_to_costs(self, attribute_string: str, cost_string: str, direction: Direction):
        """Generate a cost set based on a set of node parameters

        Args:
            attribute_string (str): Attribute to create a new cost set from.]
            cost_string (str):  Key for the newly generated cost set. 
            direction (Direction): Direction to use for calculating the cost of any edge. For example INCOMING will use the cost of the node being traveled to by the edge.

        Raises:
            KeyError: attribute_string was not the key of any node attribute in the graph

        Examples:
           >>> from dhart.spatialstructures import Graph, Direction
           >>> g = Graph()
           >>> g.AddEdgeToGraph(0, 1, 100)
           >>> g.AddEdgeToGraph(0, 2, 50)
           >>> g.AddEdgeToGraph(1, 2, 20)
           >>> csr = g.CompressToCSR()
           >>> 
           >>> attr = "Test"
           >>> ids = [0, 1, 2]
           >>> scores = ["0", "100", "200"]
           >>> g.add_node_attributes(attr, ids, scores)
           >>> 
           >>> # Get attribute scores from the graph
           >>> out_attrs = g.get_node_attributes(attr)
           >>> 
           >>> g.attrs_to_costs(attr, "attr_cost", Direction.INCOMING)
           >>> 
           >>> g.GetEdgeCost(1, 2, "attr_cost")
           200.0

        """

        spatial_structures_native_functions.C_AttrsToCosts(
            self.graph_ptr, attribute_string, cost_string, direction
        )

