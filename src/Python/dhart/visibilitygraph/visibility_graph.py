from typing import *

from dhart.raytracer import EmbreeBVH
from dhart.spatialstructures import Graph

from . import visibility_graph_native_functions

__all__ = ['VisibilityGraphAllToAll','VisibilityGraphUndirectedAllToAll','VisibilityGraphGroupToGroup']

def VisibilityGraphAllToAll(
    bvh: EmbreeBVH, nodes: List[Tuple[float, float, float]], height: float
) -> Graph:

    """ Construct a directed visibility graph using all nodes in nodes

    O(n^3) complexity in both space and time. Should only be used with smaller
    datasets, otherwise running out of memory is likely.

    Args:
        bvh: A pointer to a valid BVH object. 
        nodes: a list of nodes,

    
    Example:
        Create a visibility graph between 3 nodes
        
        >>> from dhart.geometry import LoadOBJ, CommonRotations, ConstructPlane
        >>> from dhart.spatialstructures.graph import CostAggregationType
        >>> from dhart.raytracer import EmbreeBVH
        >>> from dhart.visibilitygraph import VisibilityGraphAllToAll

        >>> loaded_obj = ConstructPlane()
        >>> loaded_obj.Rotate(CommonRotations.Yup_to_Zup)
        >>> bvh = EmbreeBVH(loaded_obj)

        >>> # The model is a flat plane, so only nodes 0,2 should connect.
        >>> points = [(0,0,1), (0,0,-10), (0,2,0)]
        >>> height = 1.7
        >>> VG = VisibilityGraphAllToAll(bvh, points, height)

        >>> print(VG.AggregateEdgeCosts(CostAggregationType.SUM, True))
        [ 2.236068 10.198039 12.434107]


    """
    graph_ptr = visibility_graph_native_functions.C_VisibilityGraphAllToAll(
        bvh.pointer, nodes, height
    )
    return Graph(graph_ptr)


def VisibilityGraphUndirectedAllToAll(
    bvh: EmbreeBVH,
    nodes: List[Tuple[float, float, float]],
    height: float,
    cores: int = -1,
) -> Graph:
    """ Construct visibility graph using all nodes in nodes

    Unlike the standard visibility graph algorithm, the Undirected visibility
    graph is able to be parallelized, making it better suited to larger datasets.

    Arguments:
        bvh: the pointer to a valid embree bvh
        nodes: a list of nodes as (x,y,z) tuples
        height: height to evaluate the visibility graph from
        cores: number of cores to use in the evaluation. 0 will not be parallelized

    Example:
        Create a visibility graph between 3 nodes
        

        >>> from dhart.geometry import LoadOBJ, CommonRotations, ConstructPlane
        >>> from dhart.spatialstructures.graph import CostAggregationType
        >>> from dhart.raytracer import EmbreeBVH
        >>> from dhart.visibilitygraph import VisibilityGraphUndirectedAllToAll

        >>> loaded_obj = ConstructPlane()
        >>> loaded_obj.Rotate(CommonRotations.Yup_to_Zup)
        >>> bvh = EmbreeBVH(loaded_obj)

        >>> points = [(0,0,1), (0,0,-10), (0,2,0)]
        >>> height = 1.7
        >>> VG = VisibilityGraphUndirectedAllToAll(bvh,points,height)

        >>> print(VG.AggregateEdgeCosts(CostAggregationType.SUM, True))
        [ 2.236068 10.198039  0.      ]

    Returns:
        Graph : An undirected graph created in C++
     """
    graph_ptr = visibility_graph_native_functions.C_VisibilityGraphAlltoAllUndirected(
        bvh.pointer, nodes, height, cores
    )
    return Graph(graph_ptr)

    
def VisibilityGraphGroupToGroup(
    bvh: EmbreeBVH,
    group_a: List[Tuple[float, float, float]],
    group_b: List[Tuple[float, float, float]],
    height: float,
    cores: int = -1,
) -> Union[Graph, None]:
    """ Construct visibility graph between all nodes in group_a, and group_b

    Note: Node ids will be assigned first first to group_a, then to group_b.
    I.E if group_a has three nodes, and group_b has four, A's nodes would be
    [1,2,3] and group_b's would be [4,5,6,7]

    Arguments:
        bvh: the pointer to a valid embree bvh
        group_a: a list of nodes as (x,y,z) tuples
        group_b: a list of nodes as (x,y,z) tuples
        height: height to evaluate the visibility graph from
        cores: number of cores to use in the evaluation. 0 will not be parallelized
    
    Examples:
        Create a new visibility graph from one group of nodes to another group of nodes
        then print a summary of their scores
        
        >>> from dhart.geometry import LoadOBJ, CommonRotations, ConstructPlane
        >>> from dhart.spatialstructures.graph import CostAggregationType
        >>> from dhart.raytracer import EmbreeBVH
        >>> from dhart.visibilitygraph import VisibilityGraphGroupToGroup

        >>> loaded_obj = ConstructPlane()
        >>> loaded_obj.Rotate(rotation=CommonRotations.Yup_to_Zup)
        >>> bvh = EmbreeBVH(loaded_obj)

        >>> nodes_a = [(1,1,1), (1,2,0)]
        >>> nodes_b = [(2,1,1), (1,1,2)]

        >>> graph = VisibilityGraphGroupToGroup(bvh, nodes_a, nodes_b, 1.7, -1)
        >>> print(graph.AggregateEdgeCosts(CostAggregationType.AVERAGE, True))
        [1.        1.9840593 0.        0.       ]
        
    Returns:
        Graph : An undirected graph created in C++
        None: If the given inputs produced a graph with no edges
     """
    graph_ptr = visibility_graph_native_functions.C_VisibilityGraphGroupToGroup(
        bvh.pointer, group_a, group_b, height, cores
    )
    if graph_ptr:
        return Graph(graph_ptr)
    else:
        return None
