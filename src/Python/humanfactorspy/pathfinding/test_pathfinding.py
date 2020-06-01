import pytest

from humanfactorspy.pathfinding import DijkstraShortestPath
from humanfactorspy.spatialstructures import Graph


def test_ShortestPath():

    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 3, 10)
    g.AddEdgeToGraph(2, 3, 10)
    g.CompressToCSR()

    SP = DijkstraShortestPath(g, 0, 3)

    correct_path = [[50, 0], [10, 2], [0, 3]]

    print(SP)
    for i in range(0, len(correct_path)):
        assert correct_path[i][0] == SP[i][0]
        assert correct_path[i][1] == SP[i][1]


def test_MultipleShortestPaths():

    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 3, 10)
    g.AddEdgeToGraph(2, 3, 10)
    g.CompressToCSR()

    num_paths = 1000
    SPS = DijkstraShortestPath(g, [0] * num_paths, [3] * num_paths)

    assert len(SPS) == num_paths

    correct_path = [[50, 0], [10, 2], [0, 3]]

    for SP in SPS:
        # print(SP)
        for i in range(0, len(correct_path)):
            assert correct_path[i][0] == SP[i][0]
            assert correct_path[i][1] == SP[i][1]