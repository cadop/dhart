from humanfactorspy.native_numpy_like import NativeNumpyLike
import ctypes
from . import pathfinder_native_functions


class PathMemberStruct(ctypes.Structure):
    """ A node in a path """

    _fields_ = [
        ("cost_to_next", ctypes.c_float),
        ("id", ctypes.c_int),
    ]


class Path(NativeNumpyLike):
    """ A list of path members from C++ """

    native_type = PathMemberStruct
    delete_fp = pathfinder_native_functions.C_DestroyPath

    def __init__(self, path_ptr: ctypes.c_void_p, data_ptr: ctypes.c_void_p, size: int):
        """ Construct a path from a pointer to a path of nodes and its underlying path_members

        Args:
            path_ptr: a pointer to a C++ path object
            data_ptr: a pointer to the underlying data of that path object
            size: the size of the given path
        """
        # Map numpy array to the vector's underlying data

        super().__init__(
            path_ptr,
            data_ptr,
            size,
        )
