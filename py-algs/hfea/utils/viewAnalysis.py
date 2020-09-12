import math
import time

import rayUtils as ru

def viewRayCheck(p1,p2,geometry,type=3):
    #    Takes in two arguments
    #    each argument is an array of 3 points
    # start_point = Rhino.Geometry.Point3d(float(p1[0]), float(p1[1]), float(p1[2]))
    start_point = p1
    # direction = Rhino.Geometry.Vector3d(p2[0], p2[1], p2[2])
    direction = p2
    ray = Rhino.Geometry.Ray3d(start_point, direction)
    bounce = 1
    
    #Type 1 is using breps and not checking for trimmed surfaces
    if type == 1:
        #intpt is the intersection of the geometry and the ray
        intPt = Rhino.Geometry.Intersect.Intersection.RayShoot(ray,geometry, bounce)
        if intPt == None: return None
        else: return intPt[0]
    
    #type 2 checks if the point is really on a brep surface in the case of trims
    elif type == 2:
        intPt = Rhino.Geometry.Intersect.Intersection.RayShoot(ray,geometry, bounce)

        if intPt == None:
            return None
        else:
            #check if it was inside of a brep
            real_hits = []
            for obj in geometry:
                intPt = Rhino.Geometry.Intersect.Intersection.RayShoot(ray,[obj], bounce)
                if intPt == None: continue
                closePnt = obj.ClosestPoint(intPt[0])
                dst = intPt[0].DistanceTo(closePnt)
                thresh = 0.001
                if dst < thresh:
                    real_hits.append(intPt[0]) #use for rayshoot
                    
            #find the closest hit 
            minhit = None
            mindst = 99999999999
            for hit in real_hits:
                dst = hit.DistanceTo(start_point)
                if dst<mindst:
                    mindst = dst
                    minhit = hit
                    
            if minhit == None:
                return None
                
            return minhit
            
    #type 3 is mesh intersections
    elif type == 3:
        for mesh in geometry:
            intPt = Rhino.Geometry.Intersect.Intersection.MeshRay(mesh,ray)
            #if the ray misses the geometry, continue checking
            if intPt == -1: pass
            else: return intPt
            
        #if no hit, return False
        return False
        
def viewVectors(pnt=None,step=8,size=1000,unitVec = True):
    """
    calculate the view vectors using spherical coordinates
    """
    if unitVec == True: size = 1
    if unitVec == True: shift = [0,0,0]
    else: shift = pnt
    
    viewVecs = []
    
    #i is looking forward starting from straight up: up,down
    #up = 0 means it is directly vertical to the sky
    # down = 90 means it is straight forward
    up_angle = 50
    down_angle = 70
    left_angle = 180
    right_angle = 180
    for i in range(90-up_angle,90+down_angle,step):
        #j is looking side to side: left,right
        for j in range(-1*right_angle,left_angle,step):
        
            theta = math.radians(i)
            phi = math.radians(j)
            x = size*( math.sin(theta) * math.cos(phi) )  + shift[0]
            y = size*( math.sin(theta) * math.sin(phi) )  + shift[1]
            z = size * math.cos(theta) + shift[2]
            
            viewVecs.append( Rhino.Geometry.Vector3d(x,y,z) )

    return viewVecs
            
def getVisibility(nodePnt,geom,resolution,storeLine=False,vizTarget=[]):
    #get the view vectors from the node location
    vecs = viewVectors(pnt=nodePnt,step=resolution)
    
    node = Rhino.Geometry.Point3d(float(nodePnt[0]), float(nodePnt[1]), float(nodePnt[2]))

    lineArrays = []
    visArray = []
    lineArray = []    
    for vec in vecs:
        res  = viewRayCheck(node,vec,geom,type=3)
        visArray.append(res)
        if storeLine == True:
            if len(vizTarget)==0: continue
            if nodePnt not in vizTarget: continue
            if res == False:
                x = 3000*vec[0] + node[0]
                y = 3000*vec[1] + node[1]
                z = 3000*vec[2] + node[2]
                # lineArray.append(rs.AddLine(node,(x,y,z)))
                lineArray.append( rg.Line(node,rg.Point3d(x,y,z)) )
    
    #lineArrays.append(lineArray)
    #calculate vis percent instead of returning as array
    visArray,distSum = scoreVisibility(visArray)        
    
    return [distSum,lineArray]
    # return [visArray,lineArray]
    
def scoreVisibility(score):
    trues = [x for x in score if x != False]
    falses =  [x for x in score if x == False]
    tmp_score = ( (len(score)-len(trues) ) / float(len(score)) )*100
    
    distSum = sum(trues)

    return tmp_score,distSum
            
            
def getClosest(nodes,obj_loc):
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
    
def scoreView(nodes,obstructions,resolution = 10,height= 0.001,storeLine = False,targets=[],numCores=10):
    start_time = time.time()
    geom = getOpaque(opaqueObjects=obstructions)
    print('Mesh time: ',time.time()-start_time)

    start_time = time.time()

    nodeList = nodes.keys()

    nodeListPar = [ (x[0],x[1],x[2]+height) for x in nodeList ]#[0:1000]
    
    #add the height     
    targets = [(x[0],x[1],x[2]+height) for x in targets]
    
    #get the closest nodes to the target view points
    targetNodes = []
    for target in targets:
        targetNodes.append(getClosest(nodeListPar,target))
    
    visArray = nodeListPar.AsParallel()\
                    .WithDegreeOfParallelism(numCores)\
                    .WithExecutionMode(ParallelExecutionMode.ForceParallelism)\
                    .Select(
                    lambda node: 
                        getVisibility(node,geom,resolution,storeLine = storeLine,vizTarget=targetNodes) ).ToList()

    score_list = [ x[0] for x in visArray ]

    lineArrays = [ x for x in visArray if x[1] != []]
    nodesCalculated = [ rs.coerce3dpoint(node) for node in nodeListPar ]
    
    print('Parallel View Analysis time: ',time.time()-start_time)
    
    return lineArrays,geom,score_list,nodesCalculated
    
def scoreViewSingleCore(nodes,obstructions,resolution = 10,storeLine = False,height= 0.001,targets=[]):
    start_time = time.time()
    geom = getOpaque(opaqueObjects=obstructions)
    print('Mesh time: ',time.time()-start_time)

    start_time = time.time()

    nodeList = nodes.keys()
    #get the closest nodes to the target view points
    targetNodes = []
    for target in targets:
        targetNodes.append(getClosest(nodeList,target))
    
    score_list = []
    nodesCalculated = []
    lineArray = []
    for i in range(len(nodeList)):
        node = nodeList[i]
        
        node = (node[0],node[1],node[2]+height)
        nodesCalculated.append(rs.coerce3dpoint(node))
        
        res = getVisibility(node,geom,resolution,storeLine = storeLine,vizTarget=targetNodes)
        visArray,lines = res[0],res[1]
        
        score_list.append(visArray)
        lineArray.append(lines)
        
        # #temp make score the array
        # score = visArray
        # trues = [x for x in score if x == True]
        # falses =  [x for x in score if x == False]
        # tmp_score = ( (len(score)-len(trues) ) / float(len(score)) )
        # score_list.append(tmp_score)
        
    #print(score_list)
    
    
    print('View Analysis time: ',time.time()-start_time)
    
    return lineArray,geom,score_list,nodesCalculated