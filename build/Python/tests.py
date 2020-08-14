import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph
import humanfactorspy

# Get a sample model path
obj_path = humanfactorspy.get_sample_model("energy_blob_zup.obj")

# Load the obj file
obj = LoadOBJ(obj_path)

# Create a BVH
bvh = EmbreeBVH(obj)

# Set the graph parameters
start_point = (0, -20, 20)
spacing = (1, 1, 10)
max_nodes = 5000
up_step, down_step = 0.5, 0.5
up_slope, down_slope = 20, 20
max_step_connections = 1

# Generate the Graph
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)

# Convert the graph to a CSR
csr_graph = graph.CompressToCSR()

# Get the nodes of the graph as a list of x,y,z,type,id tuples
nodes = graph.getNodes()

# get the x,y,z coordinates of the nodes
x = [ n[0] for n in nodes ]
y = [ n[1] for n in nodes ]
z = [ n[2] for n in nodes ]

# Plot the graph
fig = plt.figure()
ax = Axes3D(fig)
ax.view_init(azim=-123, elev=15)

ax.scatter(x, y, z)
plt.show()
