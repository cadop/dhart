import pytest
import numpy

from dhart.pathfinding import DijkstraShortestPath, DijkstraFindAllShortestPaths, calculate_distance_and_predecessor, AlternateCostsAlongPath
from dhart.spatialstructures import Graph

test_cost = "Test"

@pytest.fixture
def PathTestGraph() -> Graph:
    """ Creates a graph suitable for testing pathfinding"""
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 3, 10)
    g.AddEdgeToGraph(2, 3, 10)
    g.CompressToCSR()
    return g


@pytest.fixture
def PathTestGraphAlternateCosts() -> Graph:
    """ Creates a graph suitable for testing pathfinding"""
    # Create graph and add edges
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 3, 10)
    g.AddEdgeToGraph(2, 3, 10)

    # Compress the graph so we can add
    # New cost
    g.CompressToCSR()
    
    # Add edges to test cost
    g.AddEdgeToGraph(0, 1, 1, test_cost)
    g.AddEdgeToGraph(0, 2, 150, test_cost)
    g.AddEdgeToGraph(1, 3, 5, test_cost)
    g.AddEdgeToGraph(2, 3, 100, test_cost)
    
    return g


def test_ShortestPath(PathTestGraph):
    """ Generate a path and ensure that it actually is the
        shortest path"""

    g = PathTestGraph

    # Define the path that should be generated
    correct_path = [[50, 0], [10, 2], [0, 3]]

    # Find a path from 0 to 3. It should equal correct_path
    SP = DijkstraShortestPath(g, 0, 3)

    # Assert that this path is not null
    assert(SP is not None)

    # Assert that it is equal to correct path
    for i in range(0, len(correct_path)):
        assert correct_path[i][0] == SP[i][0]
        assert correct_path[i][1] == SP[i][1]


def test_MultipleShortestPaths(PathTestGraph):
    """ Generate multiple paths and ensure that they
        are all identical """
    g = PathTestGraph

    # Before we begin, ensure that calling it with differing start
    # and end arrays raises a value error
    with pytest.raises(ValueError):
        SPS = DijkstraShortestPath(g, [0] * 10, [3] * 100)
        
    
    # Define the correct path
    correct_path = [[50, 0], [10, 2], [0, 3]]
    
    # Find the same path num_paths times
    num_paths = 1000
    SPS = DijkstraShortestPath(g, [0] * num_paths, [3] * num_paths)

    # Assert that we got an output back of the correct size
    assert len(SPS) == num_paths

    # Iterate through every path in the output
    for SP in SPS:
        # Assert that this path isn't null
        assert(SP is not None)
        
        # Assert that it equals correctpath
        for i in range(0, len(correct_path)):
            assert correct_path[i][0] == SP[i][0]
            assert correct_path[i][1] == SP[i][1]

def test_ShortestPathAltCost(PathTestGraphAlternateCosts):
    """ Find a single path on an alternate cost type and compare
        the results """
    g = PathTestGraphAlternateCosts
    
    # First assert calling a path with a non-existant cost type
    # Throws our exception
    with pytest.raises(KeyError):
        DijkstraShortestPath(g, 0, 3, "NotExistCost")

    # Create path
    SP = DijkstraShortestPath(g, 0, 3, test_cost)

    # Define what the shortest path SHOULD be 
    correct_path = [[1, 0], [5 ,1], [0, 3]]

    print(SP)
    # Compare output with what the shortest path should be
    for i in range(0, len(correct_path)):
        assert correct_path[i][0] == SP[i][0]
        assert correct_path[i][1] == SP[i][1]


def test_MultipleShortestPathAltCost(PathTestGraphAlternateCosts):
    """ Find the same path on an alternate cost type
        multiple times and compare the results """

    g = PathTestGraphAlternateCosts

    # Set up start and end point arrays
    num_paths = 1000
    start_points = [0] * num_paths
    end_points = [3] * num_paths

    # First assert calling a path with a non-existant cost type
    # Throws our exception
    with pytest.raises(KeyError):
        DijkstraShortestPath(g, start_points, end_points, "NotExistCost")

    # Create paths
    SPS = DijkstraShortestPath(g, start_points, end_points, test_cost)

    # Define what the shortest path SHOULD be 
    correct_path = [[1, 0], [5 ,1], [0, 3]]

    # Check that every path was successfully generated
    non_null_paths = [SP for SP in SPS if SP is not None]
    assert(len(non_null_paths) == num_paths)

    # Compare every path against the correct output
    for SP in SPS:
        for i in range(0, len(correct_path)):
            assert correct_path[i][0] == SP[i][0]
            assert correct_path[i][1] == SP[i][1]


def AssertValidityOfAllToAllPaths(
    g,
    all_paths,
    cost_type=""
    ):
    """ Checks if a set of all to all paths is equivalent
        to paths that were generated one by one """

    num_nodes = g.NumNodes()

    # Iterate through every path in the list, using the number of nodes
    # to determine the position of a path from it's start and end nodes
    for start in range(0, num_nodes):
        for end in range(0, num_nodes):

            # Calculate the index and get the path from start to end
            path_index = num_nodes * start + end
            actual_path = all_paths[path_index]

            # Calculate the expected shortest path using the single method
            expected_path = DijkstraShortestPath(g, start, end, cost_type)

            # Assert equality
            if expected_path is None:
                assert(actual_path is None)
            else:  # If they're not null, compare their members
                assert(list(expected_path.array) == list(actual_path.array))


def test_AllToAllPaths(PathTestGraphAlternateCosts):
    """ Assert that the results from all to all equal the actua
        shortest paths between each set of nodes. """

    g = PathTestGraphAlternateCosts

    # Assert that this will throw a key error 
    with pytest.raises(KeyError):
        DijkstraFindAllShortestPaths(g, "CostDontExist")

    # Get the result of all paths then check them against
    # the single path result
    all_paths = DijkstraFindAllShortestPaths(g)
    AssertValidityOfAllToAllPaths(g, all_paths)

    # Now do the same for an alternate cost
    all_alt_paths = DijkstraFindAllShortestPaths(g, test_cost)
    AssertValidityOfAllToAllPaths(g, all_alt_paths, test_cost)


def test_CalculateDistanceAndPredecessor():
    # Create a graph, add some nodes and edges, then compress
    g = Graph()

    nodes = [(1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 1, 2)]
    g.AddEdgeToGraph(nodes[1], nodes[2], 20)
    g.AddEdgeToGraph(nodes[0], nodes[2], 5)
    g.AddEdgeToGraph(nodes[1], nodes[0], 10)

    g.CompressToCSR()

    # Calculate distance/predecessor matrix
    distance_matrix, predecessor_matrix = calculate_distance_and_predecessor(g)

    assert(distance_matrix[0][0] == 0)
    assert(distance_matrix[1][0] == -1)


    # Print output
    print(distance_matrix)
    print(predecessor_matrix)

def test_AlternateCostsAlongPathIDs():
    g = Graph()
    cost_type = "TestCost"
    g.AddEdgeToGraph(0,1,50)
    g.AddEdgeToGraph(0,2,10)
    g.AddEdgeToGraph(1,2,150)
    g.AddEdgeToGraph(1,3,70)
    g.AddEdgeToGraph(2,3,70)

    g.CompressToCSR()

    g.AddEdgeToGraph(0, 1, 100, cost_type)
    g.AddEdgeToGraph(0, 2, 50, cost_type)
    g.AddEdgeToGraph(1, 2, 20, cost_type)
    g.AddEdgeToGraph(1,3, 1000, cost_type)
    g.AddEdgeToGraph(2,3, 1500, cost_type)

    shortest_path = [0,2,3]

    alternate_costs = AlternateCostsAlongPath(g, shortest_path, cost_type)
    assert(alternate_costs == [50, 1500])
    
def test_AlternateCostsAlongPathStruct():
    g = Graph()
    cost_type = "TestCost"
    g.AddEdgeToGraph(0,1,50)
    g.AddEdgeToGraph(0,2,10)
    g.AddEdgeToGraph(1,2,150)
    g.AddEdgeToGraph(1,3,70)
    g.AddEdgeToGraph(2,3,70)

    g.CompressToCSR()

    g.AddEdgeToGraph(0, 1, 100, cost_type)
    g.AddEdgeToGraph(0, 2, 50, cost_type)
    g.AddEdgeToGraph(1, 2, 20, cost_type)
    g.AddEdgeToGraph(1,3, 1000, cost_type)
    g.AddEdgeToGraph(2,3, 1500, cost_type)

    SP = DijkstraShortestPath(g, 0, 3)

    alternate_costs = AlternateCostsAlongPath(g, SP, cost_type)
    assert(alternate_costs == [50,1500])