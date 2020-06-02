import pytest
import unittest
import os
import pywavefront
import itertools

from humanfactorspy.Exceptions import InvalidOBJException, FileNotFoundException
from humanfactorspy.geometry import MeshInfo, LoadOBJ, OBJGroupType, CommonRotations


# ---- TESTING CONSTANTS ----
good_mesh_path = "Example Models\\sponza.obj"
good_mesh_paths = ["Example Models\\sponza.obj", "Example Models\\sibenik.obj"]
bad_mesh_paths = ["BadMeshTHatDoesntExist", "Example Models\\sibenik.obj"]
bad_mesh_path = "MeshPathThatDoesntExist"
good_db_path = "sql\\Database.db"
bad_db_path = good_mesh_path


def test_GoodMesh():
    hfc = LoadOBJ(good_mesh_path, rotation=CommonRotations.Yup_to_Zup)

def test_GoodMeshOBJLoader():
    hfc = LoadOBJ(good_mesh_path, rotation=CommonRotations.Yup_to_Zup)

def test_BadMesh():
    with pytest.raises(FileNotFoundException):
        hfc = LoadOBJ(bad_mesh_path)


def test_LoadFromVertices():
    scene = pywavefront.Wavefront(good_mesh_path, collect_faces=True)
    vertices = scene.vertices
    meshes = scene.mesh_list
    indices = list(itertools.chain.from_iterable([mesh.faces for mesh in meshes]))
    MI = MeshInfo(indices, vertices, "TestMesh", 39)

def test_rotation():
    mesh = LoadOBJ(good_mesh_path)
    mesh.Rotate((90,0,0))