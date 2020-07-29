import ctypes
import numpy
from numpy.lib import recfunctions as rfn
from scipy.sparse import csr_matrix
from ctypes import c_float, c_int, c_void_p
from typing import *
from enum import IntEnum

from humanfactorspy.native_numpy_like import NativeNumpyLike
from .node import NodeStruct, NodeList
from . import spatial_structures_native_functions

__all__ = ['CostAggregationType','EdgeSumArray','Graph']

class CostAggregationType(IntEnum):
    SUM = 0
    AVERAGE = 1
    COUNT = 2


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

        Parameters:
            cost_type : str, optional
                The cost type to use for constructing the CSR. Defaults to empty string.

            Returns:
                A scipy csr matrix
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
            parent: Either a node with values for x,y, and z, or an integer
            representing a node ID
            child: Where the edge from parent is going to. Must be the same type
            of parent i.e. if parent is an int, then the child must also be an 
            int
            cost: the cost from parent to child
            cost_type: The type of cost to add this edge to. If left blank or 
            as the empty string then the edge will be added to the 
            graph's default cost set.
        
        Pre Conditions:
            1) If cost_type is not left blank, then the edge from parent to 
               child must already exist in the graph for its default cost type.
            2) If adding costs to an alternate cost type, then the graph must
               already be compressed

        Post Conditions:
            1) If parent or child doesn't exist in the graph as nodes, they will 
               be added
            2) If cost_type doesn't already exist in the graph it will be 
               created

      .. note::
          The graph offers some basic functionality to add edges and nodes but 
          it's main use is to provide access to the output of the GraphGenerator
          and VisibilityGraph. If adding edges or alternate cost types please
          make sure to read the documentation for these functions and that 
          all preconditions are followed.

      .. warning::
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

    def get_node_points(self):
        """
        Get the nodes of the graph as xyz values in a numpy array by defining the array name
        view to be used as the x,y,z values. It uses the `structured_to_unstructured` 
        method of numpy to perform the conversion.

        Parameters
        ----------

        None

        Returns
        -------

        ndarray[n x 3]
            where n is the number of nodes in the graph and 3 is an x,y,z point

        Examples
        --------

        >>> 
        >>> 

        """

        # first get the nodes
        nodes = self.getNodes()
        # extract the array names of the xyz values
        node_xyz = nodes.array[['x','y','z']]
        # convert to an unstructured array to be seen like normal numpy array
        xyz_array = rfn.structured_to_unstructured(node_xyz)

        return node_point_array

    def Clear(self):
        """ Clear all edges/nodes from the graph """
        spatial_structures_native_functions.C_ClearGraph(self.graph_ptr)

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
         its edges"""

        vector_ptr, data_ptr = spatial_structures_native_functions.C_AggregateEdgeCosts(
            self.graph_ptr, ct, directed, cost_type)
        return EdgeSumArray(vector_ptr, data_ptr, len(self.getNodes()))

    def __del__(self):
        if self.graph_ptr:
            spatial_structures_native_functions.DestroyGraph(self.graph_ptr)

    def GetEdgeCost(self, parent: int, child: int, cost_type: str = ""):
        return spatial_structures_native_functions.C_GetEdgeCost(
            self.graph_ptr,
            parent,
            child,
            cost_type
            )
