import time
import math
import multiprocessing

import numpy as np
import matplotlib.pyplot as plt

from hfea.utils import interactions as act
from hfea.utils import geomUtils as gu
from hfea.utils import rayUtils as ru
from hfea.utils import graphData as gd

from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate
from humanfactorspy.viewanalysis import SphericalViewAnalysis
from humanfactorspy.viewanalysis import SphericallyDistributeRays
from humanfactorspy.viewanalysis import AggregationType

def view_search(db_name, node_list,cost=""):
    """ Convienience method to store View_Dist
    """
    from hfea.search import ShortestPath as sp

    if cost=="": cost = 'View_Dist'
    # Add nodes to the Queue
    act.queueManager(db_name, node_list, cost, False, 
                        ConnectionType='OneToAll')

    # Call the shortest path algorithm 
    sp.main(dbName=db_name,cost_type=cost,force_gen=True)
    

def run():
    # Define params
    cost = 'Dist'
    db_name = 'c_test'


    geom_bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj')

    # Get node dicts from the DB
    node_map, id_map = act.get_node_dicts(db_name)
    graph = act.get_graph(db_name)
    nodes = list(node_map.keys())

    height = 10 # Set a height offset to cast rays from the points
    ray_count = 10 # Set the number of rays to use per node

    nodes = nodes[0:2]
    # Spherical view analysis returns distance
    scores = SphericalViewAnalysisAggregate(geom_bvh, nodes, ray_count, height,
                                                upward_fov = 5, downward_fov=5, agg_type=AggregationType.MIN)
    
    print('Len Sum: ', len(scores))
    print('Score Sum: ', sum(scores))


    # Spherical view analysis returns distance
    scores2 = SphericalViewAnalysis(geom_bvh, nodes, ray_count, height,
                                                upward_fov = 5, downward_fov=5)

    # Get minimum distance 
    score_dist = scores.array['distance']
    bit_and = np.bitwise_and( np.isfinite(score_dist) , score_dist>0 )
    scores = np.amin(score_dist, axis=1, where=bit_and, initial=9999)
    
    print('Len Sum: ', len(scores))
    print('Score Sum: ', sum(scores))

    # Store the costs in the DB
    gd.setAttrToOutCost(graph, node_map, nodes, scores, dbName=db_name,
                        storeNodes=True, storeEdges=True, cost_name='View_MIN')

    # ##### Manual Python #####
    # nodes_height = [ (x[0],x[1],x[2]+height) for x in nodes]
    # # Get Directions
    # dirs = SphericallyDistributeRays(1000, upward_fov=5,downward_fov=5)
    # # Get minimum score of directions 
    # scores = np.empty((len(dirs),len(nodes)))
    # for idx, dir in enumerate(dirs):
    #     scores[idx] = ru.cast_ray_multi(geom_bvh, nodes_height, dir)['distance']

    # # Get bit array 
    # bit_and = np.bitwise_and( np.isfinite(scores) , scores>0 )
    # # Get minimum, defaults to 9999
    # min_scores = np.amin(scores, axis=0, where=bit_and, initial=99999)
    # print('Manual Score Sum: ', sum(min_scores))

    # # Store the costs in the DB
    # gd.genViewCost(graph, node_map,nodes,min_scores,dbName=db_name,
    #                 storeNodes=True,storeEdges=True, cost_name='View_MIN')

    # ###########

    # Set the threshold 
    scores = gd.set_threshold(scores)

    # Store the costs in the DB
    gd.setAttrToOutCost(graph, node_map,nodes,scores,dbName=db_name,
                        storeNodes=True,storeEdges=True, cost_name='View_MIN_Thresh')

    # # Inverse scores and scale to max so there are no negatives
    # scores_inv = (-1*np.asarray(min_scores,dtype=np.float64)+(max(min_scores)))
    # #Store inverted scores so shorter distance is higher cost
    # gd.genViewCost(graph, node_map,nodes,scores_inv,dbName=db_name,
    #                 storeNodes=True,storeEdges=True, cost_name='View_MIN_inv')


    # Store a combination of scores
    costs = ['View_MIN_Thresh', 'Dist']
    weights = [1, 1]
    param_name = 'View_MIN_Dist'
    gd.set_weighted_cost(costs, weights, param_name, dbName=db_name)

    # Get some nodes and calculate view-based shortest path
    n1 = (-391, 134, 626.7)
    n2 = (2351, 837, 634.5)

    n1 = act.get_closest_node(n1,node_map.keys())
    n2 = act.get_closest_node(n2,node_map.keys())

    n1_id = node_map[n1]
    n2_id = node_map[n2]

    node_list = [n1_id, n2_id]
    view_search(db_name, node_list, cost=param_name)


if __name__ == '__main__':
    multiprocessing.freeze_support()
    
    run()
