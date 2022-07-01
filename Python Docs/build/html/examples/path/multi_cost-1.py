import matplotlib.pyplot as plt
import numpy as np
import dhart
from dhart.geometry import LoadOBJ, CommonRotations
from dhart.graphgenerator import GenerateGraph
from dhart.raytracer import EmbreeBVH
from dhart.pathfinding import DijkstraShortestPath
from dhart.spatialstructures.cost_algorithms import (
                                    CalculateEnergyExpenditure, CostAlgorithmKeys)
#
obj_path = dhart.get_sample_model("energy_blob_zup.obj")
obj = LoadOBJ(obj_path)
bvh = EmbreeBVH(obj, True)
start_point = (-30, 0, 20)
spacing = (2, 2, 180)
max_nodes = 5000
up_step, down_step = 30, 70
up_slope, down_slope = 60, 60
max_step_connections = 1
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)
#
nodes = graph.getNodes()
print(f"Graph Generated with {len(nodes.array)} nodes")
# Expected:
## Graph Generated with 875 nodes
#
p_desired = np.array([[-30,0],[30,0]])
closest_nodes = graph.get_closest_nodes(p_desired,z=False)
print("Closest Node: ", closest_nodes)
# Expected:
## Closest Node:  [  0 786]
#
from scipy.spatial.distance import cdist
closest_nodes_all = cdist(graph.get_node_points(), graph.get_node_points())
start_id, end_id = closest_nodes[0], closest_nodes[1]
path = DijkstraShortestPath(graph, start_id, end_id)
path_sum = np.sum(path['cost_to_next'])
print('Total path cost: ', path_sum)
# Expected:
## Total path cost:  62.0201
#
path_xyz = np.take(nodes[['x','y','z']], path['id'])
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5) # doctest: +SKIP
plt.plot(x_path,y_path, c="red", marker='.',linewidth=3) # doctest: +SKIP
plt.show() # doctest: +SKIP
