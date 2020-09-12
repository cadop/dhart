import time
import math
import multiprocessing  

import hfea.search.ShortestPath as sp
import hfea.utils.interactions as act
  
def runSearch():

    # Define params
    cost = 'Dist'
    db_name = 'c_test'

    # Get node dicts from the DB
    node_map, id_map = act.get_node_dicts(db_name)

    # Find the closest nodes to the desired points
    # 620 -> 1670
    # -401, 134, 625.5
    # 2359, 534, 625.5

    n1 = (-391, 134, 626.7)
    n2 = (2351, 837, 634.5)

    n1 = act.get_closest_node(n1,node_map.keys())
    n2 = act.get_closest_node(n2,node_map.keys())

    n1_id = node_map[n1]
    n2_id = node_map[n2]
    print(n1_id)
    print(n2_id)

    act.clear_paths(db_name)
    act.clear_queue(db_name)

    # Add nodes to the Queue
    act.queueManager(db_name, [n1_id, n2_id], 'Dist',False, 
                        ConnectionType='OneToAll')

    # Call the shortest path algorithm 
    sp.main(dbName=db_name,cost_type=cost,force_gen=True)
    
    return 


if __name__ == '__main__':
    multiprocessing.freeze_support()

    runSearch()
