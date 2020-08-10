"""
A collection of NativeNumpyLike instantiations that don't exclusively belong
in any one namespace.


Note:
    Unlike every other implementation of nativenumpylikes, these classes
    contain their own delete funciton pointers. This is just to keep
    things concise for these extremely simple implementations. 

    Generally, you'd want to make distinct delet functions for the
    sake of clarity. 
"""

from ctypes import c_int, c_float, c_void_p
from typing import *
from humanfactorspy.common_native_functions import getDLLHandle
from humanfactorspy.native_numpy_like import NativeNumpyLike

__all__ = ["FloatArray2D", "IntArray2D"]

# Include the native functions here
HFPython = getDLLHandle()


def DeleteFloatArrayC(pointer : c_void_p):
    HfPython.DeleteFloatArray(pointer)


def DeleteIntArrayC(pointer : c_void_p):
    HFPython.DeleteIntArray(pointer)


class FloatArray2D(NativeNumpyLike):
    """ A generic instantiantion of a view of a float array in C++ """

    native_type = c_float
    delete_fp = DeleteFloatArrayC

    def __init__(self, vector_ptr: c_void_p,
                 data_ptr: c_void_p, size: Tuple[int, int]):
        super().__init__(vector_ptr, data_ptr, size)


class IntArray2D(NativeNumpyLike):
    """ A generic instantiantion of a view of an int array in C++ """

    native_type = c_int
    delete_fp = DeleteIntArrayC

    def __init__(self, vector_ptr: c_void_p,
                 data_ptr: c_void_p, size: Tuple[int, int]):
        super().__init__(vector_ptr, data_ptr, size)


