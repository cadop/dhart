# Plot the graph in 3D
fig = plt.figure()
ax = Axes3D(fig)
ax.view_init(azim=-123, elev=15)

ax.scatter(x, y, z)
plt.show()