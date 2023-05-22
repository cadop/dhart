# Plot the graph in 3D
fig = plt.figure()
ax = fig.add_subplot(projection='3d')
ax.view_init(azim=-140, elev=30)
# Set limits
ax.axes.set_xlim3d(left=-15, right=10)
ax.axes.set_ylim3d(bottom=-15, top=10)
ax.axes.set_zlim3d(bottom=-0, top=25)

# Pull out hitpoints
x,y,z = hit_pos[:,0], hit_pos[:,1], hit_pos[:,2]
# Assign points
ax.scatter(x,y,z, s = 0.5)

# Draw figure
plt.show()