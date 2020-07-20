from ctypes import *
from humanfactorspy.Exceptions import *
from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertFloatsToArray,
    ConvertIntsToArray,
)
from typing import *

HFPython = getDLLHandle()


def C_FindPath(
    graph_ptr: c_void_p, start: int, end: int
) -> Union[Tuple[c_void_p, c_void_p, int], None]:
    """ Find a path in C++ from start to end 
    
    Args:
        graph_ptr: A pointer to a valid graph
        start: starting node id
        end: ending node id
    
    Returns:
        c_void_p: Pointer to the Path object in C++
        c_void_p: Pointer to the Path's data in C++
        int: The size of the generated path
        OR
        None: If there was no path between A and B

    Raises:
        humanfactorspy.Exceptions.OUT_OF_RANGE: Start or End did not exist in 
            the given graph.
    """

    c_start = c_int(start)
    c_end = c_int(end)

    c_size = c_int(0)
    path_ptr = c_void_p(0)
    data_ptr = c_void_p(0)

    res = HFPython.CreatePath(
        graph_ptr, c_start, c_end, byref(c_size), byref(path_ptr), byref(data_ptr)
    )

    if res == HF_STATUS.NO_PATH:
        return None
    elif res == HF_STATUS.OUT_OF_RANGE:
        raise OutOfRangeException

    return path_ptr, data_ptr, c_size.value


def C_FindPaths(
    graph_ptr: c_void_p, start: List[int], end: List[int],
) -> List[Tuple[c_void_p, c_void_p, int]]:
    num_paths = len(start)
    path_sizes_type = c_int * num_paths

    c_starts = ConvertIntsToArray(start)
    c_ends = ConvertIntsToArray(end)
    c_path_num = c_int(len(start))
    c_sizes = path_sizes_type()
    path_ptrs = (c_void_p * num_paths)()
    data_ptrs = (c_void_p * num_paths)()

    res = HFPython.CreatePaths(
        graph_ptr,
        c_starts,
        c_ends,
        byref(path_ptrs),
        byref(data_ptrs),
        byref(c_sizes),
        c_path_num,
    )

    # Read output sizes

    out_tuples = [
        out_tuple if out_tuple[2] != 0 else None
        for out_tuple in zip(path_ptrs, data_ptrs, c_sizes)
    ]

    return out_tuples


def C_DestroyPath(path_ptr: c_void_p) -> None:
    """ Delete a path in C++"""
    try:  # Sometimes the pointers need tobe converted to c_void_p again.
        HFPython.DestroyPath(path_ptr)
    except ArgumentError:
        HFPython.DestroyPath(c_void_p(path_ptr))
