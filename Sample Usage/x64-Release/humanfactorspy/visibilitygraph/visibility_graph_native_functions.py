from ctypes import *
from humanfactorspy.Exceptions import *
from typing import *

from humanfactorspy.common_native_functions import getDLLHandle, ConvertPointsToArray

HFPython: Union[
    None, CDLL
] = None  # The C++ DLL that contains all functionality we need

HFPython = getDLLHandle()


def C_VisibilityGraphAllToAll(
    bvh: c_void_p, nodes: Tuple[float, float, float], height: float
) -> c_void_p:
    """ Create a directed visibility graph in C++ """
    num_nodes = len(nodes)
    node_ptr = ConvertPointsToArray(nodes)
    graph_ptr = c_void_p(0)

    HFPython.CreateVisibilityGraphAllToAll(
        bvh, node_ptr, c_int(num_nodes), byref(graph_ptr), c_float(height)
    )
    return graph_ptr


def C_VisibilityGraphAlltoAllUndirected(
    bvh: c_void_p, nodes: Tuple[float, float, float], height: float, cores: int = 0
) -> c_void_p:
    """ Create an Undirected Visbility graph in C++

    Unlike the standard visibility graph algorithm, the Undirected visibility
    graph is able to be parallelized, and has a much lower memory footprint
    making it better suited to larger datasets.

    Args:
        bvh: the pointer to a valid embree bvh
        nodes: a list of nodes as (x,y,z) tuples
        height: height to evaluate the visibility graph from
        cores: number of cores to use in the evaluation. 0 will not be parallelized
    Returns:
        c_void_p : a pointer to the visibility graph in C++
    """

    num_nodes = len(nodes)
    node_ptr = ConvertPointsToArray(nodes)
    graph_ptr = c_void_p(0)

    HFPython.CreateVisibilityGraphAllToAllUndirected(
        bvh, node_ptr, c_int(num_nodes), byref(graph_ptr), c_float(height), c_int(cores)
    )
    return graph_ptr


def C_VisibilityGraphGroupToGroup(
    bvh: c_void_p,
    nodes_a: Tuple[float, float, float],
    nodes_b: Tuple[float, float, float],
    height: float,
    cores: int = 0,
) -> Union[c_void_p, None]:
    """ Create an Undirected Visbility graph in C++

        Returns none if no edges were produced
    """

    num_nodes_a = len(nodes_a)
    num_nodes_b = len(nodes_b)
    node_a_ptr = ConvertPointsToArray(nodes_a)
    node_b_ptr = ConvertPointsToArray(nodes_b)
    graph_ptr = c_void_p(0)

    res = HFPython.CreateVisibilityGraphGroupToGroup(
        bvh,
        node_a_ptr,
        c_int(num_nodes_a),
        node_b_ptr,
        c_int(num_nodes_b),
        byref(graph_ptr),
        c_float(height),
    )
    if res == HF_STATUS.OK:
        return graph_ptr
    else:
        return None
