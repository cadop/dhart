try:
    import rhinoscriptsyntax as rs
    import Rhino
    import scriptcontext
    import System
    from Rhino.Geometry import *
    from humanMetrics import *
    import HumanMetrics.RayIntersections as RayIntersections
except:
    print('Not in Rhino Environment')

from humanfactorspy.geometry import LoadOBJ
from humanfactorspy.raytracer import EmbreeBVH
import humanfactorspy

def get_bvh(obj_path,use_precise=False):
    """ Get the BVH for a obj

    """

    # Load mesh
    loaded_obj = LoadOBJ(obj_path)
    # Create BVH
    bvh = EmbreeBVH(loaded_obj,use_precise)

    return bvh

def getOpaque(opaqueObjects='ViewObstruction'):
    '''
    Helper function for turning objects on a given layer into a mesh
    used in visability analysis
    '''
    #Get floor objects and save
    objs = Rhino.RhinoDoc.ActiveDoc.Objects.FindByLayer(opaqueObjects)
    #use rhinocommon to convert to mesh for analysis
    res = objs2mesh(objs)      

    return [res]

def getLayerObjsP(layername = None,mesh=True):
    """
    input a layer name
    if mesh=True, all objects will be converted to a mesh
    """
    if type(layername) == type(None):
        return False
    
    objs = Rhino.RhinoDoc.ActiveDoc.Objects.FindByLayer(layername)
    #jaggedAndFaster = MeshingParameters.Coarse

    minimal = Rhino.Geometry.MeshingParameters.Minimal    
    brepMesh = Rhino.Geometry.Mesh()
    
       
    newObjs = []
    for obj in objs:
        brep = rs.coercebrep(obj)
        if brep == None: 
            if obj.ObjectType == brepMesh.ObjectType:
                newObjs.append(obj.MeshGeometry)
            continue
        newObjs.append(brep)
        
        #meshes = Mesh.CreateFromBrep(brep, minimal)
        # if meshes == None or meshes.Length == 0:
            # return Result.Failure
        # for mesh in meshes:
            # brepMesh.Append(mesh)

    return [RayIntersections.CombineMeshes(newObjs)]

    
def getLayerObjs(layername = None,mesh=True):
    """
    input a layer name
    if mesh=True, all objects will be converted to a mesh
    """
    if type(layername) == type(None):
        return False
    
    objs = Rhino.RhinoDoc.ActiveDoc.Objects.FindByLayer(layername)
    if mesh == True:
        return [objs2mesh(objs)]
    
    #TODO workout returning breps
    return False
    
    geom_group = []
    for obj in objs:
        if ( rs.IsMesh(obj) or rs.IsSurface(obj) or rs.IsPolysurface(obj) ): #only compare mesh or surfaces
            geom = rs.coercebrep(obj)
            geom = rs.coercemesh(obj)
            geom_group.append(geom)

    geometry = [rs.coercemesh(x) for x in geom_group]
    
        
def objs2mesh(geoms):
    """
    Input a list of geometries
    Return a single mesh constructed of breps and existing meshes
    """
    #jaggedAndFaster = MeshingParameters.Coarse
    minimal = Rhino.Geometry.MeshingParameters.Minimal
            
    brepMesh = Rhino.Geometry.Mesh()
            
    for obj in geoms:
        brep = rs.coercebrep(obj)
        if brep == None: 
            if obj.ObjectType == brepMesh.ObjectType:
                brepMesh.Append(obj.MeshGeometry)
            continue
        meshes = Mesh.CreateFromBrep(brep, minimal)
        if meshes == None or meshes.Length == 0:
            return Result.Failure
        for mesh in meshes:
            brepMesh.Append(mesh)
    return brepMesh
                
        
def MeshOptions(a,b,c,d,e,f,g,h,i,j,k):
    mo="_DetailedOptions _JaggedSeams={} _PackTextures={} ".format(a,b)
    mo+="_Refine={} _SimplePlane={} _AdvancedOptions _Angle={} ".format(c,d,e)
    mo+="_AspectRatio={} _Distance={} _Density={} _Grid={} ".format(f,g,h,i)
    mo+="_MaxEdgeLength={} _MinEdgeLength={} _Enter _Enter".format(j,k)
    return mo
    
def MeshConverter(layer=None):
    #If the input is not none, select objects to convert by a layer name
    if layer!=None:
        objs = rs.ObjectsByLayer(layer)
    else: objs = rs.AllObjects()
    obj_list = []
    #Only convert non-meshes 
    for obj in objs:
        if rs.IsSurface(obj) or rs.IsPolysurface(obj):
            obj_list.append(obj)
            
    #rs.GetObjects(msg,8+16,preselect=True,select=True)
    rs.SelectObjects(obj_list)
    rs.Command("_-Mesh "+MeshOptions("No","Yes","Yes","Yes",0,0,0.01,0,0,0,0))
    lco=rs.LastCreatedObjects()
    t_meshes=[rs.MeshQuadsToTriangles(obj) for obj in lco]
    
    return lco    
    
def hideMeshLayer(analysisLayer):
    print(rs.LayerVisible(analysisLayer,visible=False) )
    
def moveObjectLayer(objs,layer_name):
    layer_index = scriptcontext.doc.Layers.Find(layer_name, True)
    if layer_index>=0: pass
    else: layer_index = scriptcontext.doc.Layers.Add(layer_name, System.Drawing.Color.Black)
        
    if layer_index<0:
        print("Unable to add Layer name", layer_name, "layer.")
        return Rhino.Commands.Result.Failure

    for id in objs:
        rs.ObjectLayer(id,layer_name)

def copyMeshes(layer,lco):
    objs = rs.ObjectsByLayer(layer)
    meshes = []
    for obj in objs:
        if rs.IsMesh(obj):
            obj_copy = rs.CopyObjects(obj)
            meshes.append(obj_copy)
            
    return lco+meshes

def createAnalysisLayer(targetLayer,hide=True):
    
    convertedObjs = MeshConverter(targetLayer)
    analysisObjects = copyMeshes(targetLayer,convertedObjs)
    analysisObjects = rs.JoinMeshes(analysisObjects,delete_input=True)
    
    analysisLayer = targetLayer+'_conv'
    moveObjectLayer([analysisObjects],analysisLayer)
    if hide: hideMeshLayer(analysisLayer)
    return analysisLayer    