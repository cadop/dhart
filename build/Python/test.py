from humanfactorspy.pathfinding import DijkstraShortestPath, DijkstraFindAllShortestPaths
from humanfactorspy.spatialstructures import Graph
import numpy as np

g = Graph()
g.AddEdgeToGraph(0, 1, 100)
g.AddEdgeToGraph(0, 2, 50)
g.AddEdgeToGraph(1, 3, 10)
g.AddEdgeToGraph(2, 1, 10)
g.AddEdgeToGraph(3, 4, 10)
g.AddEdgeToGraph(4, 1, 10)
g.AddEdgeToGraph(4, 2, 10)
g.AddEdgeToGraph(2, 4, 10)
csr = g.CompressToCSR()

# Size of G
g_size = len(g.getNodes())

# Get APSP
SP = DijkstraFindAllShortestPaths(g)

# Reshape APSP result to nxn of graph size
apsp_mat = np.reshape(SP, (g_size,g_size))

for i, row in enumerate(apsp_mat):
    print('Paths from node: ',i)
    for j, path in enumerate(row):
        if path is not None:
            print('To node: ',j, ' = ', path['id'])
        else: 
            print('To node: ',j, ' =   ')
    
# Convert to a distance matrix
# Create an empty array the size of the graph
# Then set distance to infinity if there is no path
# and 0 if it is the distance to itself
dst = np.empty((g_size,g_size))
for i, path in enumerate(SP):
    if path is None: 
        if int(i/g_size) == i%g_size: dist = 0
        else: dist = np.inf
    else: dist = np.sum(path['cost_to_next'])
    dst[int(i/g_size)][i%g_size] = dist

print(dst)