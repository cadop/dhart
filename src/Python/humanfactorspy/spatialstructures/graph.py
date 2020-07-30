import ctypes
import numpy
from numpy.lib import recfunctions as rfn
from scipy.sparse import csr_matrix
from scipy.spatial.distance import cdist
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

    def CompressToCSR(self, cost_type: str='') -> csr_matrix:
        """ Compress the graph if needed then return a reference as a CSR matrix

        Parameters:
            cost_type : str, optional
                The cost type to use for constructing the CSR. Defaults to empty string.

            Returns:
                A scipy csr matrix

        """
        # convert string to bytes
        cost_type = cost_type.encode('utf-8')
        (
            nnz,
            row_count,
            col_count,
            data_ptr,
            inner_indices_ptr,
            outer_indices_ptr,
        ) = spatial_structures_native_functions.C_GetCSRPtrs(self.graph_ptr, cost_type)

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
        cost_type: str='',
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

        # convert string to bytes
        cost_type = cost_type.encode('utf-8')

        if isinstance(parent, int) and isinstance(child, int):
            spatial_structures_native_functions.C_AddEdgeFromNodeIDs(
                self.graph_ptr, parent, child, score, cost_type
            )
        else:
            spatial_structures_native_functions.C_AddEdgeFromNodes(
                self.graph_ptr, parent, child, score, cost_type
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

        >>> import matplotlib.pyplot as plt
        >>> import numpy as np
        >>> 
        >>> import humanfactorspy
        >>> from humanfactorspy.geometry import LoadOBJ, CommonRotations
        >>> from humanfactorspy.graphgenerator import GenerateGraph
        >>> from humanfactorspy.raytracer import EmbreeBVH
        >>> from humanfactorspy.pathfinding import DijkstraShortestPath
        >>> 
        >>> # Load BVH
        >>> obj_path = humanfactorspy.get_sample_model("energy_blob_zup.obj")
        >>> loaded_obj = LoadOBJ(obj_path)
        >>> embree_bvh = EmbreeBVH(loaded_obj)
        >>> 
        >>> # Set graph parameters 
        >>> start_point, spacing, max_nodes = (-30, 0, 20), (1, 1, 10), 100000
        >>> 
        >>> # Generate the graph
        >>> graph = GenerateGraph(embree_bvh, start_point, spacing, max_nodes)
        
        Check closest nodes using single x,y point

        >>> closest_node = graph.get_closest_nodes(np.array([30,0]), z=False)
        >>> print("Closest Node: ", closest_node)
        Closest Node:  1795

        Check closest nodes using multiple x,y points

        >>> closest_node = graph.get_closest_nodes(np.array([[30,0],[20,20]]), z=False)
        >>> print("Closest Node: ", closest_node)
        Closest Node:  [1795 2513]

        Check closest nodes using single x,y,z point

        >>> closest_node = graph.get_closest_nodes(np.array([30,0,0]))
        >>> print("Closest Node: ", closest_node)
        Closest Node:  1796

        Check closest nodes using multiple x,y,z points

        >>> closest_node = graph.get_closest_nodes(np.array([[30,0,0],[20,20,0]]))
        >>> print("Closest Node: ", closest_node)
        Closest Node:  [1796 2513]

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
        """ Get the closest point in the graph to the input set of points

        Todo
        ----

        Implement this

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

    def AggregateEdgeCosts(self, ct: CostAggregationType, directed: bool, cost_type: str='') -> EdgeSumArray:
        """ Get an aggregated score for every node in the graph based on
         its edges"""

        # convert string to bytes
        cost_type = cost_type.encode('utf-8')

        vector_ptr, data_ptr = spatial_structures_native_functions.C_AggregateEdgeCosts(
            self.graph_ptr, ct, directed, cost_type
        )
        return EdgeSumArray(vector_ptr, data_ptr, len(self.getNodes()))

    def __del__(self):
        if self.graph_ptr:
            spatial_structures_native_functions.DestroyGraph(self.graph_ptr)
