import pytest

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import embree_raytracer, EmbreeBVH
from humanfactorspy.spatialstructures import Graph, NodeList, node
from humanfactorspy.visibilitygraph import visibility_graph
from time import time


def test_VisibilityGraph():
    mesh_path = "Example Models\\plane.obj"
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    graph_size = 1000
    nodes = [(i, i, 0) for i in range(int(-graph_size / 2), int(graph_size / 2))]

    start = time()
    graph = visibility_graph.VisibilityGraphAllToAll(bvh, nodes, 1.7)
    end = time()

    print(
        f"Directed Visibility Graph Completed in {end-start} seconds for {graph_size} nodes"
    )
    csr = graph.CompressToCSR()
    print("\nChecking format....")
    csr.check_format(True)

    print("Done!")
    print(csr.count_nonzero())
    # print(csr)


def test_UndirectedVisibilityGraph():
    mesh_path = "Example Models\\plane.obj"
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    graph_size = 1000
    nodes = [(i, i, 0) for i in range(int(-graph_size / 2), int(graph_size / 2))]

    graph = visibility_graph.VisibilityGraphUndirectedAllToAll(bvh, nodes, 1.7, -1)
    csr = graph.CompressToCSR()
    print("\nChecking format....")
    csr.check_format(True)

    print("Done!")
    print(csr.count_nonzero())
    # print(csr)


def test_visibilitygraph_group():
    mesh_path = "Example Models\\plane.obj"
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)
    input("remember me")
    graph_size = 10
    nodes_a = [(i, i, 0) for i in range(int(-graph_size / 2), int(graph_size / 2))]
    nodes_b_bad = [(0, 0, -20), (0, 0, -21)]
    nodes_b_good = [(0, 0, 2), (0, 0, 3)]
    graph = visibility_graph.VisibilityGraphGroupToGroup(bvh, nodes_a, nodes_b_bad, 1.7, -1)
    assert(not graph)
    graph = visibility_graph.VisibilityGraphGroupToGroup(bvh, nodes_a, nodes_b_good, 1.7, -1)
    csr = graph.CompressToCSR()
    print("Done!")
    print(csr.count_nonzero())
    # print(csr)
