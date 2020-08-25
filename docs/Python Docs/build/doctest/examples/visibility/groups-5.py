VG = VisibilityGraphGroupToGroup(bvh, nodes, nodes[0:100], height) # Calculate the visibility graph
visibility_graph = VG.CompressToCSR() # Convert to a CSR (matrix)
scores = VG.AggregateEdgeCosts(2, True) # Aggregate the visibility graph scores

# Plot the graph using visibility graph as the colors
fig = plt.figure(figsize=(6,6))
plt.scatter(nodes['x'], nodes['y'], c=scores)
plt.show()