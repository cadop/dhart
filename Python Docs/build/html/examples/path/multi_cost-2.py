# Compared to the example in which we modify the graph, here we will just use a different
# edge cost for the calculation.
#
CalculateEnergyExpenditure(graph)
energy_cost_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
energy_path = DijkstraShortestPath(graph, start_id, end_id, energy_cost_key)
path_sum = np.sum(energy_path['cost_to_next'])
print('Total path cost: ', path_sum)
# Expected:
## Total path cost:  377.03708
#
path_xyz = np.take(nodes[['x','y','z']], energy_path['id'])
x_nodes, y_nodes, z_nodes = nodes['x'], nodes['y'], nodes['z']
x_path, y_path, z_path = path_xyz['x'], path_xyz['y'], path_xyz['z']
plt.scatter(x_nodes, y_nodes, c=z_nodes, alpha=0.5) # doctest: +SKIP
plt.plot(x_path,y_path, c="red", marker='.',linewidth=3) # doctest: +SKIP
plt.show() # doctest: +SKIP
