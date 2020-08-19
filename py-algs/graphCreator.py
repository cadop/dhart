import geomUtils as gt
import rayUtils as ru
import mathUtils as mu

import math
import copy
import time
import sys
from collections import defaultdict

import pickle

        
def walkable_check(pos,geom,obstacles): #Should take in the parent node to compare angle for walkability
    #result is checked by the function get_intersect
    floor_hit = check_floor(pos,geom,type = 3)
    if floor_hit == None:
        return None
    #possible objects is a new array
    hit = ru.cast_ray(geom, pos, (0,0,-1), pos=True )
    # hit = ru.check_ray(pos,(0,0,-1),obstacles,type = 3)
    if hit != None:
        if hit[2] >= floor_hit[2]: #This should be an angle rather than just distance above floor
            return False
    return floor_hit
        

def check_floor(parent,child,geom,graphParams):

    res = ru.cast_ray(geom, child, (0,0,-1) )
    
    #Ray doesnt hit anything (no floor)
    if res == -1:
        return None    
        
    #TODO: Change this because it really should check if it is a step or slope first    
    else:
        # This is where the child is actually defined as it is the intersection
        #  point with the floor
        nodePos = child[0],child[1],round(child[2]-res,1)

        if (parent[2]-nodePos[2]) > graphParams.get('downstep'):#downstep_limit:
            return None
        #if the child is higher than the parent
        elif (nodePos[2]-parent[2]) > graphParams.get('upstep'): #[3] is the step height
            return None
        
        return nodePos           
        
def check_start(child,geom):

    res = ru.cast_ray(geom, child, (0,0,-1) )

    #Ray doesnt hit anything (no floor)
    if res == -1:
        return None    
        
    else:
        nodePos = child[0],child[1],round(child[2]-res,1) #should match check_floor tolerance
        return nodePos      

def checkConnection(parent,child,testPnt,geom,obstacles,graphParams):    
    """
    check if the parent and child have a connection between them
    parent: xyz of parent node
    child: xyz of child node to test
    testPnt: xyz of the test point (includes the z offset for shooting ray down
    geom: geometry that is considered the floor or possibly walkable
    obstacles: geometry that is considered an obstacle no matter if it qualifies as walkable
    
    Return: 
    0 is not connected
    1 is connected with no step
    2 is connected with a step up
    3 is connected with a step down
    4 is connected with a step over
    """
    
    def isConnected(node1,node2):
        """
        helper function to check if there is a connection between the two nodes
        """
        realdst = math.sqrt(  (node1[0]-node2[0])**2 + 
                              (node1[1]-node2[1])**2 +
                              (node1[2]-node2[2])**2 )
        # shoot a ray from parent to child
        # check if the ray distance is shorter than the node distance
        direc = mu.unitVec(mu.makeVec(node1,node2))
        res = ru.cast_ray(mesh, node1, direc )

        # print(res)
        if res != -1:
            if res < realdst:
                return False
        
        return True

    def slopeCalc(node1,node2):
        
        #slope is rise/run. slope to angle is (180/pi)*(tan-1(slope))
        nodeRun = math.sqrt( (node1[0]-node2[0])**2 +  (node1[1]-node2[1])**2 )
        nodeRise = node2[2] -  node1[2]
        #upslope limit is offsets[7], downslope limit is offsets[8]
        #slope = math.degrees( math.atan2(nodeRise,nodeRun) )
        slope = math.degrees( math.atan2(nodeRise,nodeRun) )
        #parent is lower than child, it is upslope
        if slope > -1*graphParams.get('downslope') and slope < graphParams.get('upslope'):
            return True

        else: 
            return False
        
    #Mesh version
    if not isinstance(geom, list): geom=[geom]
    for mesh in geom:

        node1 = list(parent)
        node2 = list(child)
        node1[2] += 0.01
        node2[2] += 0.01

        #check if there is a direct connection
        if  isConnected(node1,node2): 
            ### TODO should add a slope return type
            # if so, record it as a non-step 

            #check if the nodes are at the same height (for slope) within some tolerance
            if abs( node1[2]-node2[2] ) < 0.1:
                return 1

            #if slope is within limits return valid
            if slopeCalc(node1,node2):
                return 1
            
            #slope is not within limits, not a valid location
            return 0

        # if not, then check if there is a step-based connection
        #check if parent or child is above or below
        if parent[2] > child[2]:
            #the child is lower, so we raise the child point (going down stairs)
            node1 = list(child)
            node1[2] += graphParams.get('downstep')
            node2 = list(parent)
            node2[2]+= 0.01 #add small amount to not be on ground mesh
            step = 3
            
        elif parent[2] < child[2]:
            # child is higher, (going up stairs)
            node1 = list(parent)
            node1[2] += graphParams.get('upstep')
            node2 = list(child)
            node2[2] += 0.01 #add small amount to not be on ground mesh
            step = 2
            
        elif parent[2] == child[2]:
            #same level but there is something inbetween
            node1 = list(parent)
            node1[2] += graphParams.get('upstep')
            node2 = list(child)
            node2[2] += 0.01 #add small amount to not be on ground mesh
            step = 4

        ## if true, return it is a step
        if isConnected(node1,node2):     
            return step #step is stored as up or down in the above check     
        
    return 0
    
def getChildren(parent,possible_children,graph,geom,obstacles,graphParams):
    # Go through each child
    valid_children = []
     
    for child in possible_children:
        # Get the ray intersect on the floor
        res = check_floor(parent,child,geom,graphParams)
                
        if res!=None: #If ray hits floor
            #check if the child can be connected to the parent (visibility graph/ edge cull)
            connected = checkConnection(parent,res,child,geom,obstacles,graphParams)
            #test if the width requirement is satisfied
            if graphParams.get('width') > 0:
                spacing = checkWidth(res,child,geom,obstacles,graphParams)
            else: spacing = True
            # connected = True #uncomment for testings
            if (connected != 0) and (spacing == True): 
                if res in graph:  #If node is already in graph
                    pass
                else: #not is not in graph, check if anything is above it
                    if obstacles != None:
                        res = walkable_check(child,geom,obstacles)
        if (res != False) and (res !=None) and (connected != 0) and (spacing==True): 
            valid_children.append([res,connected,spacing])
                    
    return valid_children
            
    
def checkWidth(node,child,geom,obstacles,graphParams):
    step = 30
    for i in range(0,360,step):
        ang = math.radians(i)
        x = math.sin(ang)
        y = math.cos(ang)

        p1 = node[0],node[1],node[2]+(graphParams.get('upstep'))
        p2 = [x,y,0]
        #check if ray hits an obstacle or object
        dist = ru.cast_ray(geom, p1, p2 )

        if dist != -1:
            if dist < graphParams.get('width'):
                return False

    return True

def scoreRelation(parent,child,graphParams):
    dst = math.sqrt(  (parent[0]-child[0])**2 + 
                      (parent[1]-child[1])**2 +
                      (parent[2]-child[2])**2 )
                      
    dst = round( dst , 8 ) #round distance to avoid float errors                  
          
    n1,n2,n3 = child[0]-parent[0],child[1]-parent[1],child[2]-parent[2]
    u1,u2,u3 = (0,0,1)
                      
    numerator = abs( (n1*u1) + (n2*u2) + (n3*u3) )
    denom = abs( math.sqrt( n1**2 + n2**2 + n3**2 ) ) * abs( math.sqrt( u1**2 + u2**2 + u3**2 ) )
    res = math.asin(numerator/denom)
    
    if child[2] > parent[2]: direc = 1
    else: direc = -1
    
    angle = math.degrees(res) * direc
                      
    return dst,angle

    
def buildGraph(To_Do,graphParams, geometry_bvh,graph): 
    #Takes in a list with a single start location: To_Do

    #generate directions
    directions = mu.genDirections(graphParams.get('max_connection'))
    #get offsets
    x_offset = graphParams.get('x_offset')
    y_offset = graphParams.get('y_offset')
    height = graphParams.get('height')

    def computeParent(parent):
    
        def direcs(parent):        
            possible_children = []
            
            for direc in directions:
                i,j = direc
                #Offset the child from the parent                         
                child = [ round( (parent[0] + (i* x_offset ) ) ,8),
                          round( (parent[1] + (j* y_offset ) ) ,8),
                          round(  parent[2] +     height       ,8) ]

                possible_children.append(child) # Make a list of the possible children
            return possible_children
            
        possible_children = list(map(direcs,[parent]))[0]

        children = getChildren(parent,possible_children,graph,geometry_bvh,None,graphParams)

        relations = []
        rel_count = 0

        for item in children: 
            child = item[0]
            if(child is None): 
                continue
                
            stepType = item[1]
            spacing = item[2]
            dst,angle = scoreRelation(parent,child,graphParams)
            weight = [dst,angle,stepType,spacing] #make a list of weights
            relations.append([parent,child,weight])
            rel_count+=1
        
        if rel_count == 0:
            weight = [0,0,0,0]
            relations.append( [parent,parent,[999999999999.9999 for x in weight] ] )
        return relations
        
    while len(To_Do) != 0:

        if len(graph.keys()) > graphParams.get('max_nodes'):
            print('Max Nodes Reached')
            return graph

        #should also check how many objects there are, especially obstacles,
        # which has a big impact on speed

        relations = []
        while len(To_Do) != 0:
            parent = To_Do.pop(0) 
            relations += computeParent(parent)
        
        To_Do_Set = set()
        
        for vals in relations:

            parent = (vals[0][0],vals[0][1],vals[0][2])
            child = (vals[1][0],vals[1][1],vals[1][2])

            # child_arr = vals[1] # store an array version instead of tuple 
            weight = vals[2]
            
            #uses a defaultdict to add children to the parent
            graph[parent] += [[child,weight]]

            #store the children in the set
            To_Do_Set.add(child)   


        #check if the children are in the graph or not
        To_Do = []
        for child in To_Do_Set:
            if  (child not in graph) and  (child not in To_Do) : To_Do.append(child)
    

    return graph
  
def buildNetwork(start_location, geometry_bvh, graphParams): 

    start_location= (start_location[0],start_location[1], start_location[2]+graphParams.get('height') )
    #Round off the number so Rhino doesn't have too many issues?
    start_location = (round(start_location[0],8),
                      round(start_location[1],8),
                      round(start_location[2],8))

    start_location = check_start(start_location,geometry_bvh)
    print('starting node location: ',start_location)

    graph = defaultdict(list)

    return buildGraph([start_location],graphParams, geometry_bvh, graph=graph)
                        
