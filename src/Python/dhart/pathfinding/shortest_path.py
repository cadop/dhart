from . import Path
from . import pathfinder_native_functions
from dhart.Exceptions import OutOfRangeException
from typing import *
import numpy
from dhart.spatialstructures import Graph
from dhart.native_collections import FloatArray2D, IntArray2D

__all__ = ["ConvertNodesToIds", "DijkstraShortestPath", 
           "DijkstraFindAllShortestPaths", "calculate_distance_and_predecessor"]


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
            if multiple start/end ids were passed.If a path cannot be found to
            connect a start and end point that path will be set as None.
        Union[Path, None]:
             One start or end point was passed, return the single path or none
             if no path could be found between start and end.
    
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
        
        >>> from dhart.pathfinding import DijkstraShortestPath
        >>> from dhart.spatialstructures import Graph

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
        
        >>> from dhart.pathfinding import DijkstraShortestPath
        >>> from dhart.spatialstructures import Graph

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


    Example:
       Creating a graph then calculating an alternate cost type
       and generating a path on it.
                

        >>> import numpy as np
        >>> from dhart import get_sample_model
        >>> from dhart.geometry.obj_loader import LoadOBJ
        >>> from dhart.graphgenerator import GenerateGraph
        >>> from dhart.pathfinding import DijkstraShortestPath
        >>> from dhart.raytracer import EmbreeBVH
        >>> from dhart.spatialstructures.cost_algorithms import (
        ...                CalculateEnergyExpenditure, CostAlgorithmKeys)

        Load the energy blob and create a BVH from it

        >>> energy_blob_path = get_sample_model("energy_blob_zup.obj")
        >>> energy_blob_mesh = LoadOBJ(energy_blob_path)
        >>> bvh = EmbreeBVH(energy_blob_mesh)

        Define graph generator parameters

        >>> start_point = (-30, 0, 20)
        >>> spacing = (1, 1, 10)
        >>> max_nodes = 10000
        >>> up_step, down_step = 5, 5
        >>> up_slope, down_slope = 60, 60
        >>> max_step_connections = 1

        Generate a graph on it

        >>> g = GenerateGraph(bvh, start_point, spacing, max_nodes,
        ...                    up_step, up_slope, down_step, down_slope,
        ...                    max_step_connections, cores=-1)

        Compress the graph

        >>> csr = g.CompressToCSR()

        Generate an alternate cost type and store it in the graph

        >>> CalculateEnergyExpenditure(g)

        Generate a path using the energy expenditure cost and distance (the default)

        >>> start_point = 1
        >>> end_point = 150
        >>> energy_expend_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
        >>> distance_path = DijkstraShortestPath(g, start_point, end_point)
        >>> energy_path = DijkstraShortestPath(g, start_point, end_point, energy_expend_key)
        >>> 
        >>> # Print both paths
        >>> print("Distance Path:", [ (np.around(x[0],5), x[1]) for x in distance_path.array ] )
        Distance Path: [(1.0, 1), (1.0, 12), (1.0, 26), (1.0, 43), (1.0, 64), (1.00001, 89), (1.41489, 118), (0.0, 150)]

        >>> print("Energy Path:", [ (np.around(x[0],5), x[1]) for x in energy_path.array ] )
        Energy Path: [(2.47461, 1), (2.49217, 12), (2.5, 26), (2.48045, 43), (2.45134, 64), (2.43783, 89), (2.75192, 118), (0.0, 150)]


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


def DijkstraFindAllShortestPaths(
    graph: Graph,
    cost_type: str = "",
    ) -> List[Union[Path, None]]:
    """ Find All Pairs Shortest Path
    
    Find the shortest path between every possible combination of nodes 
    in a graph.

    Parameters
    ----------

    Graph : The graph to generate paths in. 
    cost_type : Which cost to use for path generation. If no cost type is specified,
        then the graph's default cost type will be used. If a cost type is specified
        then it must already exist in the graph.
    
    Returns
    -------

    List[Union[path, None]]:
        Of a path from every node to every other node in the graph in order
        of start point then end point. If a specific path could not be 
        generated, then it will be set to None.
    
    Precondition
    ------------

    If cost_type is not left as the default, then it must be the name
    of a valid cost already defined in graph.

    Raises
    ------

    KeyError: cost_type wasn't blank and did not point to an already defined
        cost in the graph

    Examples
    --------

    >>> from dhart.pathfinding import DijkstraShortestPath, DijkstraFindAllShortestPaths
    >>> from dhart.spatialstructures import Graph
    >>> import numpy as np
    >>> 
    >>> g = Graph()
    >>> g.AddEdgeToGraph(0, 1, 100)
    >>> g.AddEdgeToGraph(0, 2, 50)
    >>> g.AddEdgeToGraph(1, 3, 10)
    >>> g.AddEdgeToGraph(2, 1, 10)
    >>> g.AddEdgeToGraph(3, 4, 10)
    >>> g.AddEdgeToGraph(4, 1, 10)
    >>> g.AddEdgeToGraph(4, 2, 10)
    >>> g.AddEdgeToGraph(2, 4, 10)
    >>> csr = g.CompressToCSR()
    >>> 
    >>> # Size of G
    >>> g_size = len(g.getNodes())
    >>> 
    >>> # Get APSP
    >>> SP = DijkstraFindAllShortestPaths(g)
    >>> 
    >>> # Reshape APSP result to nxn of graph size
    >>> apsp_mat = np.reshape(SP, (g_size,g_size))
    >>> 
    >>> for i, row in enumerate(apsp_mat):
    ...     print('Paths from node: ',i)
    ...     for j, path in enumerate(row):
    ...         if path is not None:
    ...             print('To node: ',j, ' = ', path['id'])
    ...         else: 
    ...             print('To node: ',j, ' =   ')
    ... # doctest: +NORMALIZE_WHITESPACE
    Paths from node:  0
    To node:  0  =   
    To node:  1  =  [0 2 1]
    To node:  2  =  [0 2]
    To node:  3  =  [0 2 1 3]
    To node:  4  =  [0 2 4]
    Paths from node:  1
    To node:  0  =   
    To node:  1  =   
    To node:  2  =  [1 3 4 2]
    To node:  3  =  [1 3]
    To node:  4  =  [1 3 4]
    Paths from node:  2
    To node:  0  =   
    To node:  1  =  [2 1]
    To node:  2  =   
    To node:  3  =  [2 1 3]
    To node:  4  =  [2 4]
    Paths from node:  3
    To node:  0  =   
    To node:  1  =  [3 4 1]
    To node:  2  =  [3 4 2]
    To node:  3  =   
    To node:  4  =  [3 4]
    Paths from node:  4
    To node:  0  =   
    To node:  1  =  [4 1]
    To node:  2  =  [4 2]
    To node:  3  =  [4 1 3]
    To node:  4  =   

    >>> # Convert to a distance matrix
    >>> # Create an empty array the size of the graph
    >>> # Then set distance to infinity if there is no path
    >>> # and 0 if it is the distance to itself
    >>> dst = np.empty((g_size,g_size))
    >>> for i, path in enumerate(SP):
    ...     if path is None: 
    ...         if int(i/g_size) == i%g_size: dist = 0
    ...         else: dist = np.inf
    ...     else: dist = np.sum(path['cost_to_next'])
    ...     dst[int(i/g_size)][i%g_size] = dist
    >>> 
    >>> print(dst)
    [[ 0. 60. 50. 70. 60.]
     [inf  0. 30. 10. 20.]
     [inf 10.  0. 20. 10.]
     [inf 20. 20.  0. 10.]
     [inf 10. 10. 20.  0.]]

    """

    # Call out to native code and get results
    all_results = pathfinder_native_functions.C_FindAllPaths(graph.graph_ptr,
                                                             graph.NumNodes(),
                                                             cost_type)

    # Wrap values that weren't null in python paths
    out_paths = [Path(result[0], result[1], result[2])
                 if result else None for result in all_results]

    return out_paths


def calculate_distance_and_predecessor(graph: Graph, cost_type: str = ""
    ) -> Tuple[FloatArray2D, IntArray2D]:
    """ Calculate distance and predecessor matricies for a graph in C++

    Args:
        graph : Graph to generate predecessor/distance matricies from
        cost_type : Type of cost to use to generate distance and predecessor
                   matricies. Uses graph's default cost type if left blank

    Raises:
        KeyError : cost_type wasn't left blank, and didn't already exist in the
                   graph.

    Returns:
        The Distance and Predecessor matricies for graph.

    Examples:
        Create the predecessor and distance matricies for a graph

        >>> from dhart.pathfinding import calculate_distance_and_predecessor
        >>> from dhart.spatialstructures import Graph

        >>> # Create a graph, add some nodes and edges, then compress
        >>> g = Graph()
        >>> nodes = [(1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 1, 2)]
        >>> g.AddEdgeToGraph(nodes[1], nodes[2], 20)
        >>> g.AddEdgeToGraph(nodes[0], nodes[2], 5)
        >>> g.AddEdgeToGraph(nodes[1], nodes[0], 10)
        >>> csr = g.CompressToCSR()
        >>> # Calculate distance/predecessor matrix
        >>> distance_matrix, predecessor_matrix = calculate_distance_and_predecessor(g)
        >>> print(distance_matrix)
        [[ 0. 15. 10.]
         [-1.  0. -1.]
         [-1.  5.  0.]]
        >>> print(predecessor_matrix)
        [[ 0  2  0]
         [-1  1 -1]
         [-1  2  2]]

    """

    # Get pointers to arrays back from native code
    (dist_vector,
        dist_data,
        pred_vector,
        pred_data,
     ) = pathfinder_native_functions.c_calculate_distance_and_predecessor(
        graph.graph_ptr, cost_type
    )

    # Get the size of the graph
    num_nodes = graph.NumNodes()

    # Wrap in NativeNumpyLikes
    dist_matrix = FloatArray2D(dist_vector, dist_data, (num_nodes, num_nodes))
    pred_matrix = IntArray2D(pred_vector, pred_data, (num_nodes, num_nodes))

    return (dist_matrix, pred_matrix)
