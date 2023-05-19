import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

from dhart.geometry import LoadOBJ
from dhart.raytracer import EmbreeBVH
from dhart.graphgenerator import GenerateGraph
from dhart.visibilitygraph import VisibilityGraphAllToAll, VisibilityGraphGroupToGroup
import dhart as hfpy

# Get a sample model path
obj_path = hfpy.get_sample_model("VisibilityTestCases.obj")

# Load the obj file
obj = LoadOBJ(obj_path)

# Create a BVH
bvh = EmbreeBVH(obj, True)

# Set the graph parameters
# Test examples are failing on 1. Seems to be embree failing on edge intersection
start_point = (1.1 , 1.1, 20) #offset X by 60 for each new model
spacing = (1, 1, 5)
max_nodes = 10000
up_step, down_step = 0.1, 0.1
up_slope, down_slope = 1, 1
max_step_connections = 1

# Generate the Graph
graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                        up_step,up_slope,down_step,down_slope,
                        max_step_connections, cores=-1)