import cProfile
import itertools
import multiprocessing
import os
import pstats
import random
import sys
from sys import path as syspath
from time import sleep

from . import PathTools as pathutils
from ..database import DB as DB

def genPaths(node_map,node_lookup,predecessor,node_pairs):

    # Generate paths, and store start/end
    print("Generating Paths...")

    paths = []
    for pair in node_pairs:
        path = pathutils.get_path(predecessor, pair[0], pair[1])
        if len(path) == 1:
            print('Path was not found for nodes: ',pair[0],' ',pair[1])
        else: 
            paths.append(path)

    # Convert indexes to XYZ
    final_paths = [[node_map[node] for node in path] for path in paths]

    # Put headers into strings
    headers = [(str(ij[0]), str(ij[-1])) for ij in paths]

    return final_paths, headers
    

def main(dbName = 'EXdb', cost_type=None, force_gen=False):
    
    # dir_path = os.path.dirname(os.path.realpath(__file__))
    # os.chdir(dir_path)
    # sys.path.insert(0, os.path.abspath(".."))

    print("Initiating QUEUE SHORTPATH")

    # Get needed info from DB
    DB.initDB(dbName)
    graph = DB.getDictionary(cost_type)
    node_map = DB.getHashMap(True) #node_map ~ {(x,y,z):id}
    node_lookup = {y:x for x,y in node_map.items()} #node_lookup ~ {id:(x,y,z)}
    
    print("Getting Predecessor matrix...")
    # print("inputs to pred: ",graph.keys(), dbName, cost_type)
    graphAr, shortPath, predecessor  = pathutils.getDistAndPred(graph, node_map, node_lookup, 
                                                                dbName, cost_type, forceGen=force_gen)
    print("cost_type: ",cost_type)
    node_pairs = DB.getQueue(cost_type)
    print("Pairs: ",node_pairs)
    final_paths,headers = genPaths(node_map,node_lookup,predecessor,node_pairs)

    # Add to DB
    print('Adding Paths to DB')
    DB.addShortPaths(final_paths, headers,cost_type)
    print("finished Adding Paths")
    return


if __name__ == '__main__':
    multiprocessing.freeze_support()

    dir_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(dir_path)
    os.chdir('..')
    # Setup input vars
    n = len(sys.argv)

    #Add the module to the system path
    sys.path.insert(0, os.path.abspath(".."))
    import graphUtils.database.DB as DB

    dbName = sys.argv[1] if n >= 2 else "c_test"
    cost_type = sys.argv[2] if n >= 3 else "Dist"
    
    print("DB Name: ",dbName)
    print("Cost Type: ",cost_type)
    # main(dbName = dbName, cost_type=cost_type)

    try:
        main(dbName = dbName, cost_type=cost_type)
        sleep(1)
        sys.exit(1)
    except Exception as E:
        print("exception")
        print(E)
        sleep(40)
        