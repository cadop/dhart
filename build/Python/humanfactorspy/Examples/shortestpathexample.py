from humanfactorspy import get_sample_model
from humanfactorspy.geometry.obj_loader import LoadOBJ
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.pathfinding import DijkstraShortestPath
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.spatialstructures.cost_algorithms import (
    CalculateEnergyExpenditure, CostAlgorithmKeys)

# Load the energy blob and create a BVH from it
energy_blob_path = get_sample_model("energy_blob_zup.obj")
energy_blob_mesh = LoadOBJ(energy_blob_path)
bvh = EmbreeBVH(energy_blob_mesh)

# Define graph generator parameters
start_point = (-30, 0, 20)
spacing = (1, 1, 10)
max_nodes = 10000
up_step, down_step = 5, 5
up_slope, down_slope = 60, 60
max_step_connections = 1

# Generate a graph on it
g = GenerateGraph(bvh, start_point, spacing, max_nodes,
                  up_step, up_slope, down_step, down_slope,
                  max_step_connections, cores=-1)

# Compress the graph
g.CompressToCSR()

# Generate an alternate cost type and store it in the graph
CalculateEnergyExpenditure(g)

# Generate a path using the energy expenditure cost and distance (the default)
start_point = 1
end_point = 150
energy_expend_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
distance_path = DijkstraShortestPath(g, start_point, end_point)
energy_path = DijkstraShortestPath(
    g, start_point, end_point, energy_expend_key)

# Print both paths
print()
print("Distance Path:", distance_path.array)
print("Energy Path:", energy_path.array)
