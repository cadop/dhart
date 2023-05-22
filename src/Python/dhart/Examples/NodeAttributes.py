"""

.. plot::
    :context: reset


    import matplotlib.pyplot as plt
    import numpy as np

    import dhart
    from dhart.geometry import LoadOBJ
    from dhart.graphgenerator import GenerateGraph
    from dhart.raytracer import EmbreeBVH
    from dhart.pathfinding import DijkstraShortestPath
    from dhart.spatialstructures import Direction
    from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

    
.. plot::
    :context: close-figs


    # Get a sample model path
    obj_path = dhart.get_sample_model("VisibilityTestCases.obj")

    # Load the obj file
    obj = LoadOBJ(obj_path)

    # Create a BVH
    bvh = EmbreeBVH(obj, True)

    # Set the graph parameters
    start_point = (1.1 , 1.1, 20) 
    spacing = (1, 1, 5) 
    max_nodes = 10000
    up_step, down_step = 0.1, 0.1
    up_slope, down_slope = 1, 1
    max_step_connections = 1

    # Generate the Graph
    graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                            up_step,up_slope,down_step,down_slope,
                            max_step_connections, cores=-1)
    # Get Nodes
    nodes = graph.getNodes()
    print(f"Graph Generated with {len(nodes.array)} nodes")
    
    
.. plot::
    :context: close-figs


    # Define a start and end point in x,y 
    p_desired = np.array([[0,0],[35,35]])
    closest_nodes = graph.get_closest_nodes(p_desired,z=False)

    # Define a start and end node to use for the path (from, to)
    start_id, end_id = closest_nodes[0], closest_nodes[1]


    
.. plot::
    :context: close-figs


    # Call the shortest path 
    path = DijkstraShortestPath(graph, start_id, end_id)

    # As the cost array is numpy, simple operations to sum the total cost can be calculated
    path_sum = np.sum(path['cost_to_next'])
    print('Total path cost: ', path_sum)
   
    
.. plot::
    :context: close-figs


    # Get the x,y,z values of the nodes at the given path ids
    path_xyz = np.take(nodes[['x','y','z']], path['id'])

    # Extract the xyz locations of the nodes
    x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']

    # Extract the xyz locations of the path nodes
    x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']

    # Plot the graph
    fig = plt.figure(figsize=(6,6))
    plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5)
    plt.plot(x_path,y_path, c="red", marker='.',linewidth=3)
    plt.show()

While energy expenditure is a built-in attribute for path planning, any numerical node attribute
can be used for path planning as well. 

.. plot::
    :context: close-figs

    height = 1.7 # Set a height offset to cast rays from the points
    ray_count = 100 # Set the number of rays to use per node
    scores = SphericalViewAnalysisAggregate(bvh, nodes, ray_count, height, 
                                                        upward_fov = 20, downward_fov=20, 
                                                        agg_type=AggregationType.AVERAGE)

                                                        
.. plot::
    :context: close-figs


    # add scores to node attributes
    # This is our node attribute name, we can use it later to refer to this data
    attr_name = "vg_group"
    ids = graph.getNodes().array['id']
    edge_cost_name = "vg_group_cost"

    # We need scores to be a string for dhart. 
    # While this has some overhead, it lets us use node attributes in a flexible way

    str_scores = [str(1.0/(x+0.01)) for x in scores] 
    # We need an attribute name, the node ids, and an equally sized list of values to assign
    graph.add_node_attributes(attr_name, ids, str_scores)

    
This function takes in our node attribute, which we previously called "vg_group" and convert it to 
costs of the graph. These costs we will call "vg_group_cost" for this example.  
Finally, the third input is a direction. Since these attributes are set on a node and not an edge, 
we need to define which node attribute an edge gets. We do this with a directed graph, and set the 
node attribute on either the incoming or outgoing edge. 

    
.. plot::
    :context: close-figs

    graph.attrs_to_costs(attr_name, edge_cost_name, Direction.INCOMING)

    # We can pick some node ids to define an edge, and query the cost of our custom attribute
    sample_cost = graph.GetEdgeCost(1, 2, edge_cost_name)

    
.. testcode::

    print(f'edge cost {sample_cost}')

    
.. plot::
    :context: close-figs

    # get custom path based on vg
    p_desired = np.array([[0,0],[35,35]])
    closest_nodes = graph.get_closest_nodes(p_desired, z=False)

    # Call the shortest path again, with the optional cost type
    visibility_path = DijkstraShortestPath(graph, closest_nodes[0], closest_nodes[1], edge_cost_name)
    path_xyz = np.take(nodes[['x','y','z']], visibility_path['id'])

    # Extract the xyz locations of the nodes
    x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
    # Extract the xyz locations of the path nodes
    x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']

    # Plot the graph
    fig = plt.figure(figsize=(6,6))
    plt.scatter(x_nodes, y_nodes, c=scores, alpha=0.5)
    plt.plot(x_path,y_path, c="red", marker='.',linewidth=3)
    plt.show()

"""

import matplotlib.pyplot as plt
import numpy as np

import dhart
from dhart.geometry import LoadOBJ
from dhart.graphgenerator import GenerateGraph
from dhart.raytracer import EmbreeBVH
from dhart.pathfinding import DijkstraShortestPath
from dhart.spatialstructures import Direction
from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

# Get a sample model path
obj_path = dhart.get_sample_model("VisibilityTestCases.obj")

# Load the obj file
obj = LoadOBJ(obj_path)

# Create a BVH
bvh = EmbreeBVH(obj, True)

# Set the graph parameters
start_point = (1.1 , 1.1, 20) 
spacing = (1, 1, 5) 
max_nodes = 10000
up_step, down_step = 0.1, 0.1
up_slope, down_slope = 1, 1
max_step_connections = 1

# Generate the Graph
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)
# Get Nodes
nodes = graph.getNodes()
print(f"Graph Generated with {len(nodes.array)} nodes")

# Define a start and end point in x,y 
p_desired = np.array([[0,0],[35,35]])
closest_nodes = graph.get_closest_nodes(p_desired,z=False)

# Define a start and end node to use for the path (from, to)
start_id, end_id = closest_nodes[0], closest_nodes[1]

# Call the shortest path 
path = DijkstraShortestPath(graph, start_id, end_id)

# As the cost array is numpy, simple operations to sum the total cost can be calculated
path_sum = np.sum(path['cost_to_next'])
print('Total path cost: ', path_sum)

# Get the x,y,z values of the nodes at the given path ids
path_xyz = np.take(nodes[['x','y','z']], path['id'])

# Extract the xyz locations of the nodes
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']

# Extract the xyz locations of the path nodes
x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']

# Plot the graph using visibility graph as the colors
fig = plt.figure(figsize=(6,6))
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5)
plt.plot(x_path,y_path, c="red", marker='.',linewidth=3)
plt.show()

# While energy expenditure is a built-in attribute for path planning, any numerical node attribute
# can be used for path planning as well. 

height = 1.7 # Set a height offset to cast rays from the points
ray_count = 100 # Set the number of rays to use per node
scores = SphericalViewAnalysisAggregate(bvh, nodes, ray_count, height, 
                                                    upward_fov = 20, downward_fov=20, 
                                                    agg_type=AggregationType.AVERAGE)

# add scores to node attributes
# This is our node attribute name, we can use it later to refer to this data
attr_name = "vg_group"
ids = graph.getNodes().array['id']
edge_cost_name = "vg_group_cost"

# We need scores to be a string for dhart. 
# While this has some overhead, it lets us use node attributes in a flexible way

str_scores = [str(1.0/(x+0.01)) for x in scores] 
# We need an attribute name, the node ids, and an equally sized list of values to assign
graph.add_node_attributes(attr_name, ids, str_scores)

# This function takes in our node attribute, which we previously called "vg_group" and convert it to 
# costs of the graph. These costs we will call "vg_group_cost" for this example.  
# Finally, the third input is a direction. Since these attributes are set on a node and not an edge, 
# we need to define which node attribute an edge gets. We do this with a directed graph, and set the 
# node attribute on either the incoming or outgoing edge. 
graph.attrs_to_costs(attr_name, edge_cost_name, Direction.INCOMING)

# We can pick some node ids to define an edge, and query the cost of our custom attribute
sample_cost = graph.GetEdgeCost(1, 2, edge_cost_name)

print(f'edge cost {sample_cost}')

# get custom path based on vg
p_desired = np.array([[0,0],[35,35]])
closest_nodes = graph.get_closest_nodes(p_desired, z=False)

# Call the shortest path again, with the optional cost type
visibility_path = DijkstraShortestPath(graph, closest_nodes[0], closest_nodes[1], edge_cost_name)
path_xyz = np.take(nodes[['x','y','z']], visibility_path['id'])

# Extract the xyz locations of the nodes
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
# Extract the xyz locations of the path nodes
x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']


fig = plt.figure(figsize=(6,6))
plt.scatter(x_nodes, y_nodes, c=scores, alpha=0.5)
plt.plot(x_path,y_path, c="red", marker='.',linewidth=3)
plt.show()