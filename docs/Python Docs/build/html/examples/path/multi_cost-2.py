# Compared to the example in which we modify the graph, here we will just use a different
# edge cost for the calculation.

# Calculate energy expenditure of the graph edges, which internally allows access to this weight
CalculateEnergyExpenditure(graph)
# >>>
# Get the key
energy_cost_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
# >>>
# Call the shortest path again, with the optional cost type
energy_path = DijkstraShortestPath(graph, start_id, end_id, energy_cost_key)
# >>>
# As the cost array is numpy, simple operations to sum the total cost can be calculated
path_sum = np.sum(energy_path['cost_to_next'])
print('Total path cost: ', path_sum)
# Total path cost:  377.05716

# Get the x,y,z values of the nodes at the given path ids
path_xyz = np.take(nodes[['x','y','z']], energy_path['id'])
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
