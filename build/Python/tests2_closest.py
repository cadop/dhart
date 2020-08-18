import matplotlib.pyplot as plt
import numpy as np

import humanfactorspy
from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.pathfinding import DijkstraShortestPath

# Load BVH
obj_path = humanfactorspy.get_sample_model("energy_blob_zup.obj")
loaded_obj = LoadOBJ(obj_path)
embree_bvh = EmbreeBVH(loaded_obj)

# Set graph parameters 
start_point, spacing, max_nodes = (-30, 0, 20), (1, 1, 10), 100000

# Generate the graph
graph = GenerateGraph(embree_bvh, start_point, spacing, max_nodes)

closest_node = graph.get_closest_nodes(np.array([30,0]), z=False)
print("Closest Node: ", closest_node)

closest_node = graph.get_closest_nodes(np.array([[30,0],[20,20]]), z=False)
print("Closest Node: ", closest_node)

closest_node = graph.get_closest_nodes(np.array([30,0,0]))
print("Closest Node: ", closest_node)

closest_node = graph.get_closest_nodes(np.array([[30,0,0],[20,20,0]]))
print("Closest Node: ", closest_node)