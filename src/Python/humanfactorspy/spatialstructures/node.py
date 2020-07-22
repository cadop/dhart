import ctypes
import numpy

from . import spatial_structures_native_functions
from humanfactorspy.native_numpy_like import NativeNumpyLike
from typing import Tuple

__all__ = ['NodeStruct','NodeList','CreateListOfNodeStructs']

class NodeStruct(ctypes.Structure):
    """ A point in space """

    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
        ("type", ctypes.c_int),
        ("id", ctypes.c_int),
    ]


class NodeList(NativeNumpyLike):
    """ A list of node structs from C++ """

    native_type = NodeStruct 
    delete_fp = spatial_structures_native_functions.DestroyNodes

    def __init__(self, vector_ptr: ctypes.c_void_p, data_ptr: ctypes.c_void_p):
        """ Construct a nodelist from a pointer to a vector of nodes and its underlying data

        Args:
            vector_ptr: a pointer to a vector of node structs
            data_ptr: a pointer to the underlying data of that vector
        """
        # Map numpy array to the vector's underlying data
        node_array_size = spatial_structures_native_functions.SizeOfNodeVector(
            vector_ptr
        )
        super().__init__(
            vector_ptr,
            data_ptr,
            node_array_size,
        )


def CreateListOfNodeStructs(points: Tuple[float, float, float]) -> numpy.array:
    """ Create an array of NodeStructs from a list of tuples. """
    arr = numpy.empty((len(points),), dtype=NodeStruct)
    for i in range(0, len(points)):
        arr[i][0] = points[i][0]
        arr[i][1] = points[i][1]
        arr[i][2] = points[i][2]
        arr[i][3] = 0
        arr[i][4] = -1

    return arr
