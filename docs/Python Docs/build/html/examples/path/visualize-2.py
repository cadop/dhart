# Repeat the process with a new graph based on different parameters and same locations
# We change the graph parameters to limit a 'step'.

# Change some graph parameters
up_step, down_step = 0.2, 0.5
# >>>
# Generate a new Graph
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)

# We now get the nodes of the new graph, and can see the number of nodes are the same.

nodes = graph.getNodes()
print(f"Graph Generated with {len(nodes.array)} nodes")
# Graph Generated with 3453 nodes

# If we now get the closest node, it is now a different index. The key here is that the limitations
# of the graph generator step size have caused certain edges to not be valid, which changed the order
# in which nodes were created.

closest_nodes = graph.get_closest_nodes(p_desired,z=False)
print("Closest Node: ", closest_nodes)
# Closest Node:  [   0 2614]
# >>>
start_id, end_id = closest_nodes[0], closest_nodes[1]
path = DijkstraShortestPath(graph, start_id, end_id)

# If we sum the costs of the path, we see it has now increased. Important to note, we did not
# change the cost type when querying the shortest path. Instead, it is the lack of edge
# connections due to the graph generator settings. While the total number of nodes are the same,
# there a missing edges, which forces the shortest path by distance to travel around the
# thresholds defined when creating the graph.

# As the cost array is numpy, simple operations to sum the total cost can be calculated
path_sum = np.sum(path['cost_to_next'])
print('Total path cost: ', path_sum)
# Total path cost:  81.42913

# Now extract the location data for the nodes and path to be plotted.

path_xyz = np.take(nodes[['x','y','z']], path['id'])
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']
# >>>
# Plot the graph
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5) # doctest: +SKIP
plt.plot(x_path,y_path, c="red", marker='.',linewidth=3) # doctest: +SKIP
plt.show() # doctest: +SKIP

# Seen in the image, the path avoids the center mound as it was too steep for the graph
# generator to run on.
