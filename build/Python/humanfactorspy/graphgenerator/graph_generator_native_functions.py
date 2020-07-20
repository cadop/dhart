from ctypes import *
from humanfactorspy.Exceptions import *

from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
    GetStringPtr,
)
from typing import *

HFPython = getDLLHandle()


def GenerateGraph(
    rt_ptr: c_void_p,
    start_point,
    spacing,
    max_nodes: int,
    up_step: float,
    up_slope: float,
    down_step: float,
    down_slope: float,
    max_step_connections: int,
    cores : int = -1
) -> Union[c_void_p, None]:
    """ Generate the graph in C++ with a raytracer from CreateRaytracer

    Returns:
        If the graph creation was successful, return a pointer to the graph.
        Otherwise, return None.
    """

    # Convert the float tuples to c arrays
    point = c_float * 3
    spacing_as_point = point()
    start_as_point = point()

    start_as_point[0] = c_float(start_point[0])
    start_as_point[1] = c_float(start_point[1])
    start_as_point[2] = c_float(start_point[2])
    spacing_as_point[0] = c_float(spacing[0])
    spacing_as_point[1] = c_float(spacing[1])
    spacing_as_point[2] = c_float(spacing[2])

    graph_ptr = c_void_p(0)
    error_code = HFPython.GenerateGraph(
        rt_ptr,
        byref(start_as_point),
        byref(spacing_as_point),
        max_nodes,
        c_float(up_step),
        c_float(up_slope),
        c_float(down_step),
        c_float(down_slope),
        max_step_connections,
        c_int(cores),
        byref(graph_ptr)
    )

    if error_code == HF_STATUS.OK:
        return graph_ptr
    elif error_code == HF_STATUS.NO_GRAPH:
        return None
    