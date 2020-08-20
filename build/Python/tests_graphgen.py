from humanfactorspy.geometry import LoadOBJ
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph

import humanfactorspy

# Try to load ujson since it's really fast. Otherwise use normal json
try:
    import ujson as json
except:
    import json as json
    print("Ujson not detected. Using standard, slower python json writer ")

obj_path = humanfactorspy.get_sample_model("Weston_meshed_no-ngon.obj")

obj = LoadOBJ(obj_path)
bvh = EmbreeBVH(obj,True)

start_point = (-1, -6, 660)
# start_point = (2519,614,660)
spacing = (20, 20, 70)
max_nodes = 50000

graph = GenerateGraph(bvh, start_point, spacing, max_nodes, 
                        up_step=20, down_step=20, up_slope=40,
                        down_slope= 1, max_step_connections=1, cores=4)

csr_graph = graph.CompressToCSR()
nodes = graph.getNodes()

print(len(nodes))
print(nodes[0:3])

obj_path = None
json_out_path = "out_graph_weston.json"
# Convert nodes/edges of the graph to lists
print("Converting to lists")
nodes, edges = graph.ConvertToLists()
print(f"Nodes: {len(nodes)}, Edges:{len(edges)}")


print("Writing to JSON at", json_out_path)
json_dict = {}
json_dict["nodes"] = nodes
json_dict["edges"] = edges
with open(json_out_path, "w") as out_json:
    json.dump(json_dict, out_json)