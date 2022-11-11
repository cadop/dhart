from ctypes import *
from dhart.Exceptions import *

from dhart.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
    GetStringPtr,
    ConvertIntsToArray
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
    min_connections: int,
    cores : int = -1,
    obstacle_geometry: List[int] = [],
    walkable_geometry: List[int] = []
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

    # Determine how many elements are in either array.
    num_walkables = len(walkable_geometry)
    num_obstacles = len(obstacle_geometry)
    has_geometry_ids = num_walkables > 0 or num_obstacles > 0
    
    # If this doesn't have geometry IDs sepecified, then we can just call
    # the basic generate graph function
    if not has_geometry_ids:
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
            min_connections,
            c_int(cores),
            byref(graph_ptr)
        )
    else:
        # If we have geometry ids, then convert their arrays to c_style integer
        # arrays and call the Obstacles version
        walkable_array = ConvertIntsToArray(walkable_geometry)
        obstacle_array = ConvertIntsToArray(obstacle_geometry)

        error_code = HFPython.GenerateGraphObstacles(
            rt_ptr,
            byref(start_as_point),
            byref(spacing_as_point),
            max_nodes,
            c_float(up_step),
            c_float(up_slope),
            c_float(down_step),
            c_float(down_slope),
            max_step_connections,
            min_connections,
            c_int(cores),
            obstacle_array,
            walkable_array,
            c_int(num_obstacles),
            c_int(num_walkables),
            byref(graph_ptr)
        )
    
    # If no graph could be generated using these settings
    # return null, otherwise return the ponter to the new graph
    if error_code == HF_STATUS.OK:
        return graph_ptr
    elif error_code == HF_STATUS.NO_GRAPH:
        return None
    