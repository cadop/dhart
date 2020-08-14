import tkinter as tk
from tkinter import filedialog
import numpy

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.visibilitygraph import VisibilityGraphUndirectedAllToAll
from humanfactorspy.spatialstructures.graph import CostAggregationType

import humanfactorspy

# Try to load ujson since it's really fast
try:
    import orjson as this_json
    from sys import setrecursionlimit
    setrecursionlimit(99999)

except:
    import json as this_json
    print("Ujson not detected. Using standard, slower python json writer ")

obj_path = humanfactorspy.get_sample_model("Weston_Analysis.obj")
ray_count = int(pow(10, 4.5))
height = 120
start_point = (-1, -6, 1623.976928)
spacing = (20, 20, 20)
max_nodes = 1000000

agg_type = CostAggregationType.COUNT
if obj_path is None:
    root = tk.Tk()
    root.withdraw()
    obj_path = filedialog.askopenfilename()
    print(obj_path)

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

print("Generating Graph")
graph = GenerateGraph(bvh,
                    start_point,
                    spacing,
                    max_nodes,
                    up_step=20,
                    down_step=20,
                    max_step_connections=2,
                    cores=-1)
if not graph:
    print("No graph generated!")
    exit()

print("Converting to lists")
nodes, edges = graph.ConvertToLists()

print(f"Nodes: {len(nodes)}, Edges:{len(edges)}")

json_dict = {}
json_dict["nodes"] = nodes
json_dict["edges"] = edges

points = graph.getNodes()

print(f"Firing {len(points) * len(points)} rays")
VG = VisibilityGraphUndirectedAllToAll(bvh, points, height)

print("Aggregating.")
VG.CompressToCSR()
json_dict["vg_sum"] = VG.AggregateEdgeCosts(CostAggregationType.SUM, False).array.tolist()
json_dict["vg_count"] = VG.AggregateEdgeCosts(CostAggregationType.COUNT, False).array.tolist()
json_dict["vg_avg"] = VG.AggregateEdgeCosts(CostAggregationType.AVERAGE, False).array.tolist()

print("Writing")
json_dump = this_json.dumps(json_dict)
write_string = "w"

# Certain, faster json writers only deal in bytes so 
# change our write type if needed
if isinstance(json_dump, str):
    write_string = "w"
else:
    write_string = "wb"

with open("out_vg_score.json", write_string) as out_json:
     out_json.write(json_dump)   
