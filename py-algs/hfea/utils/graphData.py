import math
import time
from collections import defaultdict
import numpy as np

from ..database import DB as DB

def setGraphEdgeData(dbName,graphArr=None,graph_dict=None,idx=None,edgeName='Dist'):
    '''
    Use for setting edge costs in the DB from the graph
    '''

    # DB.initDB(dbName)
    idMap = DB.getHashMap()

    if graph_dict != None and idx != None:
        new_dict = extractGraphEdge(graph_dict,idx)
        graphArr = graph_to_array(new_dict)

    newEdges = [ ( idMap[x[0]], idMap[x[1]], x[2] ) for x in graphArr  ]
    DB.setEdgeCosts(newEdges, edgeName)

    return

def storeDefaultGraphData(dbName,graph,dir_path=""):
    '''
    Use for setting edge costs in the DB from the graph
    Only used for storing edge costs that are generated during the graph creation

    0 is distance
    1 is slope angle -- weird
    2 is step type
    3 is spacing
    4 is score

    '''

    #initialize database
    DB.initDB(dbName,dir_path=dir_path,ret_path=True)
    #set the distance edge cost as the default for the graph in the DB
    dist_graphDict = extractGraphEdge(graph,0)
    DB.inputGraph(dist_graphDict)
    #get the idMap of the graph
    idMap = DB.getHashMap()

    #set the distance cost as 'Dist'
    graphArr_dist = graph_to_array(dist_graphDict)
    newEdges_dist = [ ( idMap[x[0]], idMap[x[1]], x[2] ) for x in graphArr_dist  ]
    DB.setEdgeCosts(newEdges_dist, 'Dist')

    #Check it was inserted correctly
    DB.getEdgeDictionary(name='Dist')

    setGraphEdgeData(dbName,graph_dict=graph,idx=2,edgeName='Step')

    #set the Step type
    new_dict = extractGraphEdge(graph,2)
    graphArr = graph_to_array(new_dict)
    newEdges = [ ( idMap[x[0]], idMap[x[1]], x[2] ) for x in graphArr  ]
    DB.setEdgeCosts(newEdges, 'Step')

    #set the Slope cost
    new_dict = extractGraphEdge(graph,1)
    graphArr = graph_to_array(new_dict)
    newEdges = [ ( idMap[x[0]], idMap[x[1]], x[2] ) for x in graphArr  ]
    DB.setEdgeCosts(newEdges, 'Slope')
    
    #set the Cross Slope cost
    new_dict = extractCrossSlope(graph)
    graphArr = graph_to_array(new_dict)
    newEdges = [ ( idMap[x[0]], idMap[x[1]], x[2] ) for x in graphArr  ]
    DB.setEdgeCosts(newEdges, 'Cross')
    
    #set the Energy Cost
    new_dict = extractEnergy(graph)
    graphArr = graph_to_array(new_dict)
    newEdges = [ ( idMap[x[0]], idMap[x[1]], x[2] ) for x in graphArr  ]
    DB.setEdgeCosts(newEdges, 'Energy')
    
    return

def graph_to_array(graph):
    graph_array = []
    for parent in graph:
        for child in graph[parent]:
            edge1 = parent
            edge2 = child[0]
            weight = child[1]
            graph_array.append( [edge1,edge2,weight] )
    return graph_array
    
def getClosestNode(nodes,obj_loc):
    closest_node = None
    closest_dist = 99999999999
    for node in nodes:
        x = obj_loc[0] - node[0]
        y = obj_loc[1] - node[1]
        z = obj_loc[2] - node[2]
        dist = math.sqrt(x*x + y*y + z*z)
        if closest_dist >= dist:
            closest_dist = dist
            closest_node = node  
                    
    return closest_node   

def extractGraphEdge(graph,idx):
    """
    Extract the graph edge 
    
    """
    graphDict = {}

    for parent in graph:
        for connection in graph[parent]:
            
            child = connection[0]
            weight = connection[1][idx]
            # print(weight)
            # if weight > 1: print(weight)
            #If this parent is not in the graph, add it
            if parent not in graphDict:
                graphDict.setdefault(parent,[[child,weight]])

            #If this parent is in the graph, update its children
            else: graphDict[parent] = graphDict[parent] + [[child,weight]]
                
    return graphDict

def extractEnergy(graph):
    """
    Calculate the energy cost of the edges in the graph  
    """
    graphDict = {}
    for parent in graph:
        for connection in graph[parent]:
            child = connection[0]
            dst = connection[1][0]
            slope = connection[1][1]
            #cSlope = 
            g = math.tan(math.radians(slope))
            #if slope is out of the range of the function set it to the limits
            if g <-0.4: g = -0.4
            if g>0.4: g = 0.4
            e = 280.5*(g**5) - 58.7*(g**4) - 76.8*(g**3) + 51.9*(g**2) + 19.6*(g) + 2.5
            
            if e< 0: print("E IS: ",slope, '   ',g,'   ',e)
            weight = e * dst
            
            # print(weight)
            # if weight > 1: print(weight)
            #If this parent is not in the graph, add it
            if parent not in graphDict:
                graphDict.setdefault(parent,[[child,weight]])
            #If this parent is in the graph, update its children
            else: graphDict[parent] = graphDict[parent] + [[child,weight]]
                
    return graphDict

    
def extractCrossSlope(graph):
    """
    Calculate the cross slope values of the edges in the graph  
    
    """

    graphDict = {}
    for parent in graph:
        for connection in graph[parent]:
            child = connection[0]
            dst = connection[1][0]
            slope = connection[1][1]
            
            #get vector of child
            cVec = [child[0]-parent[0],child[1]-parent[1]]
            
            perpChild = []
            #go through the other children and see if there is a perpendicular edge
            for others in graph[parent]:
                if child == others: continue
                child2 = others[0]
                dst2 = others[1][0]
                #get vector of child
                cVec2 = [child2[0]-parent[0],child2[1]-parent[1]]
                
                dotProd = ( cVec[0] * cVec2[0] ) + ( cVec[1] * cVec2[1] ) 
                if abs(dotProd) < 0.001:
                    #they are perpendicular, so store the child
                    perpChild.append([child2,dst2])
            
            if len(perpChild) == 0:
                weight = dst
            else:
                #there is a perpendicular axis
                if len(perpChild) == 1:
                    weight = abs(child[2] - perpChild[0][0][2]) + perpChild[0][1]
                elif len(perpChild) == 2:
                    weight = abs(perpChild[0][0][2] - perpChild[1][0][2]) + perpChild[0][1]
                else:
                    print('Cross Slope len of Children: ',len(perpChild))
            # print(parent,child,cVec)
            # print(perpChild)
            
            #If this parent is not in the graph, add it
            if parent not in graphDict:
                graphDict.setdefault(parent,[[child,weight]])
            #If this parent is in the graph, update its children
            else: graphDict[parent] = graphDict[parent] + [[child,weight]]
                
    return graphDict

def set_weighted_cost(costs, weights, param_name, dbName = ''):
    """ Generate a weighted cost by combining multiple edge costs and weighting values

    costs : array of strings to load as edge costs
    weights : array corresponding to costs to use as weighting factors
    param_name : string of edge cost name to store in database

    """

    db_name = dbName # for now a legacy naming conversion

    DB.initDB(db_name)

    s = time.time()

    edge_dict = defaultdict(lambda:1)

    for idx,cost in enumerate(costs):
        edge_costs = DB.getEdgeDictionary(cost)
        weight_factor = weights[idx]
        if weight_factor < 0:
            weighted = lambda x: x**-1
        else:
            weighted = lambda x: x*weight_factor

        for e in edge_costs.items():
            n1, n2 = e[0][0], e[0][1]
            c = e[1]

            edge_dict[(n1,n2)] *= weighted(c)

    edge_data = [ (k[0],k[1],v) for k,v in edge_dict.items() ]

    print("Construction Time: ",time.time()-s)

    DB.setEdgeCosts(edge_data, param_name)
    print("Total Time: ",time.time()-s)

    return 


def genViewCost(graph, idMap, nodes, scores, dbName = '', 
                storeNodes=True, storeEdges=False, cost_name='View'):
    """ Stores the edge cost and node attributes by taking the array of spatial view data.

    Stores a view node attribute and View edge cost equal to average distance and
    view_inv / View_inv for an inverse cost.
    """

    viewEdges = []
    viewEdges_append = viewEdges.append
    viewEdges_inv = []

    viewNodes = []
    viewNodes_append = viewNodes.append
    viewNodes_inv = []

    node_score = dict(zip(nodes,scores))

    #iterate through the graph
    for n1 in graph:
        #iterate through the children of this parent
        n1_id = idMap[n1]
        
        #viewNodes.append( (idMap[n1], n1_cost, 'view') ) 

        viewSum = 0
        cnt = 0

        for n2 in graph[n1]:
            n2 = n2[0]
            n2_id = idMap[n2]

            #find the score of the given key
            # loc = nodes.index(n2)
            # cost = scores[loc]

            cost = node_score[n2]

            viewSum += cost
            cnt+=1

            #make a list of (parent,child,weight)
            viewEdges_append((n1_id,n2_id,float(cost)))

            # viewEdges_inv.append((idMap[n1],idMap[n2],1.0/cost))

        #no children or connections so ignore  
        if viewSum == 0: 
            cnt = 1
            continue 
        #add node attribute based on outgoing costs
        viewNodes_append( (n1_id,cost_name, float(node_score[n1])) ) 
        # viewNodes_inv.append( (idMap[n1],'view_inv', 1.0/(viewSum/cnt)) ) 
        # viewNodes.append( (idMap[n1],'viewCnt', cnt) ) 

    s = time.time()

    if storeEdges:   
        DB.setEdgeCosts(viewEdges, cost_name)
        # DB.setEdgeCosts(viewEdges_inv, 'View_inv')

    if storeNodes: 
        DB.setNodeParameters(viewNodes)
        # DB.setNodeParameters(viewNodes_inv)

    print("DB Storage: ",time.time()-s)

    return viewEdges,viewNodes

def setAttrToOutCost(graph, idMap, nodes, scores, dbName = '', 
                storeNodes=True, storeEdges=False, cost_name='View'):
    """ Store the node attribute as its outgoing edge costs
    """

    viewEdges = []
    viewEdges_append = viewEdges.append

    viewNodes = []
    viewNodes_append = viewNodes.append

    node_score = dict(zip(nodes,scores))

    #iterate through the graph
    for n1 in graph:
        #iterate through the children of this parent
        n1_id = idMap[n1]

        # set the cost as this nodes attribute
        cost = float(node_score[n1])

        # Find the children of this node
        for n2 in graph[n1]:

            n2 = n2[0]
            n2_id = idMap[n2]

            # Check if the node is to itself (which means it didn't have children)
            # if n1_id == n2_id:
            #     viewEdges_append((n1_id,n2_id,99999999999.99))
            #     continue

            # make a list of (parent,child,weight)
            viewEdges_append((n1_id, n2_id, cost))

        # Store the node attribute itself
        viewNodes_append((n1_id, cost_name, cost)) 

    s = time.time()

    if storeEdges:   
        DB.setEdgeCosts(viewEdges, cost_name)

    if storeNodes: 
        DB.setNodeParameters(viewNodes)

    print("DB Storage: ",time.time()-s)

    return viewEdges,viewNodes


def set_threshold(scores,dist=25):
    """ Set a threshold to the array
    """

    # Set a cutoff threshold to greatly increase value of distances lower than 
    # some threshold
    dist_threshold = dist
    #Set high cost to anything under threshold
    scores_thresh = scores
    # Anything under threshold becomes high cost
    scores_thresh[np.where(scores_thresh < dist_threshold)] = 99999
    # Anything under threshold becomes 1, which is multiplied by distance later
    #  so this sets it to distance
    scores_thresh[np.where(scores_thresh < 99999)] = 1
    # Reset all the high values to a smaller multiplier
    scores_thresh[np.where(scores_thresh > 1)] = 3

    return scores_thresh

def cal_point_weights(graph_dict):
    ptWt_dict = {}
    lowVal = 9999999999999
    highVal = -9999999999999
    wtList = []
    for item in graph_dict:
        ptWt_dict[item] = 0
        list_pt = graph_dict[item]
        #if the length is 0, this is a bad node
        # it could happen when max_nodes is reached, so dont want to 
        # hurt the scale of visualization
        if len(list_pt) == 0: 
            ptWt_dict[item] = -1
            wtList.append(ptWt_dict[item])
            continue
        
        nodeWeights = []
        for i in range(len(list_pt)):
            ptWt_dict[item] += list_pt[i][1] # weight of point
            nodeWeights.append(list_pt[i][1])
            
        ptWt_dict[item] = ptWt_dict[item]/len(list_pt)
        
        #Use the largest angle difference as the weight for the node
        #This works for angle analysis, but doesnt make sense for other weighting systems
        
        ptWt_dict[item] = abs(min(nodeWeights)) + abs(max(nodeWeights))
        
        if(lowVal > ptWt_dict[item]) : lowVal = ptWt_dict[item]
        if(highVal < ptWt_dict[item]) : highVal = ptWt_dict[item]
        wtList.append(ptWt_dict[item])
#    print " point weight dict ", ptWt_dict
    return ptWt_dict, wtList, lowVal, highVal

