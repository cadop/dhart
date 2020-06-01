import ctypes
import numpy
from scipy.sparse import csr_matrix
from ctypes import c_float, c_int, c_void_p
from typing import *
from enum import IntEnum

from humanfactorspy.native_numpy_like import NativeNumpyLike
from .node import NodeStruct, NodeList
from . import spatial_structures_native_functions


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
    """ A wrapper for the human factors graph in C++

    Provides a view of the graph through a numpy csr and manages its 
    lifetime.

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

    def CompressToCSR(self) -> csr_matrix:
        """ Compress the graph if needed then return a reference as a CSR matrix

            Returns:
                A scipy csr matrix
        """
        (
            nnz,
            row_count,
            col_count,
            data_ptr,
            inner_indices_ptr,
            outer_indices_ptr,
        ) = spatial_structures_native_functions.C_GetCSRPtrs(self.graph_ptr)

        self.nnz = nnz
        self.row_count = row_count
        self.col_count = col_count

        data_nparr_pointer = ctypes.cast(data_ptr, ctypes.POINTER(c_float))
        outer_nparr_pointer = ctypes.cast(outer_indices_ptr, ctypes.POINTER(c_int))
        inner_nparr_pointer = ctypes.cast(inner_indices_ptr, ctypes.POINTER(c_int))

        self.__data_array = numpy.ctypeslib.as_array(data_nparr_pointer, shape=(nnz,))
        self.__outer__indices_array = numpy.ctypeslib.as_array(
            outer_nparr_pointer, shape=(row_count + 1,)
        )
        self.__inner__indices_array = numpy.ctypeslib.as_array(
            inner_nparr_pointer, shape=(nnz,)
        )

        self.csr = csr_matrix(
            (
                self.__data_array,
                self.__inner__indices_array,
                self.__outer__indices_array,
            ),
            (row_count, col_count),
            copy=False,
        )
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
        score: float,
    ) -> None:
        """ Add a new edge to the graph from parent to child with the given cost

        NOTE: 
            1) This will invalidate the current CSR representation of the graph
            2) Not supported for visibility graphs

        Args:
            parent: Either a node with values for x,y, and z, or an integer representing a node ID
            child: Where the edge from parent is going to. Must be the same type of parent
                i.e. if parent is an int, then the child must also be an int
            cost: the cost from parent to child
        """

        if isinstance(parent, int) and isinstance(child, int):
            spatial_structures_native_functions.C_AddEdgeFromNodeIDs(
                self.graph_ptr, parent, child, score
            )
        else:
            spatial_structures_native_functions.C_AddEdgeFromNodes(
                self.graph_ptr, parent, child, score
            )

    def getNodes(self) -> NodeList:
        """ Get a list of nodes from the graph as a nodelist """
        ret = spatial_structures_native_functions.GetNodesFromGraph(self.graph_ptr)
        return NodeList(ret[0], ret[1])

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
        self, ct: CostAggregationType, directed: bool
    ) -> EdgeSumArray:
        """ Get an aggregated score for every node in the graph based on
         its edges"""

        vector_ptr, data_ptr = spatial_structures_native_functions.C_AggregateEdgeCosts(
            self.graph_ptr, ct, directed
        )
        return EdgeSumArray(vector_ptr, data_ptr, len(self.getNodes()))

    # def __get__(this, node: Node.Node) -> Node.NodeList:
    #   """ Get a list of edges for the given node  UNIMPLEMENTED"""
    #    pass

    def __del__(self):
        if self.graph_ptr:
            spatial_structures_native_functions.DestroyGraph(self.graph_ptr)
