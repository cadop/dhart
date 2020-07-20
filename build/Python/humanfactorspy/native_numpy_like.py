import numpy
import ctypes
from typing import *


class NativeNumpyLike(object):
    """ A wrapper for a native C++ vector that maps to a numpy array

    Upon garbage collection, the memory allocated in C++ is automatically 
    freed. Not intended to be instantiated on its own

    Attributes:
        array: A numpy array that can be used to access the underlying C++
            vector's data. 
     """

    def __init__(
        self,
        vector_ptr: ctypes.c_void_p,
        data_ptr: ctypes.c_void_p,
        size: Union[int, Tuple],
    ):
        """ Create a new numpy like instance

        Args:
            vector_ptr: A pointer to the C++ vector holding the information
            data_ptr: A pointer to the underlying data of the vector. This
                is what is used for the numpy buffer.
            size: The shape of the array. I.E. 100 would be a flat 100 element
                array, (100, 200) would be a 2dimensional array, etc.
         """
        self.vector_pointer = vector_ptr
        self.data_pointer = data_ptr

        # Convert size to a tuple if only an int was specified
        if isinstance(size, int):
            self.size = (size,)
        else:
            self.size = size

        # Map numpy array to the vector's underlying data
        np_arr_ptr = ctypes.cast(data_ptr, ctypes.POINTER(self.native_type))
        self.array = numpy.ctypeslib.as_array(np_arr_ptr, shape=self.size)

    def GetPointers(self) -> Tuple[ctypes.c_void_p, ctypes.c_void_p]:
        """ Return the underlying vector and data pointers. 

        Returns:
            Tuple[c_void_p, c_void_p]: vector pointer and data pointer respectively
        """
        return (self.vector_pointer, self.data_pointer)

    def __getitem__(self, key):
        return self.array[key]

    def __len__(self):
        return self.size[0]

    def __str__(self):
        return str(self.array)

    def __del__(self):
        if self.vector_pointer:
            self.delete_fp.__get__(self.vector_pointer)