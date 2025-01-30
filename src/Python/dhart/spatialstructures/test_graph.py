import pytest
import unittest
import os
import pdb

import numpy

from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import embree_raytracer, EmbreeBVH
from dhart.spatialstructures import NodeList, NodeStruct, Graph, CostAggregationType, Direction
from dhart.Exceptions import LogicError, InvalidCostOperation
from dhart.utils import is_point
import dhart.spatialstructures.node as NodeFunctions
import dhart.spatialstructures.edge as EdgeFunctions
import dhart.spatialstructures.cost_algorithms as cost_algorithms


# This is a pytest fixture!  You can add SimpleGraph to the arguments
# of any test function to pass the return of this function to it.
# This can drastically cut down on code size, to get to the logic
# that matters. 
@pytest.fixture
def SimpleGraph() -> Graph:
    """ Create a simple graph with 3 nodes """
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 2, 20)
    g.CompressToCSR()
    return g


@pytest.fixture
def SimpleGraphWithCosts() -> Graph:
    # Create a graph, add some edges, then compress it
    g = Graph()
    g.AddEdgeToGraph(0, 1, 100)
    g.AddEdgeToGraph(0, 2, 50)
    g.AddEdgeToGraph(1, 2, 20)
    g.CompressToCSR()

    # Add some edges for an alternate cost
    g.AddEdgeToGraph(0, 1, 1, test_cost)
    g.AddEdgeToGraph(0, 2, 1, test_cost)
    g.AddEdgeToGraph(1, 2, 1, test_cost)

    return g

@pytest.fixture
def SimpleXYZGraph() -> Graph:

    """Simplegraph, but with nodes at x,y,z locations instead of integers"""
    nodes = [
        [0,0,1],
        [0,0,2],
        [0,0,3]
    ]
    
    g = Graph()
    g.AddEdgeToGraph(nodes[0], nodes[1], 100)
    g.AddEdgeToGraph(nodes[0], nodes[2], 50)
    g.AddEdgeToGraph(nodes[1], nodes[2], 20)
    g.CompressToCSR()
    return g

test_cost = "Test"

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

def test_GetEdgesForNode():
    # Construct graph with 3 nodes and 3 edges
    g = Graph()

    N0 = NodeStruct(1,2,3,0,0)
    N1 = NodeStruct(2,3,4,0,1)
    N2 = NodeStruct(19,2,3,0,2)
    nodes = [N0, N1, N2]
    edges = [
        (nodes[0], nodes[1]),
        (nodes[0], nodes[2]),
        (nodes[1], nodes[2]),
    ]

    for edge in edges:
        g.AddEdgeToGraph(edge[0], edge[1], 39)

    # Compression needed before doing anything
    g.CompressToCSR()

    edge_list = g.GetEdgesForNode(nodes[0])

    # Assert that we have the correct number of edges
    assert len(edge_list.array) == 2

    # Assert that the edges are correct
    expected_edges = ((N1, 1, 39), (N2, 1, 39))
    for i in range(len(expected_edges)):
        edge = edge_list.array[i]
        expected_edge = expected_edges[i]
        assert edge[0] == expected_edge[0]
        assert edge[1] == expected_edge[1]
        assert edge[2] == expected_edge[2]

def test_CreateNodes():
    nodes = [(1, 2, 3), (20, 2110, 100)]
    structs = NodeFunctions.CreateListOfNodeStructs(nodes)

    for i in range(0, len(nodes)):
        node = nodes[i]
        print(nodes)
        np_node = structs[i]
        assert node[0] == np_node[0]
        assert node[1] == np_node[1]
        assert node[2] == np_node[2]

def test_CreateEdges():
    nodes = [(1, 2, 3), (3, 5, 8), (5, 8, 13)]
    node_structs = NodeFunctions.CreateListOfNodeStructs(nodes)
    steps = [1,2]
    weights = [20,40]

    parent = node_structs[0]

    edges = [(node_structs[i], steps[i-1], weights[i-1]) for i in range(1, len(nodes))]
    edgestructs = EdgeFunctions.CreateListOfEdgeStructs(edges)
    for i in range(0, len(nodes)-1):
        edge = edges[i]
        np_edge = edgestructs[i]
        assert edge[0] == np_edge[0]
        assert edge[1] == np_edge[1]
        assert edge[2] == np_edge[2]

def test_AggregateCostType(SimpleGraphWithCosts):
    """ Test aggregating the edges of a graph using an alternate
    cost type """

    g = SimpleGraphWithCosts

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

def test_GetCosts():
    """Tests that getting multiple costs from the graph is accurate"""
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

    ids = [0,1,1,2, 2, 3]

    # All costs of cost_type
    expected_all_costs = [100.0, 50.0, 20.0, 1000.0, 1500.0]
    all_costs = g.GetEdgeCosts(cost_type)
    for i in range(0, len(expected_all_costs)):
        assert(all_costs[i] == expected_all_costs[i])

    # Specific edges to get costs for
    expected_some_costs = [100.0, 20.0, 1500.0]
    some_costs = g.GetEdgeCosts(cost_type, ids)
    for i in range(0, len(expected_some_costs)):
        assert(some_costs[i] == expected_some_costs[i])

def test_AddingAndReadingCostTypesNodeStruct():
    """ Tests that alternate cost types can be added and read. Also ensures
    that error cases are handled and thrown."""

    # Create Graph
    g = Graph()

    N0 = NodeStruct(1,2,3,0,0)
    N1 = NodeStruct(2,3,4,0,1)
    N2 = NodeStruct(19,2,3,0,2)

    # Add initial edges to default cost type and compress
    g.AddEdgeToGraph(N0, N1, 100)
    g.AddEdgeToGraph(N1, N2, 200)

    # Ensure that we catch callers trying to add alternate costs
    # to sets of edges before compressing the graph
    with pytest.raises(LogicError):
        g.AddEdgeToGraph(0, 1, 250, "cost")

    # Compress the graph
    g.CompressToCSR()

    # Add edges to the graph for this new cost type
    test_cost = "Test"
    g.AddEdgeToGraph(N0, N1, 250, test_cost)
    g.AddEdgeToGraph(N1, N2, 251, test_cost)

    # Assert that the edges added succssfully
    assert(g.GetEdgeCost(0, 1, test_cost) == 250)
    assert(g.GetEdgeCost(1, 2, test_cost) == 251)

    # Ensure we throw if our precondition was violated
    with pytest.raises(InvalidCostOperation):
        g.AddEdgeToGraph(N1, N0, 10, test_cost)

def test_AddingAndReadingCostTypesIDs():
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


def test_ClearCost(SimpleGraphWithCosts):
    """ Ensures that a cost_type can be cleared by calling Clear
    """
    g = SimpleGraphWithCosts

    # Clear this alternate cost from the graph
    g.Clear(test_cost)

    # Ensure that the graph still exists
    assert(g.GetEdgeCost(0, 1) == 100)

    # If we try to get the cost of one of it's edges
    # we should get a no_cost exception
    with pytest.raises(KeyError):
        g.GetEdgeCost(0, 1, test_cost)


def test_NumNodesEqualsLengthOfNodes(SimpleGraph):
    """ Asserts that the number returned by numnodes actually matches
        the length of the graph's nodes array """

    g = SimpleGraph

    # Get the nodes and num nodes
    nodes = g.getNodes().array
    num_nodes = g.NumNodes()

    # Assert that they are equal
    assert(len(nodes) == num_nodes)


def test_StoresCrossSlope(SimpleXYZGraph):
    """ Ensure cross slope  is being added to the graph once CrossSlope is called """

    # Create a test graph
    g = SimpleXYZGraph

    # Calculate cross slope for it
    cost_algorithms.CalculateCrossSlope(g)

    # Get the cross algorithm key for it
    key = cost_algorithms.CostAlgorithmKeys.CROSS_SLOPE

    # Print the CSR of it
    csr = g.CompressToCSR(key)
    print("========= CROSS SLOPE ============")
    print(csr)


def test_StoresEnergyExpenditure(SimpleXYZGraph):
    """ Ensure energy expenditure is being added to the graph once EnergyExpenditure  is called """

    # Calculate cross slope for it
    cost_algorithms.CalculateEnergyExpenditure(SimpleXYZGraph)

    # Get the cross algorithm key for it
    key = cost_algorithms.CostAlgorithmKeys.ENERGY_EXPENDITURE

    # Print the CSR of it
    csr = SimpleXYZGraph.CompressToCSR(key)
   
    print("========= ENERGY EXPENDITURE ============")
    print(csr)


def test_AddNodeAttribute(SimpleGraph):

    # Add node attributes to the simple graph
    attr = "Test"
    ids = [0, 1, 2]
    scores = ["zero", "one", "two"]
    SimpleGraph.add_node_attributes(attr, ids, scores)

    # Get attribute scores from the graph
    out_attrs = SimpleGraph.get_node_attributes(attr)

    # Assert it's equal to our input array
    assert out_attrs == scores


def test_ClearNodeAttributes(SimpleGraph):

    # Add node attributes to the simple graph
    attr = "Test"
    ids = [0, 1, 2]
    scores = ["zero", "one", "two"]
    SimpleGraph.add_node_attributes(attr, ids, scores)

    # Clear the attribute
    SimpleGraph.clear_node_attribute(attr)

    # Assert that getting the node attribute now
    # returns an empty list, indicating that the
    # attribute doesn't exist
    cleared_attr = SimpleGraph.get_node_attributes(attr)
    assert cleared_attr == []

def test_IsPoint():
    # No square brackets operator
    assert not is_point(1) 

    # Has square brackets operator, but doesn't hold numbers
    assert not is_point(["string", "string", "string"]) 
    
    # Has square brackets operator, but holds lists
    assert not is_point([[1, 2, 3], [1, 2, 3], [1, 2, 3]])

    # Has square brackets operator but only holds 2 elements
    assert not is_point([1, 2])

    # The following statments should all be 
    # considered points
    assert is_point([1, 2, 3])
    assert is_point([1.0, 2.0, 2.5])
    assert is_point((1, 2, 3))
    assert is_point((1.0, 2.0, 3.0))
 
def test_attributes_to_costs(SimpleGraph):

    # Add node attributes to the simple graph
    attr = "Test"
    ids = [0, 1, 2]
    scores = ["0", "100", "200"]
    SimpleGraph.add_node_attributes(attr, ids, scores)

    # Get attribute scores from the graph
    out_attrs = SimpleGraph.get_node_attributes(attr)

    SimpleGraph.attrs_to_costs(attr, "attr_cost", Direction.INCOMING)

    # Assert it's equal to our input array
    assert 200 == SimpleGraph.GetEdgeCost(1, 2, "attr_cost")

