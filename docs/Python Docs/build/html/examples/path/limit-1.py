# Start with the required imports

import matplotlib.pyplot as plt
import numpy as np
# >>>
import humanfactorspy
from humanfactorspy.geometry import LoadOBJ
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.pathfinding import DijkstraShortestPath

# Next, load a model and create the BVH

# Get a sample model path
obj_path = humanfactorspy.get_sample_model("energy_blob_zup.obj")
# >>>
# Load the obj file
obj = LoadOBJ(obj_path)
# >>>
# Create a BVH
bvh = EmbreeBVH(obj, True)

# Now set parameters to be used for the graph generator. First, we will use a large
# step size which will ignore any small bumps in the mesh. Later on we will compare
# the resulting graphs.

# Set the graph parameters
start_point = (-30, 0, 20)
spacing = (1, 1, 10)
max_nodes = 5000
up_step, down_step = 5, 5
up_slope, down_slope = 60, 60
max_step_connections = 1
# >>>
# Generate the Graph
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)
# >>>
# Get Nodes
nodes = graph.getNodes()
print(f"Graph Generated with {len(nodes.array)} nodes")
# Graph Generated with 3450 nodes

# Once the graph is generated, we can define an array of spatial points and get the
# closest nodes of the graph to these points.

# Define a start and end point in x,y
p_desired = np.array([[-30,0],[30,0]])
closest_nodes = graph.get_closest_nodes(p_desired,z=False)
print("Closest Node: ", closest_nodes)
# Closest Node:  [   0 3123]
# >>>
# Define a start and end node to use for the path (from, to)
start_id, end_id = closest_nodes[0], closest_nodes[1]

# Now we call the shortest path algorithm on the default cost, which is euclidean distance.
# We can take the sum of the cost values in the array and print it to use as a comparison later.

# Call the shortest path
path = DijkstraShortestPath(graph, start_id, end_id)
# >>>
# As the cost array is numpy, simple operations to sum the total cost can be calculated
path_sum = np.sum(path['cost_to_next'])
print('Total path cost: ', path_sum)
# Total path cost:  62.035927

# Get the x,y,z values of the nodes at the given path ids
path_xyz = np.take(nodes[['x','y','z']], path['id'])
# >>>
# Extract the xyz locations of the nodes
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
# >>>
# Extract the xyz locations of the path nodes
x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']
# >>>
# Plot the graph
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5) # doctest: +SKIP
plt.plot(x_path,y_path, c="red", marker='.',linewidth=3) # doctest: +SKIP
plt.show() # doctest: +SKIP

# As seen in the image, the shortest path on this graph is a straight line.
