import tkinter as tk
from tkinter import filedialog
import numpy

# import json

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH, IntersectOccluded
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.visibilitygraph import VisibilityGraphUndirectedAllToAll, VisibilityGraphGroupToGroup
from humanfactorspy.spatialstructures.graph import CostAggregationType
from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

# Try to load ujson since it's really fast
try:
    import orjson as this_json
    from sys import setrecursionlimit

    setrecursionlimit(99999)
except:
    import json as this_json
    print("Ujson not detected. Using standard, slower python json writer ")


json_dict = {}


def getBVH(obj_path):
    if obj_path is None:
        root = tk.Tk()
        root.withdraw()
        obj_path = filedialog.askopenfilename()
        print(obj_path)

    obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)
    return bvh


def CreateGraph(bvh, ray_count, height, start_point, spacing, max_nodes):
    print("Generating Graph")
    graph = GenerateGraph(
        bvh,
        start_point,
        spacing,
        max_nodes,
        up_step=50,
        down_step=50,
        max_step_connections=2,
        cores=-1,
    )
    if not graph:
        print("No graph generated!")
        exit()

    print("Converting to lists")
    nodes, edges = graph.ConvertToLists()

    print(f"Nodes: {len(nodes)}, Edges:{len(edges)}")

    json_dict["nodes"] = nodes
    json_dict["edges"] = edges

    return graph


def GenerateVisibilityGraph(points, height=1.7):
    global json_dict
    print(f"Firing {len(points) * len(points)} rays for Visibility Graph")
    VG = VisibilityGraphUndirectedAllToAll(bvh, points, height)
    
    print("Aggregating.")
    VG.CompressToCSR()
    
    json_dict["nodes"] = VG.ConvertToLists(True) # Right now, vg doesn't order nodes the same way they were input
    json_dict["vg_sum"] = VG.AggregateEdgeCosts(
        CostAggregationType.SUM, False
    ).array.tolist()
    json_dict["vg_count"] = VG.AggregateEdgeCosts(
        CostAggregationType.COUNT, False
    ).array.tolist()
    json_dict["vg_avg"] = VG.AggregateEdgeCosts(
        CostAggregationType.AVERAGE, False
    ).array.tolist()

def GenerateGroupedVisibilityGraph(group_a, group_b, height=1.7, directed =False):
    global json_dict
    print(f"Firing {len(group_a) * len(group_b)} rays for Visibility Graph")
    VG = VisibilityGraphGroupToGroup(bvh, group_a, group_b, height)
    if (not VG):
        print("No visibility graph could be generated!")
        exit()

    print("Aggregating.")
    VG.CompressToCSR()
    json_dict["nodes"] = VG.ConvertToLists(True) # Right now, vg doesn't order nodes the same way they were input
    json_dict["vg_sum"] = VG.AggregateEdgeCosts(
        CostAggregationType.SUM, directed
    ).array.tolist()
    json_dict["vg_count"] = VG.AggregateEdgeCosts(
        CostAggregationType.COUNT, directed
    ).array.tolist()
    json_dict["vg_avg"] = VG.AggregateEdgeCosts(
        CostAggregationType.AVERAGE, directed
    ).array.tolist()


def RunViewAnalysis(
    points,
    num_rays,
    height,
    agg_type=AggregationType.AVERAGE,
    upper_fov=50.0,
    lower_fov=70.0,
):
    global json_dict
    print(f"Performing View Analysis for {len(points) * ray_count} rays")
    va = SphericalViewAnalysisAggregate(
        bvh, points, ray_count, height, upper_fov, lower_fov, agg_type
    )
    json_dict["va_score"] = va.array.tolist()


def SeperateIndoor(nodes, height_cutoff = -1):
    """ Fire a ray directly upwards from each node to tell if it's indoors or not """
    indoors = []
    outdoors = []

    for node in nodes:
        res = IntersectOccluded(bvh, node, (0,0,1))
        if res and node[2] > height_cutoff:
            indoors.append(node)
        else:
            outdoors.append(node)

    return (indoors, outdoors)


if __name__ == "__main__":
    # Global
    obj_path = "H:/ButchersDenFinal.obj"
    height = 100

    # Graph
    start_point = (0, 0, 0)
    spacing = (12, 12, 50)
    max_nodes = 300000


    # Visibilitygraph Analysis
    visibility_graph_agg_type = CostAggregationType.COUNT

    # View Analysis
    view_analysis_agg_type = AggregationType.AVERAGE
    ray_count = int(pow(10, 4))
    up_fov = 50
    down_fov = 70

    bvh = getBVH(obj_path)
    graph = CreateGraph(bvh, ray_count, height, start_point, spacing, max_nodes)
    points = graph.getNodes()
    
    # RunViewAnalysis(points, ray_count, height, view_analysis_agg_type, up_fov, down_fov)
    
    indoor, outdoor = SeperateIndoor(points, 31)
    # GenerateVisibilityGraph(points)
    # GenerateVisibilityGraph(indoor)
    GenerateGroupedVisibilityGraph(outdoor, indoor, height, True)

    print("Writing")
    print(json_dict.keys())
    with open("out_graph.json", "wb") as out_json:
        out_json.write(this_json.dumps(json_dict))
