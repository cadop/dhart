import pytest
import unittest
import os

import numpy

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import embree_raytracer, EmbreeBVH
from humanfactorspy.spatialstructures import NodeList, NodeStruct, Graph, CostAggregationType
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


def test_AggregateCostType():
    """ Test aggregating the edges of a graph using an alternate
    cost type """

    # Create a graph, add some edges, then compress it
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 2, 20)
    g.CompressToCSR()

    # Add some edges for an alternate cost
    test_cost = "Test"
    g.AddEdgeToGraph(0, 1, 1, test_cost)
    g.AddEdgeToGraph(0, 2, 1, test_cost)
    g.AddEdgeToGraph(1, 2, 1, test_cost)

    # Aggregate the edges of both the default
    # and alternate costs
    ct = CostAggregationType.SUM
    default_aggregated = g.AggregateEdgeCosts(ct, True)
    alt_aggregated = g.AggregateEdgeCosts(ct, True, test_cost)

    # Get arrays and convert to list
    default_arr = list(default_aggregated.array)
    alt_arr = list(alt_aggregated.array)

    # Compare them to expectations
    assert(default_arr == [150, 20, 0])
    assert(alt_arr == [2, 1, 0])

def test_GetCSRCost():
    """ Tests getting a CSR with an alternate cost type """
    
    # Create a graph, add an edge, then compress it
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.CompressToCSR()

    # Ensure that trying to get a csr from cost type
    # before it is defined will throw
    with pytest.raises(KeyError):
        g.CompressToCSR("Test")

    # Add an alternate cost to the graph
    test_cost = "Test"
    g.AddEdgeToGraph(0, 1, 150, test_cost)

    # Get the CSR for the default cost and the test cost
    default_csr = g.CompressToCSR()
    cost_csr = g.CompressToCSR(test_cost)

    # Ensure both have proper values for their assigned cost
    print(cost_csr[0])
    assert(cost_csr[0].getcol(1) == 150)
    assert(default_csr[0].getcol(1) == 100)

def test_GetCost():
    """ Tests that getting a cost from the graph is accurate """
    # Create a graph, add an edge, then compress it
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.CompressToCSR()

    # Get the cost from the edge
    cost_from_graph = g.GetEdgeCost(0, 1)

    # Assert this edge cost equals what we specified above
    assert(cost_from_graph == 100)


def test_AddingAndReadingCostTypes():
    """ Tests that alternate cost types can be added and read. Also ensures
    that error cases are handled and thrown."""

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


def test_ClearCost():
    """ Ensures that a cost_type can be cleared by calling Clear 
    """

    # Create a graph, add some edges, then compress it
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 2, 20)
    g.CompressToCSR()

    # Add some edges to an alternate cost
    test_cost = "Test"
    g.AddEdgeToGraph(0, 1, 1, test_cost)
    g.AddEdgeToGraph(0, 2, 1, test_cost)
    g.AddEdgeToGraph(1, 2, 1, test_cost)

    # Clear this alternate cost from the graph
    g.Clear(test_cost)
    
    # Ensure that the graph still exists
    assert(g.GetEdgeCost(0,1) == 100)

    # If we try to get the cost of one of it's edges
    # we should get a no_cost exception
    with pytest.raises(KeyError):
        g.GetEdgeCost(0, 1, test_cost)

def test_NumNodesEqualsLengthOfNodes():
    """ Asserts that the number returned by numnodes actually matches
        the length of the graph's nodes array """
    
    # Create graph and dd some edges
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 2, 20)
    g.CompressToCSR()


    # Get the nodes and num nodes
    nodes = g.getNodes().array
    num_nodes = g.NumNodes()

    # Assert that they are equal
    assert(len(nodes) == num_nodes)

