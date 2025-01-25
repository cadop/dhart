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
    mesh_path = dhart.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    g = Graph()

    g.AddEdgeToGraph((0,0,1), (0,2,0), -1)
    g.AddEdgeToGraph((0,0,1), (2,0,0), -1)

    up_step, up_slope = 2, 45
    down_step, down_slope = 2, 45
    node_z = 0.01
    ground_offset = 0.01

    CalculateAndStoreStepTypes(g, bvh)

    expected_steps = [1,1]
    parent = 0
    for child in range(1, 3):
        print(g.GetEdgeCost(parent, child, "step_type"))
        result_step = g.GetEdgeCost(parent, child, "step_type")
        assert(result_step == expected_steps[child-1])
    
