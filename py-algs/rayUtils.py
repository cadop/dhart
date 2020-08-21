#Rhino Imports 
try: 
    import Rhino.Geometry.Point3d as Point3d
    import Rhino.Geometry.Vector3d as Vector3d
    import Rhino.Geometry.Ray3d as Ray3d
    from Rhino.Geometry.Intersect.Intersection import MeshRay as RhinoMeshRay

    #Custom Ray Imports
    from humanMetrics import *
    # import HumanMetrics.RayIntersections as RayIntersections
    from HumanMetrics.RayIntersections import MeshRay 
except:
    print('Not in Rhino Environment')

import mathUtils as mu
import time
import math

from humanfactorspy.raytracer import (EmbreeBVH,Intersect,
                                        IntersectForPoint,
                                        IntersectOccluded,
                                        IntersectDistanceDouble)

checked_rays_mod = {}
# checked_rays_dst = {}

rays_cast = 0
ray_time = 0

def cast_ray(geom, p1, dir, pos=False ):
    """ Use the Embree Raytracer to get the distance of a ray intersection """
    if pos:
        return IntersectForPoint(geom, p1, dir)

    dist = IntersectDistanceDouble(geom, (p1[0], p1[1], p1[2]), (dir[0], dir[1], dir[2]))
    
    return dist
    return mu.trunc(dist,p=16) 


def rhinoRayMod(geom,p1,direc):
    '''
    Helper function for the custom rhino ray intersection methods

    Takes in a mesh, start point, and direction to shoot the ray
    '''
    
    # start_t = time.time()
    # global rays_cast
    # global ray_time

    # t_p1 =  tuple(p1)
    # t_d = tuple(direc)

    # #check if we already have this ray
    # if ( t_p1,t_d ) in checked_rays_mod: 
    #     # ray_time+= time.time()-start_t

    #     return checked_rays_mod[(t_p1,t_d)]

    ray = makeRay(p1,direc)
    res = MeshRay(geom,[ray])[0]

    # #store the ray calculation 
    # checked_rays_mod[(t_p1,t_d)] = res

    # rays_cast+=1
    # ray_time+= time.time()-start_t

    return res


def makeRay(p1,p2):
    """
    Input a point xyz and direction xyz
    Returns a Rhino ray object
    """

    #maybe not necessary, remove later if finding out its wasting time    
    p2 = mu.unitVec(p2)

    start = Point3d(float(p1[0]), float(p1[1]), float(p1[2]))
    direc = Vector3d(float(p2[0]), float(p2[1]), float(p2[2]))

    # return Rhino.Geometry.Ray3d(start, direc )
    return Ray3d(start, direc )

def makePoint3D(p):
    """
    Input a tuple or array of xyz
    Returns a Rhino point3d object
    """
    # return Rhino.Geometry.Point3d(p[0],p[1],p[2])    
    return Point3d(p[0],p[1],p[2])  

def rayDist(p1,p2,geometry,type=3):

    # t_p1 = tuple(p1)
    # t_p2 = tuple(p2)

    # if (t_p1,t_p2) in checked_rays_dst:
    #     return checked_rays_dst[(t_p1,t_p2)]

    start_point = Point3d(float(p1[0]), float(p1[1]), float(p1[2]))
    direction = Vector3d(float(p2[0]), float(p2[1]), float(p2[2]))
    ray = Ray3d(start_point, direction)
    bounce = 1
    #type 3 is mesh intersections
    if type == 3:
        hits = []
        for mesh in geometry:
            intPt = RhinoMeshRay(mesh,ray)

            # print('intPt:',intPt)
            #rays+=1
            if intPt < 0: continue
            hits.append(intPt)
        
        minhit = None
        mindst = 99999999999
        for hit in hits:
            #find the closest hit 
            dst = hit
            if dst<mindst:
                mindst = dst
                minhit = hit       
        #ray_time += time.time()-stime
        if minhit == None: 
            # checked_rays_dst[(t_p1,t_p2)] = None
            return None
        else: 
            # checked_rays_dst[(t_p1,t_p2)] = minhit
            return minhit
        
def check_ray(p1,p2,geometry,type=3):
    #    Takes in two arguments
    #    each argument is an array of 3 points
    start_point = Point3d(float(p1[0]), float(p1[1]), float(p1[2]))
    direction = Vector3d(float(p2[0]), float(p2[1]), float(p2[2]))
    ray = Ray3d(start_point, direction)
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
        hits = []
        for mesh in geometry:
            intPt = Rhino.Geometry.Intersect.Intersection.MeshRay(mesh,ray)
            if intPt == -1: continue
            #should be able to use:
            # intPt = ray.PointAt(intPt)
            intPt = start_point[0],start_point[1],round(start_point[2]-intPt,8)
            hits.append(intPt)
        
        minhit = None
        mindst = 99999999999
        for hit in hits:
            #find the closest hit 
            # dst = start_point.DistanceTo( Rhino.Geometry.Point3d(hit[0],hit[1],hit[2]) )
            dst = start_point.DistanceTo( Point3d(hit[0],hit[1],hit[2]) )
            if dst<mindst:
                mindst = dst
                minhit = hit          
        
        if minhit == None: return None
        else: return minhit