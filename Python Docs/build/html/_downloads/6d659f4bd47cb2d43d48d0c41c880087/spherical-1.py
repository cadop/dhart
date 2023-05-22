import matplotlib.pyplot as plt
import numpy as np

from dhart.geometry import LoadOBJ
from dhart.raytracer import EmbreeBVH
from dhart.viewanalysis import SphericalViewAnalysis, SphericallyDistributeRays
import dhart as hfpy

# Get a sample model path
obj_path = hfpy.get_sample_model("VisibilityTestCase.obj")

# Load the obj file
obj = LoadOBJ(obj_path)

# Create a BVH
bvh = EmbreeBVH(obj)

ray_count = 10000 # Set the number of rays to use per node

# Set the up and down field of view limits
upfov, downfov = 40, 40

# Set a height offset to cast rays from the points
height = 5
query_point = [(25,25,0)]


# Get distances from the desired point to the model (bvh)
hit_points = SphericalViewAnalysis(bvh, query_point, ray_count, height, upward_fov=upfov, downward_fov=downfov)


# Get the directions that were used for the view analysis function
hit_dirs = SphericallyDistributeRays(ray_count, upward_fov=upfov, downward_fov=downfov)

# Get index where hitpoints are not -1
hit_idx = np.where(hit_points['distance'] != -1)
#pull out just the distances and the directions
hit_dirs_valid = hit_dirs[hit_idx]
hit_points = hit_points['distance'][hit_idx]

# reshape to get the correct axis
hit_pos = hit_dirs_valid * hit_points.reshape(-1,1)