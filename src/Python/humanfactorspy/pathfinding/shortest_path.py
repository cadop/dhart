from . import Path
from . import pathfinder_native_functions
from humanfactorspy.Exceptions import OutOfRangeException
from typing import *
import numpy
from humanfactorspy.spatialstructures import Graph


def ConvertNodesToIds(graph: Graph, nodes: List[Union[Tuple, int]]):
    """ Get ids for nodes in graph. Raises exception if nodes can't be found 
    
    If integers are encountered, ignore them. If scalars are given,
    ignore them.
    
    """
    if not isinstance(nodes, List):
        nodes = [nodes]

    for node in nodes:
        if not (isinstance(node, int) or numpy.issubdtype(node, numpy.integer)):
            node = graph.GetNodeID(node)
            if node < 0:
                raise OutOfRangeException

    return nodes


def DijkstraShortestPath(
    graph: Graph,
    start: List[Union[int, Tuple[float, float, float]]],
    end: List[Union[int, Tuple[float, float, float]]],
) -> Union[List[Union[Path, None]], Union[Path, None]]:
    """ Find the shortest path from start to end using Dijkstra's algorithm
    
    Accepts a list of starting / ending points or single starting/ending
    points. 

    Args:
        Graph: A valid C++ Graph
        start: one or more Starting Node IDs
        end: one or more Ending Node IDs 
    
    Returns:
        List[Union[path, None]]: if multiple start/end ids were passed
        OR
        Union[Path, None]: If there was no path between A and B

    Raises:
        humanfactorspy.Exceptions.: Start or End did not exist in 
            the given graph OR start/end lists did not match in size.
    """

    graph.CompressToCSR()

    start = ConvertNodesToIds(graph, start)
    end = ConvertNodesToIds(graph, end)

    if len(start) != len(end):
        raise Exception("Start and End arrays didn't match in size!")

    # Convert tuples to ints if it matters and throw if those nodes
    # don't actually exist in the graph
    if len(start) == 1:
        res = pathfinder_native_functions.C_FindPath(graph.graph_ptr, start[0], end[0])
        if res:
            path_ptr, data_ptr, size = res
            return Path(path_ptr, data_ptr, size)
        else:
            return None
    else:
        res = pathfinder_native_functions.C_FindPaths(graph.graph_ptr, start, end)
        out_paths = [
            Path(result[0], result[1], result[2]) if result else None for result in res
        ]
        return out_paths
