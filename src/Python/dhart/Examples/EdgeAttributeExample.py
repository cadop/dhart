from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import EmbreeBVH
from dhart.graphgenerator import GenerateGraph

import dhart

obj_path = dhart.get_sample_model("plane.obj")

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

start_point = (-1, -6, 1623.976928)

spacing = (0.5, 0.5, 0.5)
max_nodes = 500

graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)

csr_graph = graph.CompressToCSR()
nodes = graph.getNodes()

print(len(nodes))
print(nodes[0:3])