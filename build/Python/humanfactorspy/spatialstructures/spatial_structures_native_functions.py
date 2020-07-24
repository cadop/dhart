from ctypes import *
from humanfactorspy.Exceptions import *
from typing import *

from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
)

HFPython = getDLLHandle()


def SizeOfNodeVector(node_vector_ptr: c_void_p) -> int:
    """ Get the size of a node vector """
    size = c_int(0)
    HFPython.GetSizeOfNodeVector(node_vector_ptr, byref(size))
    return size.value


def GetEdgesForNode(
    graph_ptr: c_void_p, node_ptr: c_void_p
    ) -> Tuple[c_void_p, c_void_p]:

    """ *** UNIMPLEMENTED *** 
    
    Get a list of nodes from a graph that belong to the specified node
    """
    pass

def C_AggregateEdgeCosts(
    graph_ptr: c_void_p,
    aggregate_type: int,
    directed: bool
    ) -> c_void_p:

    vector_ptr = c_void_p(0)
    data_ptr = c_void_p(0)

    HFPython.AggregateCosts(
        graph_ptr,
        c_int(aggregate_type),
        c_bool(directed),
        byref(vector_ptr),
        byref(data_ptr)
    )

    return vector_ptr, data_ptr


def GetNodesFromGraph(graph_ptr: c_void_p) -> Tuple[c_void_p, c_void_p]:
    """ Get a list of nodes from from the graph pointed to by graph_ptr

    Returns:
        Tuple[c_void_p, c_void_p] :a pointer to a vector of nodes then a pointer to 
            said vector's underlying data.
    """
    vector_ptr = c_void_p(0)
    data_ptr = c_void_p(0)
    HFPython.GetAllNodesFromGraph(graph_ptr, byref(vector_ptr), byref(data_ptr))

    return vector_ptr, data_ptr


def C_CreateGraph(nodes: Union[List[Tuple[float, float, float]], None]) -> c_void_p:
    """ Create a new graph in C++
    
    Args:
        nodes: A list of nodes to pre-insert into the graph or nothing
    Returns:
        c_void_p: A pointer to the underlying graph object in C++
    """
    if nodes:
        num_nodes = len(nodes)
        node_float_ptr = ConvertPointsToArray(nodes)
    else:
        node_float_ptr = c_void_p()
        num_nodes = 0

    graph_ptr = c_void_p()
    HFPython.CreateGraph(node_float_ptr, c_int(num_nodes), byref(graph_ptr))

    return graph_ptr


def C_AddEdgeFromNodes(
    graph_ptr: c_void_p,
    parent: Tuple[float, float, float],
    child: Tuple[float, float, float],
    score: float,
    ) -> None:

    """ Add a new edge to the graph """

    parent_ptr = ConvertPointsToArray(parent)
    child_ptr = ConvertPointsToArray(child)

    HFPython.AddEdgeFromNodes(graph_ptr, parent_ptr, child_ptr, c_float(score))


def C_AddEdgeFromNodeIDs(
    graph_ptr: c_void_p, parent_id: int, child_id: int, score: float
    ) -> None:
    
    HFPython.AddEdgeFromNodeIDs(
        graph_ptr, c_int(parent_id), c_int(child_id), c_float(score)
    )


def C_GetCSRPtrs(
    graph_ptr: c_void_p,
    ) -> Tuple[int, int, int, c_void_p, c_void_p, c_void_p]:
    """ Get the information necessary to map a numpy CSR to the C++ graph
        
    Returns:
        int: Number of non-zeros for the csr
        int: Number of rows in the graph
        int: Number of columns in the graph
        c_void_p: Pointer to the data of the graph
        c_void_p: Pointer to the inner_indices of the graph
        c_void_p: Pointer to the outer_indices of the graph
    """
    nnz = c_int(0)
    num_cols = c_int(0)
    num_rows = c_int(0)

    data_ptr = c_void_p(0)
    inner_indices_ptr = c_void_p(0)
    outer_indices_ptr = c_void_p(0)

    HFPython.GetCSRPointers(
        graph_ptr,
        byref(nnz),
        byref(num_rows),
        byref(num_cols),
        byref(data_ptr),
        byref(inner_indices_ptr),
        byref(outer_indices_ptr),
    )

    return (
        nnz.value,
        num_rows.value,
        num_cols.value,
        data_ptr,
        inner_indices_ptr,
        outer_indices_ptr,
    )


def C_GetNodeID(graph_ptr: c_void_p, node: Tuple[float, float, float]) -> int:
    """ Get the id of node for the graph at graph_ptr """
    return_int = c_int()
    node_ptr = ConvertPointsToArray(node)
    HFPython.GetNodeID(graph_ptr, node_ptr, byref(return_int))

    return return_int


def C_Compress(graph_ptr: c_void_p) -> None:
    HFPython.Compress(graph_ptr)


def C_ClearGraph(graph_ptr: c_void_p) -> None:
    HFPython.ClearGraph(graph_ptr)


### Destructors

def C_DeleteFloatArray(float_ptr: c_void_p):
    HFPython.DestroyFloatVector(float_ptr)

def DestroyGraph(graph_ptr: c_void_p):
    """ Call the destructor for a list of nodes """
    HFPython.DestroyGraph(graph_ptr)


def DestroyNodes(node_list_ptr: c_void_p):
    """ Call the destructor for a list of nodes """
    HFPython.DestroyNodes(node_list_ptr)
