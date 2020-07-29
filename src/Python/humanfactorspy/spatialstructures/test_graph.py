import pytest
import unittest
import os

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import embree_raytracer, EmbreeBVH
from humanfactorspy.spatialstructures import NodeList, NodeStruct, Graph
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
    g.AddEdgeToGraph(0,1,100)
    g.CompressToCSR()

    # Get the cost from the edge
    cost_from_graph = g.GetEdgeCost(0,1)

    # Assert this edge cost equals what we specified above
    assert(cost_from_graph == 100)
    pass

def test_AddEdgeWithCostType():
    # Create Graph
    g = Graph()

    # Add initial edges to default cost type
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(1,2,200)

    # 
