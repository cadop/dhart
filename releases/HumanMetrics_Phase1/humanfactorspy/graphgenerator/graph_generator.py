""" 

"""

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
    """Generate a graph of accessible space. If no graph can be generated, null will be returned. 

    Args:
        bvh (EmbreeBVH): Geometry to use for graph generation. The mesh used
            to generate the BVH must have been Z-up.
        start_point (Tuple[float, float, float]): Starting point for the graph.
            If this isn't above solid ground, the graph cannot be generated.
        spacing (Tuple[float, float, float]): Space between nodes.
             Lower values will yield more nodes for a higher resolution graph. 
        max_nodes (int): The maximum amount of nodes to generate. Note:
            This only counts nodes that have had their children calculated
            so the actual number of nodes may be higher.
        up_step (float, optional): Maximum height of a step the graph can traverse.
             Any steps higher this will be considered inaccessible. Defaults to 0.197.
        up_slope (float, optional):  Maximum upward slope the graph can traverse in 
            degrees. Any slopes steeper than this will be considered inaccessible.
        down_step (float, optional): Maximum step down the graph can traverse. 
            Any steps steeper than this will be considered inaccessible.
        down_slope (float, optional): The maximum downward slope the graph 
            can traverse. Any slopes steeper than this will be considered inaccessible. 
        max_step_connections (int, optional): Multiplier for number of children to
            generate for each node. Increasing this value will increase the number of 
            edges in the graph, and as a result the amount of memory the algorithm requires.
        cores (int, optional):  Number of cores to use. -1 will use all available cores, 
            and 0 will run a serialized version of the algorithm.

    Returns:
        Union[Graph, None]: If the graph fails to generate with these parameters, return none
            otherwise return the new graph

    Examples:
        Generate a graph on the example plane, then print its nodes
        
        >>> from humanfactorspy.geometry import LoadOBJ, MeshInfo, CommonRotations
        >>> from humanfactorspy.raytracer import EmbreeBVH  
        >>> from humanfactorspy.geometry.mesh_info import ConstructPlane
        >>> from humanfactorspy.graphgenerator import GenerateGraph

        >>> MI = ConstructPlane()
        >>> MI.Rotate(CommonRotations.Zup_to_Yup)
        >>> BVH = EmbreeBVH(MI)
        >>> graph = GenerateGraph(BVH, (0,0,1), (1,1,1), 2)
        >>> for node in graph.getNodes():
        ...     print(f"({node[0]}, {node[1]}, {node[2]})")
        (0.0, 0.0, 5.960464477539063e-08)
        (-1.0, -1.0, 1.1920928955078125e-07)
        (-1.0, 0.0, 5.960464477539063e-08)
        (-1.0, 1.0, -1.1920928955078125e-07)
        (0.0, 1.0, -1.1920928955078125e-07)
        (1.0, 1.0, -1.1920928955078125e-07)
        (-2.0, -2.0, 2.384185791015625e-07)
        (-2.0, -1.0, 1.1920928955078125e-07)
        (-2.0, 0.0, 5.960464477539063e-08)
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
