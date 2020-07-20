from ctypes import c_void_p, c_float
import ctypes
import numpy

from . import viewanalysis_native_functions
from humanfactorspy.native_numpy_like import NativeNumpyLike

class ViewAnalysisAggregates(NativeNumpyLike):  # Should be a native numpy like
    """ A set of results  returned from view analysis, with a single value for each node """

    native_type = c_float 
    delete_fp = viewanalysis_native_functions.C_DestroyScores

    def __init__(
        self, vector_ptr: ctypes.c_void_p, data_ptr: ctypes.c_void_p, size: int
    ):
        """ Create a new view analysis result from view analysis in C++

        Args:
            vector_ptr: a pointer to a vector of floats
            data_ptr: a pointer to the underlying data of the node vector
            size: the size of the float vector
        """
        
        super().__init__(
            vector_ptr,
            data_ptr,
            size,
        )

class ViewAnalysisDirections(NativeNumpyLike):  # Should be a native numpy like
    """ A list of directions returned from C++ """

    native_type = c_float
    delete_fp = viewanalysis_native_functions.C_DestroyScores

    def __init__(
        self, vector_ptr : ctypes.c_void_p, data_ptr: ctypes.c_void_p, size:int
    ):
        super().__init__(
            vector_ptr,
            data_ptr,
            (size,3)
        )


