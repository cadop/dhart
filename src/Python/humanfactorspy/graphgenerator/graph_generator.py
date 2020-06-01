import ctypes
from typing import *

from humanfactorspy.spatialstructures import Graph
from . import graph_generator_native_functions
from humanfactorspy.raytracer import EmbreeBVH


def GenerateGraph(
    bvh: EmbreeBVH,
    start_point: Tuple[float, float, float],
    spacing: Tuple[float, float, float],
    max_nodes: int,
    up_step: float = 0.197,
    up_slope: float = 20,
    down_step: float = 0.197,
    down_slope: float = 20,
    max_step_connections: int = 1,
    cores : int = -1
) -> Union[Graph, None]:
    """ Generate a new graph in C++

    All calculations assume that Z is up.

    Returns:
        If successful in creating a graph, meaning that 1) the initial starting check
        succeeded, and 2) atleast one other node could be generated to create an edge
        with that node, then the result will be returned. If unsuccessful then None
        will be returned.
    """
    pointer = graph_generator_native_functions.GenerateGraph(
        bvh.pointer,
        start_point,
        spacing,
        max_nodes,
        up_step,
        up_slope,
        down_step,
        down_slope,
        max_step_connections,
        cores
    )
    if pointer:
        return Graph(pointer)
    else:
        return None
