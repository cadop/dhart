"""

    >>> import matplotlib.pyplot as plt
    >>> import numpy as np
    >>> 
    >>> import humanfactorspy
    >>> from humanfactorspy.geometry import LoadOBJ, CommonRotations
    >>> from humanfactorspy.graphgenerator import GenerateGraph
    >>> from humanfactorspy.raytracer import EmbreeBVH
    >>> from humanfactorspy.pathfinding import DijkstraShortestPath
    >>> 
    >>> # Load BVH
    >>> obj_path = humanfactorspy.get_sample_model("plane.obj")
    >>> loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    >>> embree_bvh = EmbreeBVH(loaded_obj)
    >>> 
    >>> # Set graph parameters 
    >>> start_point, spacing, max_nodes  = (0, 0, 1), (1, 1, 1), 100000
    >>> 
    >>> # Generate the graph
    >>> graph = GenerateGraph(embree_bvh, start_point, spacing, max_nodes)
    >>> 
    >>> # Get Nodes
    >>> nodes = graph.getNodes()
    >>> max_node = len(nodes.array) - 1
    >>> print(f"Graph Generated with {len(nodes.array)} nodes")
    Graph Generated with 1517 nodes

    The node array comes from the struct `py:mod:humanfactorspy.spatialstructures.node`
    which defines the node as an array of [x,y,z,type,id].
    Since the nodes are stored in a numpy structured array with names we can
    extract just the x,y,z values by using the [] operator. 

    >>> xyz_nodes = nodes.array[['x','y','z']]
    >>> 
    >>> # Define a start and end node to use for the path (from,to)
    >>> start_id, end_id = 0, 100
    >>> 
    >>> # Extract the node from the set index 
    >>> start_point = xyz_nodes[start_id]
    >>> end_point   = xyz_nodes[end_id]
    >>> 
    >>> print('Start: ',start_point)
    Start:  (0., 0., 0.)
    >>> print('End:   ',end_point)
    End:    (-4., 5., 0.)

    >>> # Call the shortest path 
    >>> path = DijkstraShortestPath(graph, start_id, end_id)
    >>> print(path)
    [(1.4142135,   0) (1.4142135,   3) (1.       ,  15) (1.4142134,  36)
     (1.4142137,  64) (0.       , 100)]

    Just as the node xyz value can be extracted, the path costs and ids can be as well

    >>> # extract cost array
    >>> path_costs = path['cost_to_next']
    >>> print(path_costs)
    [1.4142135 1.4142135 1.        1.4142134 1.4142137 0.       ]
    >>> 
    >>> #extract id array
    >>> path_ids = path['id']
    >>> print(path_ids)
    [  0   3  15  36  64 100]
    >>> 
    >>> # As the cost array is numpy, simple operations to sum the total cost can be calculated
    >>> path_sum = np.sum(path_costs)
    >>> print('Total path cost: ', path_sum)
    Total path cost:  6.656854

    The graph generator guarantees the order of nodes in the array to correspond with the id
    However, you can manually find the ID of the node itself as well. 
    We can use the original structured node array to identify the location and
    value of the returned node ids from the shortest path. 

    >>> node_ids = nodes.array['id']
    >>> for c,n in path:
    ...     # find the numpy index where the node id exists (index should be same as the node id)
    ...     node_id = np.where(node_ids==n)
    ...     print('Index: ',n ,' , Node id: ',node_id, ',  is: ', nodes[node_id])
    Index:  0  , Node id:  (array([0], dtype=int64),) ,  is:  [(0., 0., 0., 0, 0)]
    Index:  3  , Node id:  (array([3], dtype=int64),) ,  is:  [(-1., 1., 0., 0, 3)]
    Index:  15  , Node id:  (array([15], dtype=int64),) ,  is:  [(-2., 2., 0., 0, 15)]
    Index:  36  , Node id:  (array([36], dtype=int64),) ,  is:  [(-2., 3., 0., 0, 36)]
    Index:  64  , Node id:  (array([64], dtype=int64),) ,  is:  [(-3., 4., 0., 0, 64)]
    Index:  100  , Node id:  (array([100], dtype=int64),) ,  is:  [(-4., 5., 0., 0, 100)]

"""

import matplotlib.pyplot as plt
import numpy as np

import humanfactorspy
from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.pathfinding import DijkstraShortestPath

# Load BVH
obj_path = humanfactorspy.get_sample_model("plane.obj")
loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
embree_bvh = EmbreeBVH(loaded_obj)

# Set graph parameters 
start_point, spacing, max_nodes  = (0, 0, 1), (1, 1, 1), 100000

# Generate the graph
graph = GenerateGraph(embree_bvh, start_point, spacing, max_nodes)

# Get Nodes
nodes = graph.getNodes()
max_node = len(nodes.array) - 1
print(f"Graph Generated with {len(nodes.array)} nodes")


# The node array comes from the struct `py:mod:humanfactorspy.spatialstructures.node`
# which defines the node as an array of [x,y,z,type,id].
# Since the nodes are stored in a numpy structured array with names we can
# extract just the x,y,z values by using the [] operator. 
xyz_nodes = nodes.array[['x','y','z']]


# Define a start and end node to use for the path (from,to)
start_id, end_id = 0, 100

# Extract the node from the set index 
start_point = xyz_nodes[start_id]
end_point   = xyz_nodes[end_id]

print('Start: ',start_point)
print('End:   ',end_point)

# Call the shortest path 
path = DijkstraShortestPath(graph, start_id, end_id)
print(path)

# Just as the node xyz value can be extracted, the path costs and ids can be as well

# extract cost array
path_costs = path['cost_to_next']
print(path_costs)

#extract id array
path_ids = path['id']
print(path_ids)

# As the cost array is numpy, simple operations to sum the total cost can be calculated
path_sum = np.sum(path_costs)
print('Total path cost: ', path_sum)

# The graph generator guarantees the order of nodes in the array to correspond with the id
# However, you can manually find the ID of the node itself as well. 
# We can use the original structured node array to identify the location and
# value of the returned node ids from the shortest path. 

node_ids = nodes.array['id']
for c,n in path:
    # find the numpy index where the node id exists (index should be same as the node id)
    node_id = np.where(node_ids==n)
    print('Index: ',n ,' , Node id: ',node_id, ',  is: ', nodes[node_id])