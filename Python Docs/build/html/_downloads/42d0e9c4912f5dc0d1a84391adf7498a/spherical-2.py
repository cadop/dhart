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

#Repeat with a smaller fov

upfov, downfov = 15, 15
hit_points = SphericalViewAnalysis(bvh, query_point, ray_count, height, upward_fov=upfov, downward_fov=downfov)
hit_dirs = SphericallyDistributeRays(ray_count, upward_fov=upfov, downward_fov=downfov)

# Get index where hitpoints are not -1
hit_idx = np.where(hit_points['distance'] != -1)
#pull out just the distances and the directions
hit_points = hit_points['distance'][hit_idx]
hit_dirs_valid = hit_dirs[hit_idx]

# reshape to get the correct axis
hit_pos = hit_dirs_valid * hit_points.reshape(-1,1)