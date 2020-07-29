import pytest
import unittest
import os

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import embree_raytracer, EmbreeBVH
from humanfactorspy.spatialstructures import NodeList, NodeStruct, Graph
from humanfactorspy.Exceptions import LogicError, InvalidCostOperation
import humanfactorspy.spatialstructures.node as NodeFunctions


# from humanfactorspy.graphgenerator.graph_generator import GenerateGraph
# Setup
def test_CreateGraphAndNodes():
    nodes = [(1, 2, 3), (2, 3, 4), (19, 2, 3)]

    edges = [
        (nodes[0], nodes[1]),
        (nodes[0], nodes[2]),
        (nodes[1], nodes[2]),
    ]

    g = Graph()
    for edge in edges:
        g.AddEdgeToGraph(edge[0], edge[1], 39)

    csr = g.CompressToCSR()
    csr.check_format(True)  # If this was invalid it would fail here
    print(csr)

def test_GetEdgeCosts():
    nodes = [(1, 2, 3), (2, 3, 4), (19, 2, 3)]

    edges = [
        (nodes[0], nodes[1]),
        (nodes[0], nodes[2]),
        (nodes[1], nodes[2]),
    ]

    g = Graph()
    for edge in edges:
        g.AddEdgeToGraph(edge[0], edge[1], 39)

    g.CompressToCSR()
    assert(g.AggregateEdgeCosts(0, True)[0] == (39*2))


def test_GetNodes():
    g = Graph()
    nodes = [(1, 2, 3), (2, 3, 4), (19, 2, 3)]

    edges = [
        (nodes[0], nodes[1]),
        (nodes[0], nodes[2]),
        (nodes[1], nodes[2]),
    ]

    for edge in edges:
        g.AddEdgeToGraph(edge[0], edge[1], 39)

    node_list = g.getNodes()
    assert len(node_list.array) == 3
    for node in node_list.array:
        print(node)


def test_CreateNodes():
    nodes = [(1, 2, 3), (20, 2110, 100)]
    structs = NodeFunctions.CreateListOfNodeStructs(nodes)

    for i in range(0, len(nodes)):
        node = nodes[i]
        np_node = structs[i]
        assert node[0] == np_node[0]
        assert node[1] == np_node[1]
        assert node[2] == np_node[2]

def test_GetCSRCost():
    pass


def test_GetCost():
    # Create a graph, add an edge, then compress it
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.CompressToCSR()

    # Get the cost from the edge
    cost_from_graph = g.GetEdgeCost(0, 1)

    # Assert this edge cost equals what we specified above
    assert(cost_from_graph == 100)


def test_AddEdgeWithCostType():
    # Create Graph
    g = Graph()

    # Add initial edges to default cost type and compress
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(1, 2, 200)

    # Ensure that we catch callers trying to add alternate costs
    # to sets of edges before compressing the graph
    with pytest.raises(LogicError):
        g.AddEdgeToGraph(0, 1, 250, "cost")

    # Compress the graph
    g.CompressToCSR()

    # Add edges to the graph for this new cost type
    test_cost = "Test"
    g.AddEdgeToGraph(0, 1, 250, test_cost)
    g.AddEdgeToGraph(1, 2, 251, test_cost)

    # Assert that the edges added succssfully
    assert(g.GetEdgeCost(0, 1, test_cost) == 250)
    assert(g.GetEdgeCost(1, 2, test_cost) == 251)

    # Ensure we throw if our precondition was violated
    with pytest.raises(InvalidCostOperation):
        g.AddEdgeToGraph(1, 0, 10, test_cost)




