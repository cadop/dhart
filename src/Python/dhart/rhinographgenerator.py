import math
import copy
import collections
import time
import sys
from typing import *
from collections import defaultdict
from itertools import permutations as perm
from itertools import combinations as combi
from itertools import product as prod

from dhart.raytracer import EmbreeBVH, embree_raytracer
from dhart.geometry import mesh_info

plot_point = True

downstep_limit = 0
slope_exp = 0

rays = 0
ray_time = 0
csdisp = None

pow = math.pow
sqrt = math.sqrt

max_nodes : int = 5000

def CalculateDistance(p1: Tuple[float, float, float], p2: Tuple[float, float, float]):
    """ Calculate the distance from p1 to p2 """
    return sqrt(pow(p1[0] - p2[0], 2), pow(p1[1] - p2[1], 2), pow(p1[2] - p2[2], 2))


def rayDist(bvh: EmbreeBVH,
            origin: Tuple[float, float, float], 
            direction: Tuple[float, float, float]
            ) -> Union[float, None]:
    """ Get the distance from the origin to its hit point
    
    Args:
        bvh: The BVH to cast the ray at
        Origin: The origin to cast the ray from
        Direction: The direction to cast the ray in
    
    Returns:
        Union[float, None]: None on miss, distance from origin to the
            hitpoitn on hit
    """
    res = check_ray(origin, direction)
    if res:
        return CalculateDistance(origin, direction)
    return None


# def rayDist(p1, p2, geometry, type=3):
#    # start_point = Point3d(float(p1[0]), float(p1[1]), float(p1[2]))
#    # direction = Vector3d(float(p2[0]), float(p2[1]), float(p2[2]))
#    ray = Ray3d(start_point, direction)
#    bounce = 1

#    # type 3 is mesh intersections
#    if type == 3:
#        hits = []
#        for mesh in geometry:
#            intPt = RhinoMeshRay(mesh, ray)

#            if intPt < 0:
#                continue
#            hits.append(intPt)

#        minhit = None
#        mindst = 99999999999
#        for hit in hits:
#            # find the closest hit
#            dst = hit
#            if dst < mindst:
#                mindst = dst
#                minhit = hit
#        # ray_time += time.time()-stime
#        if minhit == None:
#            return None
#        else:
#            return minhit


def check_ray(
    bvh : EmbreeBVH,
    origin: Tuple[float, float, float],
    direction: Tuple[float, float, float]
) -> Union[Tuple[float, float, float], None]:
    """ Check if a ray fired from origin to direction hits

    Args:
        bvh : The BVH to cast the ray at
        origin: The origin to cast the ray from
        direction: The direction to cast the ray in
    
    Returns:
        Union[Tuple[float, float, float], None]: None on miss or a tuple containing
            the coordinates for the hitpoint on hit
    """
    # print(f"Origin: {str(origin)}, Direction: {str(direction)}")
    return embree_raytracer.IntersectForPoint(bvh, origin, direction)


# """
# def check_ray(p1,p2,geometry,type=3):
#    #    Takes in two arguments
#    #    each argument is an array of 3 points
#    ray = Ray3d(start_point, direction)
#    bounce = 1

#    #Type 1 is using breps and not checking for trimmed surfaces
#    if type == 1:
#        #intpt is the intersection of the geometry and the ray
#        intPt = Rhino.Geometry.Intersect.Intersection.RayShoot(ray,geometry, bounce)
#        if intPt == None: return None
#        else: return intPt[0]

#    #type 2 checks if the point is really on a brep surface in the case of trims
#    elif type == 2:
#        intPt = Rhino.Geometry.Intersect.Intersection.RayShoot(ray,geometry, bounce)

#        if intPt == None:
#            return None
#        else:
#            #check if it was inside of a brep
#            real_hits = []
#            for obj in geometry:
#                intPt = Rhino.Geometry.Intersect.Intersection.RayShoot(ray,[obj], bounce)
#                if intPt == None: continue
#                closePnt = obj.ClosestPoint(intPt[0])
#                dst = intPt[0].DistanceTo(closePnt)
#                thresh = 0.001
#                if dst < thresh:
#                    real_hits.append(intPt[0]) #use for rayshoot

#            #find the closest hit
#            minhit = None
#            mindst = 99999999999
#            for hit in real_hits:
#                dst = hit.DistanceTo(start_point)
#                if dst<mindst:
#                    mindst = dst
#                    minhit = hit

#            if minhit == None:
#                return None

#            return minhit

#    #type 3 is mesh intersections
#    elif type == 3:
#        hits = []
#        for mesh in geometry:
#            intPt = Rhino.Geometry.Intersect.Intersection.MeshRay(mesh,ray)
#            if intPt == -1: continue
#            #should be able to use:
#            # intPt = ray.PointAt(intPt)
#            intPt = start_point[0],start_point[1],round(start_point[2]-intPt,8)
#            hits.append(intPt)

#        minhit = None
#        mindst = 99999999999
#        for hit in hits:
#            #find the closest hit
#            # dst = start_point.DistanceTo( Rhino.Geometry.Point3d(hit[0],hit[1],hit[2]) )
#            dst = start_point.DistanceTo( Point3d(hit[0],hit[1],hit[2]) )
#            if dst<mindst:
#                mindst = dst
#                minhit = hit

#        if minhit == None: return None
#        else: return minhit
#


def walkable_check(pos, child, floor_geom, obstacles):
    # Should take in the parent node to compare angle for walkability
    # result is checked by the function get_intersect
    floor_hit = check_floor(parent, child, floor_geom, type=3)
    if not floor_hit:
        return None

    # possible objects is a new array
    hit = check_ray(pos, (0, 0, -1), obstacles, type=3)
    if hit:
        if (
            hit[2] >= floor_hit[2]
        ):  # This should be an angle rather than just distance above floor
            return False
    return floor_hit


def check_floor(parent, child, geom, offsets):
    res = check_ray(child, (0, 0, -1))

    # Ray doesnt hit anything (no floor)
    if not res:
        return None

    else:
        nodePos = res
        if (parent[2] - nodePos[2]) > offsets[5]:  # downstep_limit:
            return None
        # if the child is higher than the parent
        elif (nodePos[2] - parent[2]) > offsets[3]:  # [3] is the step height
            return None
        return nodePos


def check_start(child, geom):
    return check_ray(child, (0, 0, -1))


def translatePoint(p, v, d):
    """
    Take in a point, vector, and distance to translate
    Return the new translated point 
    """
    p2 = [0, 0, 0]
    vsize = math.sqrt(math.pow((v[0]), 2) + math.pow((v[1]), 2) + math.pow((v[2]), 2))

    vscale = d / vsize
    p2[0] = (v[0] * (d / vsize)) + p[0]
    p2[1] = (v[1] * (d / vsize)) + p[1]
    p2[2] = (v[2] * (d / vsize)) + p[2]

    return p2


def unitVec(v):
    """
    Take in a vector
    Return the new unit vector
    """
    uv = [0, 0, 0]
    vsize = math.sqrt(math.pow((v[0]), 2) + math.pow((v[1]), 2) + math.pow((v[2]), 2))

    uv[0] = v[0] / vsize
    uv[1] = v[1] / vsize
    uv[2] = v[2] / vsize

    return uv


def makeVec(p1, p2):
    """
    From p1 to p2, construct a vector
    """
    return [p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]]


# def makePoint3D(p):
#    """
#    Input a tuple or array of xyz
#    Returns a Rhino point3d object
#    """
#    # return Rhino.Geometry.Point3d(p[0],p[1],p[2])
#    return Point3d(p[0], p[1], p[2])


# def makeRay(p1, p2):
#    """
#    Input a point xyz and direction xyz
#    Returns a Rhino ray object
#    """
#    start = Point3d(float(p1[0]), float(p1[1]), float(p1[2]))
#    direc = Vector3d(float(p2[0]), float(p2[1]), float(p2[2]))
#    # return Rhino.Geometry.Ray3d(start, direc )
#    return Ray3d(start, direc)


def checkConnection(parent, child, testPnt, floor_geom, obstacles, offsets):
    """
    check if the parent and child have a connection between them
    parent: xyz of parent node
    child: xyz of child node to test
    testPnt: xyz of the test point (includes the z offset for shooting ray down
    floor_geom: geometry that is considered the floor or possibly walkable
    obstacles: geometry that is considered an obstacle no matter if it qualifies as walkable
    
    Return: 
    0 is not connected
    1 is connected with no step
    2 is connected with a step up
    3 is connected with a step down
    4 is connected with a step over
    """

    # TODO, if not using fast raytracing, use a plane to generate a plane - mesh intersection
    # parent= makePoint3D(parent)
    # child= makePoint3D(child)

    def isConnected(node1, node2):
        """
        helper function to check if there is a connection between the two nodes
        """
        realdst = math.sqrt(
            math.pow((node1[0] - node2[0]), 2)
            + math.pow((node1[1] - node2[1]), 2)
            + math.pow((node1[2] - node2[2]), 2)
        )
        # shoot a ray from parent to child
        # check if the ray distance is shorter than the node distance
        direc = unitVec(makeVec(node1, node2))
        res = rayDist(node1, direc)
        if res and res < realdst:
            return False
        return True

    def slopeCalc(node1, node2):

        # slope is rise/run. slope to angle is (180/pi)*(tan-1(slope))
        nodeRun = math.sqrt(
            math.pow((node1[0] - node2[0]), 2) + math.pow((node1[1] - node2[1]), 2)
        )
        nodeRise = node2[2] - node1[2]

        # upslope limit is offsets[7], downslope limit is offsets[8]
        # slope = math.degrees( math.atan2(nodeRise,nodeRun) )
        slope = math.degrees(math.atan2(nodeRise, nodeRun))

        # parent is lower than child, it is upslope
        if slope > -1 * offsets[8] and slope < offsets[7]:
            return True

        else:
            return False

    # TODO
    # first check if there is a direct connection,

    node1 = list(parent)
    node2 = list(child)
    node1[2] += 0.01
    node2[2] += 0.01

    # check if there is a direct connection
    if isConnected(node1, node2):
        ### TODO should add a slope return type
        # if so, record it as a non-step

        # check if the nodes are at the same height (for slope) within some tolerance
        if abs(node1[2] - node2[2]) < 0.01:
            return 1

        # if slope is within limits return valid
        if slopeCalc(node1, node2):
            return 1

        # slope is not within limits, not a valid location
        return 0

    # if not, then check if there is a step-based connection
    # check if parent or child is above or below
    if parent[2] > child[2]:
        # the child is lower, so we raise the child point (going down stairs)
        node1 = list(child)
        node1[2] += offsets[5]  # offsets[5] is the max down step
        node2 = list(parent)
        node2[2] += 0.01  # add small amount to not be on ground mesh
        step = 3

    elif parent[2] < child[2]:
        # child is higher, (going up stairs)
        node1 = list(parent)
        node1[2] += offsets[3]
        node2 = list(child)
        node2[2] += 0.01  # add small amount to not be on ground mesh
        step = 2

    elif parent[2] == child[2]:
        # same level but there is something inbetween
        node1 = list(parent)
        node1[2] += offsets[3]
        node2 = list(child)
        node2[2] += 0.01  # add small amount to not be on ground mesh
        step = 4

    ## if true, return it is a step
    if isConnected(node1, node2):
        return step  # step is stored as up or down in the above check

    return 0


def getChildren(parent, possible_children, graph, floor_geom, obstacles, offsets):
    # Go through each child
    valid_children = []

    for child in possible_children:
        # Get the ray intersect on the floor
        res = check_floor(parent, child, floor_geom, offsets)

        if res != None:  # If ray hits floor
            # check if the child can be connected to the parent (visibility graph/ edge cull)
            connected = checkConnection(
                parent, res, child, floor_geom, obstacles, offsets
            )
            # test if the width requirement is satisfied
            if offsets[4] > 0:
                spacing = checkWidth(res, child, floor_geom, obstacles, offsets)
            else:
                spacing = True
            # connected = True #uncomment for testings
            if (connected != 0) and (spacing == True):
                if res in graph:  # If node is already in graph
                    pass
                else:  # not is not in graph, check if anything is above it
                    if obstacles:
                        res = walkable_check(parent, child, floor_geom, obstacles)
        if (res != False) and (res != None) and (connected != 0) and (spacing == True):
            valid_children.append([res, connected, spacing])

    return valid_children


def checkWidth(node, child, floor_geom, obstacles, offsets):
    width = offsets[4]

    step = 30
    for i in range(0, 360, step):
        ang = math.radians(i)
        x = math.sin(ang)
        y = math.cos(ang)

        p1 = node[0], node[1], node[2] + (offsets[3])
        p2 = [x, y, 0]
        # check if ray hits an obstacle or object
        ray = makeRay(p1, p2)
        dist = MeshRay(floor_geom[0], [ray])[0]
        if dist != -1:
            if dist < offsets[4]:
                return False

    return True


def scoreRelation(parent, child, offsets):
    dst = math.sqrt(
        math.pow((parent[0] - child[0]), 2)
        + math.pow((parent[1] - child[1]), 2)
        + math.pow((parent[2] - child[2]), 2)
    )

    dst = round(dst, 8)  # round distance to avoid float errors

    n1, n2, n3 = child[0] - parent[0], child[1] - parent[1], child[2] - parent[2]
    u1, u2, u3 = (0, 0, 1)

    numerator = abs((n1 * u1) + (n2 * u2) + (n3 * u3))
    denom = abs(math.sqrt(n1 ** 2 + n2 ** 2 + n3 ** 2)) * abs(
        math.sqrt(u1 ** 2 + u2 ** 2 + u3 ** 2)
    )
    res = math.asin(numerator / denom)

    if child[2] > parent[2]:
        direc = 1
    else:
        direc = -1

    angle = math.degrees(res) * direc

    # score is a function of the slope and distance
    slope_penalty = 0
    slope_factor = angle / 90  # since 90 is maximum slope
    if angle < 0:
        slope_penalty = (slope_factor) ** 3
    # if angle > 0: slope_penalty = (slope_factor+0.25)**5
    if angle > 0:
        slope_penalty = (slope_factor + 1.25) ** 5
    score = dst + abs(slope_penalty) ** offsets[6]  # slope_exp
    if score < 0:
        print(score)

    return dst, angle, score


def genViewEdgeCost(graph, idMap, nodes, scores):
    """
    
    """

    viewEdges = []
    print("GenningViewEdgeCost")
    # iterate through the graph
    for n1 in graph:
        # print(graph[n1])
        for n2 in graph[n1]:
            # print(n2)
            n2 = n2[0]
            # find the score of the given key
            loc = nodes.index(n2)
            cost = scores[loc]
        viewEdges.append((idMap[n1], idMap[n2], cost))
        # make a list of (parent,child,weight)
    return viewEdges


def calcPathDist(hash, distDict, path):
    """
    hash is the mapping of points to IDs
    path is the list of points along the path
    distDict is the mapping of ID pairs to distance 

    """

    # convert points to ids
    nodeIDs = [hash[x] for x in path]
    # make pairs to create edges
    pairIDs = [(nodeIDs[i], nodeIDs[i + 1]) for i in range(len(nodeIDs) - 1)]
    # get step type for each pairIDs
    distVals = [distDict[x] for x in pairIDs]

    pathDist = sum(distVals)

    return pathDist


def calcSteps(hash, stepDict, path):
    """
    hash is the mapping of points to IDs
    path is the list of points along the path
    stepDict is the mapping of ID pairs to step type
    
    1 is connected with no step
    2 is connected with a step up
    3 is connected with a step down
    4 is connected with a step over
    """

    numSteps = 0
    upSteps = 0
    downSteps = 0
    overSteps = 0

    # convert points to ids
    nodeIDs = [hash[x] for x in path]
    # make pairs to create edges
    pairIDs = [(nodeIDs[i], nodeIDs[i + 1]) for i in range(len(nodeIDs) - 1)]
    # get step type for each pairIDs
    stepTypes = [stepDict[x] for x in pairIDs]

    upSteps = len([x for x in stepTypes if x == 2])
    downSteps = len([x for x in stepTypes if x == 3])
    overSteps = len([x for x in stepTypes if x == 4])

    #     #     #
    numSteps = upSteps + downSteps + overSteps

    return numSteps


def calcView(hash, viewDict, path):
    """
    hash is the mapping of points to IDs
    path is the list of points along the path
    viewDict is the mapping of ID pairs to view scores
    
    """

    viewScore = 0

    # convert points to ids
    nodeIDs = [hash[x] for x in path]
    # make pairs to create edges
    pairIDs = [(nodeIDs[i], nodeIDs[i + 1]) for i in range(len(nodeIDs) - 1)]
    # get step type for each pairIDs
    scoreList = [viewDict[x] for x in pairIDs]
    minView = min(scoreList)
    maxView = max(scoreList)
    viewScore = sum(scoreList)

    return viewScore, scoreList, minView, maxView


def calcEnergy(hash, energyDict, path):
    """
    hash is the mapping of points to IDs
    path is the list of points along the path
    energyDict is the mapping of ID pairs to energy scores
    
    """

    energyScore = 0

    # convert points to ids
    nodeIDs = [hash[x] for x in path]
    # make pairs to create edges
    pairIDs = [(nodeIDs[i], nodeIDs[i + 1]) for i in range(len(nodeIDs) - 1)]
    # get step type for each pairIDs
    scoreList = [energyDict[x] for x in pairIDs]
    minScore = min(scoreList)
    maxScore = max(scoreList)
    energyScore = sum(scoreList)

    return energyScore, scoreList, minScore, maxScore


def calcPathSlope(hash, slopeDict, path):
    """
    hash is the mapping of points to IDs
    path is the list of points along the path
    slopeDict is the mapping of ID pairs to slope values
    """

    slopeScore = 0

    # convert points to ids
    nodeIDs = [hash[x] for x in path]
    # make pairs to create edges
    pairIDs = [(nodeIDs[i], nodeIDs[i + 1]) for i in range(len(nodeIDs) - 1)]
    # get step type for each pairIDs
    scoreList = [slopeDict[x] for x in pairIDs]
    minView = min(scoreList)
    maxView = max(scoreList)
    slopeScore = sum(scoreList)

    return slopeScore, scoreList, minView, maxView


def CrawlGeomParallel(To_Do, offsets, obstacle_geom, floor_geom, graph):
    # Takes in a list with a single start location: To_Do

    # generate directions
    maxStepConnection = offsets[9]
    # print(maxStepConnection)
    steps = list(range(1, maxStepConnection + 1))
    angleDirec = list(perm(steps + [-1 * x for x in steps], 2))
    angleDirec = [x for x in angleDirec if abs(x[0]) != abs(x[1])]
    initDirecs = list(prod(range(-1, 2), repeat=2))
    initDirecs.remove((0, 0))
    directions = set(initDirecs + angleDirec)

    def computerParent(parent):
        def direcs(parent):
            possible_children = []

            for direc in directions:
                i, j = direc
                # Offset the child from the parent
                child = [
                    round((parent[0] + (i * offsets[0])), 8),
                    round((parent[1] + (j * offsets[1])), 8),
                    round(parent[2] + offsets[2], 8),
                ]
                possible_children.append(child)  # Make a list of the possible children
            return possible_children

        possible_children = direcs(parent)

        children = getChildren(
            parent, possible_children, graph, floor_geom, obstacle_geom, offsets
        )
        # print(children)
        # children = getChildren8(parent,possible_children,graph,floor_geom,obstacle_geom,offsets)
        relations = []

        for item in children:
            child = item[0]
            if child is None:
                continue

            stepType = item[1]
            spacing = item[2]
            dst, angle, score = scoreRelation(parent, child, offsets)
            weight = [dst, angle, stepType, spacing, score]  # make a list of weights
            relations.append([parent, child, weight])

        if len(relations) == 0:
            print("No relations!")
            weight = [0, 0, 0, 0, 0]
            relations = [[parent, parent, [math.nan for x in weight]]]

        return relations

    while len(To_Do) != 0:
        if len(graph) > max_nodes:
            return graph
        # should also check how many objects there are, especially obstacles,
        # which has a big impact on speed
        parent = To_Do.pop(0)
        # parent = list(parent)
        relations = computerParent(parent)

        # should not happen if using adjacent grid
        if len(relations) > 0:
            for vals in relations:
                parent = (vals[0][0], vals[0][1], vals[0][2])
                child = (vals[1][0], vals[1][1], vals[1][2])
                child_arr = vals[1]  # store an array version instead of tuple
                weight = vals[2]

                # uses a defaultdict, but seems to be slower
                # graph[parent].append([child,weight])

                # If this parent is not in the graph, add it
                if parent not in graph:
                    graph.setdefault(parent, [[child, weight]])
                # If this parent is in the graph, update its children
                else:
                    graph[parent] = graph[parent] + [[child, weight]]

                # If the child is not in the graph, and not already listed, add it to queue
                if (child not in graph) and (child not in To_Do):
                    To_Do.append(child)

    return graph


def buildNetwork(start_location, offsets, bvh, maximum_nodes):
    # Round off the number so Rhino doesn't have too many issues?
    start_location = (
        round(start_location[0], 8),
        round(start_location[1], 8),
        round(start_location[2], 8),
    )

    global max_nodes
    max_nodes = maximum_nodes
    start_location = check_start(start_location, None)
    graph = {}
    return CrawlGeomParallel([start_location], offsets, None, bvh, graph=graph)


# def get_start():
#    start_object = rs.GetObject("select start point")
#    # loc =  rs.SurfaceVolumeCentroid(start_object)[0]
#    obj = rs.coerce3dpoint(start_object)
#    return (obj.X, obj.Y, 0)


# def get_end():
#    start_object = rs.GetObject("select end point")
#    # loc =  rs.SurfaceVolumeCentroid(start_object)[0]
#    obj = rs.coerce3dpoint(start_object)
#    return (obj.X, obj.Y, 0)


def getClosestNode(nodes, obj_loc):
    closest_node = None
    closest_dist = 99999999999
    for node in nodes:
        x = obj_loc[0] - node[0]
        y = obj_loc[1] - node[1]
        z = obj_loc[2] - node[2]
        dist = math.sqrt(x * x + y * y + z * z)
        if closest_dist >= dist:
            closest_dist = dist
            closest_node = node

    return closest_node


def graph_to_array(graph):
    graph_array = []
    for parent in graph:
        for child in graph[parent]:
            edge1 = parent
            edge2 = child[0]
            weight = child[1]
            graph_array.append([edge1, edge2, weight])
    return graph_array


def cal_point_weights(graph_dict):
    ptWt_dict = {}
    lowVal = math.nan
    highVal = -math.nan
    wtList = []
    for item in graph_dict:
        ptWt_dict[item] = 0
        list_pt = graph_dict[item]
        # if the length is 0, this is a bad node
        # it could happen when max_nodes is reached, so dont want to
        # hurt the scale of visualization
        if len(list_pt) == 0:
            ptWt_dict[item] = -1
            wtList.append(ptWt_dict[item])
            continue

        nodeWeights = []
        for i in range(len(list_pt)):
            ptWt_dict[item] += list_pt[i][1]  # weight of point
            nodeWeights.append(list_pt[i][1])

        ptWt_dict[item] = ptWt_dict[item] / len(list_pt)

        # Use the largest angle difference as the weight for the node
        # This works for angle analysis, but doesnt make sense for other weighting systems

        ptWt_dict[item] = abs(min(nodeWeights)) + abs(max(nodeWeights))

        if lowVal > ptWt_dict[item]:
            lowVal = ptWt_dict[item]
        if highVal < ptWt_dict[item]:
            highVal = ptWt_dict[item]
        wtList.append(ptWt_dict[item])


#        return ptWt_dict, wtList, lowVal, highVal


def extractGraphEdge(graph, idx):
    """
    Extract the graph edge 
    
    """
    graphDict = {}
    for parent in graph:
        for connection in graph[parent]:

            child = connection[0]
            weight = connection[1][idx]
            if parent not in graphDict:
                graphDict.setdefault(parent, [[child, weight]])
            # If this parent is in the graph, update its children
            else:
                graphDict[parent] = graphDict[parent] + [[child, weight]]

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

            # get vector of child
            cVec = [child[0] - parent[0], child[1] - parent[1]]

            perpChild = []
            # go through the other children and see if there is a perpendicular edge
            for others in graph[parent]:
                if child == others:
                    continue
                child2 = others[0]
                dst2 = others[1][0]
                # get vector of child
                cVec2 = [child2[0] - parent[0], child2[1] - parent[1]]

                dotProd = (cVec[0] * cVec2[0]) + (cVec[1] * cVec2[1])
                if abs(dotProd) < 0.001:
                    # they are perpendicular, so store the child
                    perpChild.append([child2, dst2])

            if len(perpChild) == 0:
                weight = dst
            else:
                # there is a perpendicular axis
                if len(perpChild) == 1:
                    weight = abs(child[2] - perpChild[0][0][2]) + perpChild[0][1]
                elif len(perpChild) == 2:
                    weight = (
                        abs(perpChild[0][0][2] - perpChild[1][0][2]) + perpChild[0][1]
                    )
                else:
                    # If this parent is not in the graph, add it
                    if parent not in graphDict:
                        graphDict.setdefault(parent, [[child, weight]])
                    # If this parent is in the graph, update its children
                    else:
                        graphDict[parent] = graphDict[parent] + [[child, weight]]

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
            # cSlope =
            g = math.tan(math.radians(slope))
            # if slope is out of the range of the function set it to the limits
            if g < -0.4:
                g = -0.4
            if g > 0.4:
                g = 0.4
            e = (
                280.5 * (g ** 5)
                - 58.7 * (g ** 4)
                - 76.8 * (g ** 3)
                + 51.9 * (g ** 2)
                + 19.6 * (g)
                + 2.5
            )

            if e < 0:
                weight = e * dst

            #             # if weight > 1:             #If this parent is not in the graph, add it
            if parent not in graphDict:
                graphDict.setdefault(parent, [[child, weight]])
            # If this parent is in the graph, update its children
            else:
                graphDict[parent] = graphDict[parent] + [[child, weight]]

    return graphDict


def graphCreator(
    floor_path,
    start_point,
    end_point,
    x_offset_input,
    y_offset_input,
    z_offset_input,
    downstep,
    stepheight,
    upslope,
    downslope,
    slope,
    width,
    dis,
    maxNodes,
    maxStepConnection,
):

    global csdisp, max_nodes
    csdisp = dis

    max_nodes = maxNodes

    start_location = start_point

    offsets = [
        x_offset_input,
        y_offset_input,
        z_offset_input,
        stepheight,
        width,
        downstep,
        slope,
        upslope,
        downslope,
        maxStepConnection,
    ]

    # Make sure to add the height (offsets[2]) so the ray can shoot down 'into' geometery
    start_location = (
        start_location[0],
        start_location[1],
        start_location[2] + offsets[2],
    )

    # TODO: setup geometry
    # one group of geometry for walkable surfaces
    # individual groups of geometry for different types of surfaces
    # one group for not walkable surfaces
    # one group of all surfaces that would be obstacles (for width detection)

    # TODO: multiple graphs
    # one graph for distance
    # one graph for slope
    # one graph for surface type

    # Generate the graph
    start = time.time()
    graph = buildNetwork(start_location, offsets, floor_geom, obstacle_layer)
    nodes = graph.keys()

    return graph, graph, nodes

