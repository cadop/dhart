import tkinter as tk
from tkinter import filedialog
import numpy

# import json

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

# Try to load ujson since it's really fast
try:
    import ujson as json
except:
    import json as json

    print("Ujson not detected. Using standard, slower python json writer ")

obj_path = "H:/weston_y_up_decimated.obj"
ray_count = int(pow(10, 4.5))
height = 120
start_point = (-1, -6, 1623.976928)
spacing = (20, 20, 20)
max_nodes = 500000

upper_fov = 50
lower_fov = 70

agg_type = AggregationType.AVERAGE

if obj_path is None:
    root = tk.Tk()
    root.withdraw()
    obj_path = filedialog.askopenfilename()
    print(obj_path)

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

print("Generating Graph")
graph = GenerateGraph(
    bvh,
    start_point,
    spacing,
    max_nodes,
    up_step=20,
    down_step=20,
    max_step_connections=2,
    cores=-1,
)
if not graph:
    print("No graph generated!")
    exit()

print("Converting to lists")
nodes, edges = graph.ConvertToLists()

print(f"Nodes: {len(nodes)}, Edges:{len(edges)}")

json_dict = {}
json_dict["nodes"] = nodes
json_dict["edges"] = edges


print(f"Performing View Analysis for {len(nodes) * ray_count} rays")
va = SphericalViewAnalysisAggregate(
    bvh, graph.getNodes(), ray_count, height, upper_fov, lower_fov, agg_type
)
json_dict["va_score"] = va.array.tolist()

with open("out_graph.json", "w") as out_json:
    json.dump(json_dict, out_json)
