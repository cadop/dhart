import math

from itertools import permutations as perm
from itertools import combinations as combi
from itertools import product as prod

def dst(p1,p2):
    return math.sqrt( (p1[0]-p2[0])**2 + (p1[1]-p2[1])**2 + (p1[2]-p2[2])**2 )
    
def makeVec(p1,p2):
    """
    From p1 to p2, construct a vector
    """
    return [p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]]

def vecAng(v1,v2):
    """
    returns vector between two angles
    """

    vlen = len(v1)

    vsize = math.sqrt(sum([ math.pow( (v1[i]),2) for i in range(vlen)  ]))
    uv1 = [  v1[i]/vsize for i in range(vlen) ]

    vsize = math.sqrt(sum([ math.pow( (v2[i]),2) for i in range(vlen)  ]))
    uv2 = [  v2[i]/vsize for i in range(vlen) ]

    dotProd = sum( [ uv1[i]*uv2[i] for i in range(vlen)  ] )

    theta = math.acos( dotProd/vsize )
    theta = math.degrees(theta) # convert to degrees

    return theta 

def KMh2Mm(kmh):
    """
    convert kilometers/hour to meters/minute
    """
    mm = kmh*16.6667
    return mm


def xyz2sp(p):
    """
    convert a point from cartesian to spherical coordinates
    """
    r = math.sqrt(sum(i**2 for i in p))
    phi = math.atan2(p[1],p[0])
    theta = math.acos( p[2]/r  )
    
    return [r,phi,theta]

def sp2xyz(p):
    """
    convert a point from spherical to cartesian coordinates
    """
    r,phi,theta = p
    
    x = r*math.sin(theta)*math.cos(phi)
    y = r*math.sin(theta)*math.sin(phi)
    z = r*math.cos(theta)
    
    return [x,y,z]

def dotAngle(v1,v2):
    """
    return the dot product and angle (in degrees) between two vectors
    """
    dot = sum( [ v1[i]*v2[i] for i in range(len(v1)) ] )
    
    v1L = math.sqrt(sum(i**2 for i in v1))
    v2L = math.sqrt(sum(i**2 for i in v2))
    
    dotAng = math.acos( dot/ (v1L*v2L) )
    dotDeg = math.degrees(dotAng)
    
    return dotDeg,dot

def dotProd(v1,v2):
    '''
    Get the dot product between v1 and v2
    '''
    dotProd = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] 
    return dotProd

def vecMag(v):
    """
    get length of a vector v
    """
    
    vsize = math.sqrt( math.pow( (v[0]),2) + 
                      math.pow( (v[1]),2) +
                      math.pow( (v[2]),2) )
    return vsize

def unitVec(v):
    """
    Take in a vector
    Return the new unit vector
    """
    uv = [0,0,0]
    vsize = math.sqrt(  (v[0])**2 + 
                        (v[1])**2 +
                        (v[2])**2 )
                      
    uv[0] = v[0]/vsize 
    uv[1] = v[1]/vsize
    uv[2] = v[2]/vsize
                
    return uv

def translatePoint(p,v,d):
    """
    Take in a point, vector, and distance to translate
    Return the new translated point 
    """
    p2 = [0,0,0]
    vsize = math.sqrt( math.pow( (v[0]),2) + 
                      math.pow( (v[1]),2) +
                      math.pow( (v[2]),2) )
                      
    vscale = ( d/vsize )
    p2[0] = ( v[0]*( d/vsize ) ) + p[0]
    p2[1] = ( v[1]*( d/vsize ) ) + p[1]
    p2[2] = ( v[2]*( d/vsize ) ) + p[2]
                
    return p2

def genDirections(maxSteps):
    '''
    Used for generating multiple pairs of integers that should be connected
    to a given location. This creates smaller angles within a subgraph

    Take in an integer 
    Return the pairs of directions 
    '''
    steps = list(range(1,maxSteps+1))
    angleDirec = list(perm(steps + [-1*x for x in steps],2))
    angleDirec = [ x for x in angleDirec if abs(x[0]) != abs(x[1]) ]
    initDirecs = list(prod(range(-1,2),repeat=2))
    initDirecs.remove((0,0))
    directions = set(initDirecs+angleDirec)
    return directions