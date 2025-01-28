"""
.. plot::
    :context: close-figs

    >>> import matplotlib.pyplot as plt
    >>> import numpy as np
    >>> 
    >>> import dhart
    >>> from dhart.geometry import LoadOBJ, CommonRotations
    >>> from dhart.graphgenerator import GenerateGraph
    >>> from dhart.raytracer import EmbreeBVH
    >>> from dhart.pathfinding import (DijkstraShortestPath, AlternateCostsAlongPath)
    >>> from dhart.spatialstructures.cost_algorithms import (
    ...                                     CalculateEnergyExpenditure, 
    ...                                     CalculateCrossSlope, CostAlgorithmKeys)
    >>> 
    >>> # Get a sample model path
    >>> obj_path = dhart.get_sample_model("energy_blob_zup.obj")
    >>> 
    >>> # Load the obj file
    >>> obj = LoadOBJ(obj_path)
    >>> 
    >>> # Create a BVH
    >>> bvh = EmbreeBVH(obj, True)
    >>> 
    >>> # Set the graph parameters
    >>> start_point = (-30, 0, 20)
    >>> spacing = (2, 2, 180)
    >>> max_nodes = 5000
    >>> up_step, down_step = 30, 70
    >>> up_slope, down_slope = 60, 60
    >>> max_step_connections = 1
    >>> 
    >>> # Generate the Graph
    >>> graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
    ...                         up_step,up_slope,down_step,down_slope,
    ...                         max_step_connections, cores=-1)
    >>> 
    >>> # Get Nodes
    >>> nodes = graph.getNodes()
    >>> print(f"Graph Generated with {len(nodes.array)} nodes")
    Graph Generated with 875 nodes
    >>> 
    >>> # Define a start and end point in x,y 
    >>> p_desired = np.array([[-30,0],[30,0]])
    >>> closest_nodes = graph.get_closest_nodes(p_desired,z=False)
    >>> print("Closest Node: ", closest_nodes)
    Closest Node:  [  0 786]
    >>> 
    >>> from scipy.spatial.distance import cdist
    >>> closest_nodes_all = cdist(graph.get_node_points(), graph.get_node_points())
    >>> 
    >>> # Define a start and end node to use for the path (from, to)
    >>> start_id, end_id = closest_nodes[0], closest_nodes[1]
    >>> 
    >>> # Call the shortest path using distance metrics
    >>> distance_path = DijkstraShortestPath(graph, start_id, end_id)
    >>> 
    >>> print(f"Distance Path: {distance_path}")
    Distance Path: [(2.0127509,   0) (2.0350926,   7) (2.0710974,  17) (2.1147027,  31)
     (2.154809 ,  49) (2.167964 ,  71) (2.1390352,  97) (2.099905 , 127)
     (2.0648937, 161) (2.0383816, 199) (2.0266857, 241) (2.0407124, 287)
     (2.0689316, 323) (2.1099415, 361) (2.146391 , 386) (2.1608925, 411)
     (2.13303  , 436) (2.0665207, 461) (2.0208855, 486) (2.0013084, 511)
     (2.0037231, 536) (2.022972 , 561) (2.0424194, 586) (2.0453453, 611)
     (2.0459747, 636) (2.0430908, 661) (2.0385742, 686) (2.0355377, 711)
     (2.0344658, 736) (2.0340652, 761) (0.       , 786)]
    >>> 
    >>> # We can visualize the path using matplotlib
    >>> 
    >>> # Get the x,y,z values of the nodes at the given path ids
    >>> path_xyz_distance = np.take(nodes[['x','y','z']], distance_path['id'])
    >>> 
    >>> # Extract the xyz locations of the nodes
    >>> x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
    >>> 
    >>> # Extract the xyz locations of the path nodes
    >>> x_path, y_path, z_path = path_xyz_distance['x'], path_xyz_distance['y'], path_xyz_distance['z']
    >>> 
    >>> # Plot the points
    >>> plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.3) # doctest: +SKIP
    >>> 
    >>> # Plot the path
    >>> for i in range(len(x_path) - 1):
    ...     # This segments the path and colors it based on the cost to the next node, essentially creating a heatmap of the path
    ...     plt.plot(x_path[i:i+2], y_path[i:i+2], color=plt.cm.viridis(distance_path["cost_to_next"][i] / max(distance_path["cost_to_next"])), marker='.', linewidth=3) # doctest: +SKIP
    ... 
    >>> plt.show() # doctest: +SKIP

.. plot::
    :context: close-figs
    
    >>> # Query edge costs along the path that are NOT the costs used to calculate the path
    >>> 
    >>> # First we can check the energy expenditure along the distance path 
    >>> 
    >>> energy_cost_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
    >>> 
    >>> CalculateEnergyExpenditure(graph)
    >>> 
    >>> energy_along_distance_path = AlternateCostsAlongPath(graph, CostAlgorithmKeys.ENERGY_EXPENDITURE, distance_path)
    >>> 
    >>> # Plot the points
    >>> plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.3) # doctest: +SKIP
    >>> 
    >>> # Plot the energy expenditure along the shortest distance path
    >>> for i in range(len(x_path) - 1):
    ...     plt.plot(x_path[i:i+2], y_path[i:i+2], color=plt.cm.viridis(energy_along_distance_path[i] / max(energy_along_distance_path)), marker='.', linewidth=3) # doctest: +SKIP
    ... 
    >>> plt.show() # doctest: +SKIP

.. plot::
    :context: close-figs

    >>> # Compare it to the shortest energy path
    >>> energy_path = DijkstraShortestPath(graph, start_id, end_id, energy_cost_key)
    >>> 
    >>> # The shortest energy path and the shortest distance path differ.
    >>> print(f"Distance path: {distance_path["id"]}")
    Distance path: [  0   7  17  31  49  71  97 127 161 199 241 287 323 361 386 411 436 461
    486 511 536 561 586 611 636 661 686 711 736 761 786]
    >>> 
    >>> print(f"Energy path: {energy_path["id"]}")
    Energy path: [  0   6  15  28  45  66  92 122 156 194 236 282 318 356 381 406 431 457
    482 507 532 557 582 607 632 657 682 708 734 760 786]
    >>> 
    >>> # The energy costs along each path are as follows:
    >>> print(f"Energy cost along shortest distance path: {energy_along_distance_path}")
    Energy cost along shortest distance path: [10.597378730773926, 15.275540351867676, 20.96104621887207, 27.000240325927734, 32.534175872802734, 32.732784271240234, 30.368547439575195, 24.979759216308594, 20.05685043334961, 15.855199813842773, 13.699013710021973, 16.255855560302734, 20.647640228271484, 26.349937438964844, 31.413496017456055, 32.625999450683594, 29.52669906616211, 20.295923233032227, 12.505487442016602, 6.551901817321777, 3.032193183898926, 1.8925143480300903, 2.3157668113708496, 2.428367853164673, 2.453740119934082, 2.3408164978027344, 2.182459831237793, 2.0908737182617188, 2.0618505477905273, 2.051429271697998]
    >>> 
    >>> print(f"Energy cost along shortest energy path: {energy_path["cost_to_next"]}")
    Energy cost along shortest energy path: [12.100869  14.714971  14.879747  11.407261   5.6704216 12.496231
    11.684158   9.923416   9.30909    9.04628    9.380821  11.339294
    14.562973  17.945984  20.692276  22.447952  49.174408  22.619232
    18.948212  15.3749695 12.084686   8.797546   6.2996826  5.055084
    4.1647644  3.5954285 12.745209   4.7065125  2.9411316  2.9284668
    0.       ]
    >>> 
    >>> # Visualize the difference in energy costs between the shortest distance and shortest energy path.
    >>> # Get the x,y,z values of the nodes at shortest energy path ids
    >>> path_xyz_energy = np.take(nodes[['x','y','z']], energy_path["id"])
    >>> 
    >>> # Extract the xyz locations of the path nodes
    >>> x_path_dist, y_path_dist, z_path_dist = path_xyz_distance['x'], path_xyz_distance['y'], path_xyz_distance['z']
    >>> x_path_energy, y_path_energy, z_path_energy = path_xyz_energy['x'], path_xyz_energy['y'], path_xyz_energy['z']
    >>> 
    >>> # Plot the points
    >>> plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.3)  # doctest: +SKIP
    >>> 
    >>> # Make the start and end points clear
    >>> start_end_x = [-30.,30.]
    >>> start_end_y = [0.,0.]
    >>> plt.scatter(start_end_x, start_end_y, c='black', alpha = 1.0) # doctest: +SKIP
    >>> 
    >>> # Plot the energy along the shortest distance path
    >>> for i in range(len(x_path) - 1):
    ...     plt.plot(x_path_dist[i:i+2], y_path_dist[i:i+2], color=plt.cm.viridis(energy_along_distance_path[i] / max(energy_along_distance_path)), marker='.', linewidth=3)
    ... 
    >>> # Plot the energy along the shortest energy path
    >>> for i in range(len(x_path) - 1):
    ...     plt.plot(x_path_energy[i:i+2], y_path_energy[i:i+2], color=plt.cm.viridis(energy_path["cost_to_next"][i] / max(energy_path["cost_to_next"])), marker='.', linewidth=3) # doctest: +SKIP
    ... 
    >>> plt.show() # doctest: +SKIP

 .. plot::
    :context: close-figs

    >>> # The total cost of the shortest energy path is different than the total cost of the energy along the shortest distance path.
    >>> shortest_energy_path_cost = np.sum(energy_path['cost_to_next'])
    >>> 
    >>> energy_cost_along_distance_path = np.sum(energy_along_distance_path)
    >>> 
    >>> print(f"Shortest energy path cost: {shortest_energy_path_cost}")
    Shortest energy path cost: 377.0370788574219
    >>> 
    >>> print(f"Total energy cost along distance path: {energy_cost_along_distance_path}")
    Total energy cost along distance path: 463.08348882198334

.. plot::
    :context: close-figs


"""

import matplotlib.pyplot as plt
import numpy as np

import dhart
from dhart.geometry import LoadOBJ, CommonRotations
from dhart.graphgenerator import GenerateGraph
from dhart.raytracer import EmbreeBVH
from dhart.pathfinding import (DijkstraShortestPath, AlternateCostsAlongPath)
from dhart.spatialstructures.cost_algorithms import (
                                    CalculateEnergyExpenditure, 
                                    CalculateCrossSlope, CostAlgorithmKeys)

# Get a sample model path
obj_path = dhart.get_sample_model("energy_blob_zup.obj")

# Load the obj file
obj = LoadOBJ(obj_path)

# Create a BVH
bvh = EmbreeBVH(obj, True)

# Set the graph parameters
start_point = (-30, 0, 20)
spacing = (2, 2, 180)
max_nodes = 5000
up_step, down_step = 30, 70
up_slope, down_slope = 60, 60
max_step_connections = 1

# Generate the Graph
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)

# Get Nodes
nodes = graph.getNodes()
print(f"Graph Generated with {len(nodes.array)} nodes")

# Define a start and end point in x,y 
p_desired = np.array([[-30,0],[30,0]])
closest_nodes = graph.get_closest_nodes(p_desired,z=False)
print("Closest Node: ", closest_nodes)

from scipy.spatial.distance import cdist
closest_nodes_all = cdist(graph.get_node_points(), graph.get_node_points())

# Define a start and end node to use for the path (from, to)
start_id, end_id = closest_nodes[0], closest_nodes[1]

# Call the shortest path using distance metrics
distance_path = DijkstraShortestPath(graph, start_id, end_id)

print(f"Distance Path: {distance_path}")

# We can visualize the path using matplotlib

# Get the x,y,z values of the nodes at the given path ids
path_xyz_distance = np.take(nodes[['x','y','z']], distance_path['id'])

# Extract the xyz locations of the nodes
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']

# Extract the xyz locations of the path nodes
x_path, y_path, z_path = path_xyz_distance['x'], path_xyz_distance['y'], path_xyz_distance['z']

# Plot the points
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.3) 

# Plot the path
for i in range(len(x_path) - 1):
    # This segments the path and colors it based on the cost to the next node, essentially creating a heatmap of the path
    plt.plot(x_path[i:i+2], y_path[i:i+2], color=plt.cm.viridis(distance_path["cost_to_next"][i] / max(distance_path["cost_to_next"])), marker='.', linewidth=3)

plt.show()

# Query edge costs along the path that are NOT the costs used to calculate the path

# First we can check the energy expenditure along the distance path 

energy_cost_key = CostAlgorithmKeys.ENERGY_EXPENDITURE

CalculateEnergyExpenditure(graph)

energy_along_distance_path = AlternateCostsAlongPath(graph, CostAlgorithmKeys.ENERGY_EXPENDITURE, distance_path)

# Plot the points
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.3) 

# Plot the energy expenditure along the shortest distance path
for i in range(len(x_path) - 1):
    plt.plot(x_path[i:i+2], y_path[i:i+2], color=plt.cm.viridis(energy_along_distance_path[i] / max(energy_along_distance_path)), marker='.', linewidth=3)

plt.show()

# Compare it to the shortest energy path
energy_path = DijkstraShortestPath(graph, start_id, end_id, energy_cost_key)

# The shortest energy path and the shortest distance path differ.
print(f"Distance path: {distance_path["id"]}")

print(f"Energy path: {energy_path["id"]}")

# The energy costs along each path are as follows:
print(f"Energy cost along shortest distance path: {energy_along_distance_path}")

print(f"Energy cost along shortest energy path: {energy_path["cost_to_next"]}")

# Visualize the difference in energy costs between the shortest distance and shortest energy path.
# Get the x,y,z values of the nodes at shortest energy path ids
path_xyz_energy = np.take(nodes[['x','y','z']], energy_path["id"])

# Extract the xyz locations of the path nodes
x_path_dist, y_path_dist, z_path_dist = path_xyz_distance['x'], path_xyz_distance['y'], path_xyz_distance['z']
x_path_energy, y_path_energy, z_path_energy = path_xyz_energy['x'], path_xyz_energy['y'], path_xyz_energy['z']

# Plot the points
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.3) 

# Make the start and end points clear
start_end_x = [-30.,30.]
start_end_y = [0.,0.]
plt.scatter(start_end_x, start_end_y, c='black', alpha = 1.0)

# Plot the energy along the shortest distance path
for i in range(len(x_path) - 1):
    plt.plot(x_path_dist[i:i+2], y_path_dist[i:i+2], color=plt.cm.viridis(energy_along_distance_path[i] / max(energy_along_distance_path)), marker='.', linewidth=3)

# Plot the energy along the shortest energy path
for i in range(len(x_path) - 1):
    plt.plot(x_path_energy[i:i+2], y_path_energy[i:i+2], color=plt.cm.viridis(energy_path["cost_to_next"][i] / max(energy_path["cost_to_next"])), marker='.', linewidth=3)

plt.show()

# The total cost of the shortest energy path is different than the total cost of the energy along the shortest distance path.
shortest_energy_path_cost = np.sum(energy_path['cost_to_next'])

energy_cost_along_distance_path = np.sum(energy_along_distance_path)

print(f"Shortest energy path cost: {shortest_energy_path_cost}")

print(f"Total energy cost along distance path: {energy_cost_along_distance_path}")