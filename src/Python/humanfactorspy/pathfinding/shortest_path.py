from . import Path
from . import pathfinder_native_functions
from humanfactorspy.Exceptions import OutOfRangeException
from typing import *
import numpy
from humanfactorspy.spatialstructures import Graph

__all__ = ["ConvertNodesToIds", "DijkstraShortestPath"]


def ConvertNodesToIds(graph: Graph, nodes: List[Union[Tuple, int]]) -> List[int]:
    """ Get ids for nodes in graph. Raises exception if nodes can't be found 
    
    If integers are encountered, ignore them. If scalars are given,
    ignore them.

    Args:
        Graph: Graph to use for getting IDs from nodes
        nodes: Nodes to get the ids of. If any node in nodes already
            is an ID, then it's ignored. 

    Returns:
        A list of ids for every node in nodes
    
    Raises:
        OutOfRangeException: One or more of the nodes in nodes did not match
            a valid node in Graph. 
    """

    # If nodes is a single element, make it a list.
    if not isinstance(nodes, List):
        nodes = [nodes]

    # Iterate through every member nodes
    for node in nodes:

        # If this element isn't an int, then we must convert it to one
        if not (isinstance(node, int) or numpy.issubdtype(node, numpy.integer)):
            node = graph.GetNodeID(node)
            if node < 0:
                raise OutOfRangeException

    return nodes


def DijkstraShortestPath(
    graph: Graph,
    start: List[Union[int, Tuple[float, float, float]]],
    end: List[Union[int, Tuple[float, float, float]]],
    cost_type: str = "",
) -> Union[List[Union[Path, None]], Union[Path, None]]:
    """ Find the shortest path from start to end using Dijkstra's shortest path algorithm
    
    Accepts a list of starting / ending points or single starting/ending
    points. 

    Args:
        Graph : A valid C++ Graph
        start : one or more Starting Node IDs
        end : one or more Ending Node IDs 
        cost_type : Which cost to use for path generation. If no cost type is specified,
            then the graph's default cost type will be used. If a cost type is specified
            then it must already exist in the graph.
    
    Returns:
        List[Union[path, None]]:
            if multiple start/end ids were passed
        Union[Path, None]:
             One start or end point was passed

        If a path cannot be found to connect a start and end point that
        path will be returned as None.
    
    Preconditions:
        1) If using multiple paths, The length of start_ids must match
           the length of end_ids. 
        2) Each node in start_nodes and end_nodes must contain the x,y,z
           position (or id) of an existing node in graph
        3) If cost_type is not left as the default, then it must be the name
           of a valid cost already defined in graph.

    Raises:
        ValueError: Length of start and end arrays did not match
        KeyError: cost_type wasn't blank and did not point to an already defined
            cost in the graph


    Example:
        Creating a graph, adding edges to it, then generating a path from node 0 to 3.
        
        >>> from humanfactorspy.pathfinding import DijkstraShortestPath
        >>> from humanfactorspy.spatialstructures import Graph

        >>> g = Graph()
        >>> g.AddEdgeToGraph(0, 1, 100)
        >>> g.AddEdgeToGraph(0, 2, 50)
        >>> g.AddEdgeToGraph(1, 3, 10)
        >>> g.AddEdgeToGraph(2, 3, 10)
        >>> csr = g.CompressToCSR()

        >>> SP = DijkstraShortestPath(g, 0, 3)
        >>> print(SP)
        [(50., 0) (10., 2) ( 0., 3)]

        The same, but creating multiple paths.
        
        >>> from humanfactorspy.pathfinding import DijkstraShortestPath
        >>> from humanfactorspy.spatialstructures import Graph

        >>> g = Graph()
        >>> g.AddEdgeToGraph(0, 1, 100)
        >>> g.AddEdgeToGraph(0, 2, 50)
        >>> g.AddEdgeToGraph(1, 3, 10)
        >>> g.AddEdgeToGraph(2, 3, 10)
        >>> csr = g.CompressToCSR()

        >>> SP = DijkstraShortestPath(g, [1] * 10, [3] * 10)
        >>> for path in SP:
        ...     print(path)
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]
        [(10., 1) ( 0., 3)]


    """

    # Compress the graph if it isn't already.
    graph.CompressToCSR()

    # Convert tuples to ints if it matters and throw if those nodes
    # don't actually exist in the graph
    start = ConvertNodesToIds(graph, start)
    end = ConvertNodesToIds(graph, end)

    # Throw if the caller violates our pre condition
    if len(start) != len(end):
        raise ValueError(f"Length of start array ({len(start)}) did not match length of end array {len(end)}!")

    # If we're only generating a single path, then call the single path
    # function
    if len(start) == 1:
        res = pathfinder_native_functions.C_FindPath(
            graph.graph_ptr, start[0], end[0], cost_type)

        # Check if a path could be found between start and end
        if res:  # If so, wrap the pointers in a python Path
            path_ptr, data_ptr, size = res
            return Path(path_ptr, data_ptr, size)
        else:  # If not return null
            return None

    # If multiple paths are going to be generated, then call the multiple path
    # function
    else:
        res = pathfinder_native_functions.C_FindPaths(
            graph.graph_ptr, start, end, cost_type)

        # Wrap values that weren't null in python paths
        out_paths = [Path(result[0], result[1], result[2])
                     if result else None for result in res]
        return out_paths
