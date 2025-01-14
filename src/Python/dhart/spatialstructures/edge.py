import ctypes
import numpy
from . import spatial_structures_native_functions
from dhart.native_numpy_like import NativeNumpyLike
from typing import Tuple

__all__ = ['EdgeStruct', "EdgeList", "CreateListOfEdgeStructs"]

class EdgeStruct(ctypes.Structure):
    """ A connection between two points in space """
    
    _fields_ = [
        ("child", ctypes.c_int)
        ("weight", ctypes.c_float)
        # ("id", ctypes.c_int) # maybe add ids
    ]

class EdgeList(NativeNumpyLike):
    """ A list of edge structs from C++ """

    native_type = EdgeStruct
    delete_fp = spatial_structures_native_functions.DestroyEdges

    def __init__(self, vector_ptr: ctypes.c_void_p, data_ptr: ctypes.c_void_p):
        """ Construct an edgelist from a pointer to a vector of edges and its underlying data
        
        Args:
            vector_ptr: a pointer to a vector of edge structs
            data_ptr: a pointer to the underlying data of that vector
        """
        # Map numpy array to the vector's underlying data
        edge_array_size = spatial_structures_native_functions.SizeOfEdgeVector(vector_ptr)
        super().__init__(vector_ptr, data_ptr, edge_array_size)

def CreateListOfEdgeStructs(edges: Tuple[int, int]) -> numpy.array:
    """ Create an array of EdgeStructs from a list of tuples. """
    arr = numpy.empty((len(edges),), dtype = EdgeStruct)
    for idx in range(0, len(edges)):
        arr[idx][0] = edges[idx][1]
        arr[idx][1] = -1
        # arr[idx][2] = -1 # in case we add ids

    return arr