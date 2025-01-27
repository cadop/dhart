import math

from dhart.geometry import LoadOBJ, CommonRotations, OBJGroupType
from dhart.raytracer import EmbreeBVH
from dhart.graphgenerator.graph_generator import GenerateGraph, CalculateAndStoreStepTypes
from dhart.spatialstructures.graph import *
import dhart

def test_GetNodes():
    mesh_path = dhart.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)
    g = GenerateGraph(bvh, (0, 0, 0.1), (0.5, 0.5, 0.5), 1000)
    node_list = g.getNodes()
    assert len(node_list.array) > 0
    print(g.CompressToCSR())


def test_UnsuccessfulGraphCreationDoesntThrow():
    mesh_path = dhart.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)
    g = GenerateGraph(bvh, (200000, 0, 0.1), (0.5, 0.5, 0.5), 1000)
    assert g is None


def test_EnsureParallelGraphIsEquivalentToStandardGraph():
    mesh_path = dhart.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    start_point = (0, 0, 1)
    spacing = (1, 1, 1)
    max_nodes = 500

    parallel_graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)
    standard_graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=0)

    parallel_nodes = parallel_graph.getNodes()
    standard_nodes = standard_graph.getNodes()
    parallel_array = parallel_nodes.array
    standard_array = standard_nodes.array

    # Note that the parallel graph can potentially have more nodes since it
    # performs calculations in batches instead of one by one
    assert len(parallel_array) >= len(standard_array)
    for i in range(0, len(standard_array)):
        std_node = standard_array[i]
        par_node = parallel_array[i]

        distance = math.sqrt(
                            pow(std_node[0] - par_node[0], 2)
                            + pow(std_node[1] - par_node[1], 2)
                            + pow(std_node[2] - par_node[2], 2))

        assert distance < 0.001

        
def test_energyblob_size():
    # Get a sample model path
    obj_path = dhart.get_sample_model("energy_blob_zup.obj")
    # time.sleep(10)
    # Load the obj file
    obj = LoadOBJ(obj_path)

    # Create a BVH
    bvh = EmbreeBVH(obj, True)

    # Set the graph parameters
    start_point = (-30, 0, 20)
    spacing = (1, 1, 10)
    max_nodes = 5000
    up_step, down_step = 5, 5
    up_slope, down_slope = 60, 60
    max_step_connections = 1

    # Generate the Graph
    graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                            up_step,up_slope,down_step,down_slope,
                            max_step_connections, cores=-1)

    # Get Nodes
    nodes = graph.getNodes()
    assert len(nodes.array) == 3450

def test_obstacle_support():
    obj_path = dhart.get_sample_model("obstacle_plane.obj")
    # time.sleep(10)
    # Load the obj file
    obj = LoadOBJ(obj_path, group_type=OBJGroupType.BY_GROUP, rotation=CommonRotations.Yup_to_Zup)

    # Create a BVH
    bvh = EmbreeBVH(obj)

    # Set the graph parameters
    start_point = (0, 0, 1)
    spacing = (0.5, 0.5, 1)
    max_nodes = 5000
    
    for o in obj: print(o)
    
    non_obstacle_graph = GenerateGraph(
        bvh, start_point, spacing, max_nodes, cores=-1
    )
    assert(non_obstacle_graph is not None)
    
    # Generate the Graph
    obstacle_graph = GenerateGraph(
        bvh, start_point, spacing, max_nodes, cores=-1, obstacle_ids=[obj[1].id]
    )
    assert(obstacle_graph is not None)

    assert(obstacle_graph.NumNodes() < non_obstacle_graph.NumNodes())

def test_step_type_query():
    # Get a sample model path
    obj_path = dhart.get_sample_model("energy_blob_zup.obj")

    # Load the obj file
    obj = LoadOBJ(obj_path)

    # Create a BVH
    bvh = EmbreeBVH(obj, True)

    # Set the graph parameters
    start_point = (0, 0, 20)
    spacing = (1, 1, 1)
    max_nodes = 5000
    up_step, down_step = 0.5, 0.5
    up_slope, down_slope = 20, 20
    max_step_connections = 1
    min_connections = 4
    cores = -1

    g = GenerateGraph(bvh, start_point, spacing, max_nodes,
                            up_step,up_slope,down_step,down_slope,
                            max_step_connections, min_connections,
                            cores)


    CalculateAndStoreStepTypes(g, bvh)

    # expected step types in format (parent, (child ids), (step types))
    expected_steps = ((0, (1,2,3,4), (3,1,2,2)), (4, (0,9,11,12), (3,3,1,2)),
                    (100, (67,143,144,145),(1,1,2,1)),
                    (200, (152,181,182,199,202,236,256,257), (3,3,2,1,1,2,1,2)),
                    (500, (431, 432, 489, 573), (1,1,1,1)), (750, (663,664, 841, 842), (1,1,1,1)),
                    (1000, (895, 896, 1111, 1112), (1,1,1,1)),
                    (1250, (1123,1124,1125,1249,1251,1388,1389,1390), (3,1,1,3,1,3,1,1)),
                    (1500, (1349,1350,1498,1499,1501,1663,1664,1665), (1,1,1,1,1,1,1,1)),
                    (2000, (1826,1827,1828,1999,2001,2179,2180,2181), (1,1,1,1,1,1,1,1)),
                    (3000, (2860,2861,2862,2999,3001,3103,3104,3105), (1,2,2,1,1,1,1,1)))
    
    for parent, children, expected_types in expected_steps:
        for i in range(len(children)):
            child = children[i]
            print(parent, child)
            expected_step_type = float(expected_types[i])
            result_step = g.GetEdgeCost(parent, child, "step_type")
            assert(result_step == expected_step_type)
    
