import itertools
from . import mathUtils as mu
from ..database import DB as DB

try:
    import graphUtils.search.PathDriver as PathDriver
except:
    print("no Path Driver")


def calcPaths(dbName,costTypes,python_path=""):

    DB.initDB(dbName)
    DB.dropPaths(True)

    if costTypes is None:
        PathDriver.generatePaths(dbName)
    else:
        for cost in costTypes:
            if cost == "Distance":
                print("Cost is distance")
                PathDriver.generatePaths(dbName)
            else:
                print("cost is:",cost)
                PathDriver.generatePaths(dbName, cost,python_path)


def getPaths(dbName,costType):
    pathNodes = []
    DB.initDB(dbName)

    paths = DB.getShortPaths(name=costType, ret_costs = True)
    pathCosts = paths[-1]
    paths=paths[0]
    for path in paths:
        if len(path)==0: continue
        pathNodes.append(path)

    return pathNodes,pathCosts

def get_node_dicts(db_name):
    """ Get nodes from the database
    """
    DB.initDB(db_name)

    node_map = DB.getHashMap()
    id_map = DB.getHashMap(reverse=True)

    return node_map, id_map

def get_closest_node(node_target, node_list):
    """ Get closest node in the node list
    """

    closest_node = None

    min_dist = 99999999999999
    for node in node_list:
        dist = mu.dst(node,node_target)
        if dist < min_dist:
            closest_node = node
            min_dist = dist 

    return closest_node 

def get_graph(db_name):
    """ Get graph from database
    """
    graph = DB.getDictionary()
    return graph

def clear_paths(db_name):
    """ Clear the paths in the database
    """

    DB.initDB(db_name)
    DB.dropPaths(True)

def clear_queue(db_name):
    """ Clear the queue of shortest path nodes
    """
    DB.initDB(db_name)

    DB.clearQueue()
    return 'Cleared'

def queueManager(dbName,Points,pathName,ClearQueue,ConnectionType = 'None'):

    DB.initDB(dbName)

    if ClearQueue == True: 
        DB.clearQueue()
        return 'Cleared'

    # List of nodes to queue
    nodelist = Points

    #pntCombos = list(DB.generatePermutations(nodelist))
    if ConnectionType == 'AllToAllDirected':
        print("All to All Directed")
        pntCombos = list(itertools.permutations(nodelist,2))
        
    elif ConnectionType == 'AllToAll':
        print("All to All")
        pntCombos = list(itertools.combinations(nodelist,2))
        
    elif ConnectionType == 'OneToAll':
        print("One to All")
        mainNode = nodelist[0]
        nodelist = nodelist[1:]
        pntCombos = [ [mainNode,x] for x in nodelist ]
        
    elif ConnectionType == 'OrderedPairs':
        print("Ordered Pairs")
        if len(nodelist)%2 != 0:
            print("Must be equal number of items")
        else:    
            pntCombos = []
            idx = 0
            for i in range(0,len(nodelist)-1):
                print(nodelist[idx],nodelist[idx+1],idx)
                pntCombos.append([nodelist[idx],nodelist[idx+1]])
                idx+=2
                if idx>=len(nodelist): break
        
    print(pntCombos)
    print("Path Name: ",pathName)
    queueadd = [(item[0], item[1], pathName) for item in pntCombos]
    print(queueadd)
    DB.addToQueue(queueadd)

    queue = DB.getQueue()
    return queue

if __name__ == '__main__':
    multiprocessing.freeze_support()