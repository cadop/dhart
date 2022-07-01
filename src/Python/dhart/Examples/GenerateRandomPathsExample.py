import random

from dhart.geometry import LoadOBJ, CommonRotations
from dhart.graphgenerator import GenerateGraph
from dhart.raytracer import EmbreeBVH
from dhart.pathfinding import DijkstraShortestPath

import dhart

# Load BVH
obj_path = dhart.get_sample_model("plane.obj")

loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
embree_bvh = EmbreeBVH(loaded_obj)

# Create graph
start_point = (0, 0, 1)
spacing = (0.01, 0.01, 0.01)
max_nodes = 100000
graph = GenerateGraph(embree_bvh, start_point, spacing, max_nodes)

# Get Nodes
nodes = graph.getNodes()
max_node = len(nodes.array) - 1
print(f"Graph Generated with {len(nodes.array)} nodes")

# Create random start/endpoints
num_paths = 100
start_points = [
    int(nodes.array[random.randint(0, max_node)][4]) for i in range(0, num_paths)
]
end_points = [
    int(nodes.array[random.randint(0, max_node)][4]) for i in range(0, num_paths)
]

# Generate shortest paths for start/end points
print(f"Generating {num_paths} paths")
paths = DijkstraShortestPath(graph, start_points, end_points)

print(f"Calculating stats")
# Calculate stats
total_nodes = 0
for path in paths:
    if path is not None:
        total_nodes += len(path.array)
print(
    f"Total Nodes:{total_nodes} | Total Paths: {len(paths)} | Average Length { total_nodes/len(paths)}"
)