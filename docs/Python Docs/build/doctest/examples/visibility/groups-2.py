# Get the nodes of the graph as a list of x,y,z,type,id tuples
nodes = graph.getNodes()
print(len(nodes))

height = 1.7 # Set a height offset to cast rays from the points
VG = VisibilityGraphAllToAll(bvh, nodes, height) # Calculate the visibility graph
visibility_graph = VG.CompressToCSR() # Convert to a CSR (matrix)
scores = VG.AggregateEdgeCosts(2, True) # Aggregate the visibility graph scores

# Plot the graph using visibility graph as the colors
fig = plt.figure(figsize=(6,6))
plt.scatter(nodes['x'], nodes['y'], c=scores)
plt.show()