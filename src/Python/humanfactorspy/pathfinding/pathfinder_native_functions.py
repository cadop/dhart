from ctypes import *
from humanfactorspy.Exceptions import *
from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertFloatsToArray,
    ConvertIntsToArray,
    GetStringPtr,
)
from typing import *

HFPython = getDLLHandle()


def C_FindPath(
    graph_ptr: c_void_p, start: int, end: int, cost_type: str = ""
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
        OutOfRangeException: Start or End did not exist in 
            the given graph.
    """

    # Set up variables
    c_start = c_int(start)
    c_end = c_int(end)
    c_size = c_int(0)
    cost_str = GetStringPtr(cost_type)
    path_ptr = c_void_p(0)
    data_ptr = c_void_p(0)

    # Call into native code
    res = HFPython.CreatePath(
        graph_ptr,
        c_start,
        c_end,
        cost_str,
        byref(c_size),
        byref(path_ptr),
        byref(data_ptr),
    )

    # Check the result
    # OK means the path generated correctly, so return it
    if res == HF_STATUS.OK:
        return path_ptr, data_ptr, c_size.value

    # NO_PATH means no path could be found between start and end, so
    # return null
    elif res == HF_STATUS.NO_PATH:
        return None

    # NO_COST means that the specified cost_type was not found in the graph
    elif res == HF_STATUS.NO_COST:
        raise KeyError(f"The cost type {cost_type} does not exist in the graph!")

    # Getting here indicates programmer error. This function should account for all
    # error codes that can be thrown.
    else:
        assert False


def C_FindPaths(
    graph_ptr: c_void_p, start: List[int], end: List[int], cost_type: str = ""
) -> List[Tuple[c_void_p, c_void_p, int]]:
    """ Find multiple paths in C++ from start to end
    
    Args:
        graph_ptr: A pointer to a valid graph
        start: starting node id
        end: ending node id
        cost_type: The type of cost to use from the graph.
            If left blank, will use the cost that the
            graph was generated with. 

    Returns:
        c_void_p: Pointer to the Path object in C++
        c_void_p: Pointer to the Path's data in C++
        int: The size of the generated path
        OR
        None: If there was no path between A and B

    Raises:
        KeyError: cost_type didn't exist already in the graph
    """

    # Set up variables to match preconditions of this
    # Cinterface function
    num_paths = len(start)
    path_sizes_type = c_int * num_paths
    c_starts = ConvertIntsToArray(start)
    c_ends = ConvertIntsToArray(end)
    c_path_num = c_int(len(start))
    cost_str = GetStringPtr(cost_type)
    c_sizes = path_sizes_type()
    path_ptrs = (c_void_p * num_paths)()
    data_ptrs = (c_void_p * num_paths)()

    # Call into native code
    res = HFPython.CreatePaths(
        graph_ptr,
        c_starts,
        c_ends,
        cost_str,
        byref(path_ptrs),
        byref(data_ptrs),
        byref(c_sizes),
        c_path_num,
    )

    # If NO_COST is returned then we asked for a cost that
    # that the graph didn't have
    if res == HF_STATUS.NO_COST:
        raise KeyError(f"Tried to generate a path with non-existant cost {cost_type}.")

    # Read outputs. Replace paths that couldn't be generated
    # with None in the output array
    out_tuples = [
        out_tuple if out_tuple[2] != 0 else None
        for out_tuple in zip(path_ptrs, data_ptrs, c_sizes)
    ]

    # Return results
    return out_tuples


def C_DestroyPath(path_ptr: c_void_p) -> None:
    """ Delete a path in C++"""
    try:  # Sometimes the pointers need tobe converted to c_void_p again.
        HFPython.DestroyPath(path_ptr)
    except ArgumentError:
        HFPython.DestroyPath(c_void_p(path_ptr))
