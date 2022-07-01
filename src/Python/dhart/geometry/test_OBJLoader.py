import pytest
import unittest
import os
import pywavefront
import itertools

from humanfactorspy.Exceptions import InvalidOBJException, FileNotFoundException
from humanfactorspy.geometry import MeshInfo, LoadOBJ, OBJGroupType, CommonRotations

import humanfactorspy

# ---- TESTING CONSTANTS ----
good_mesh_path = humanfactorspy.get_sample_model("sponza.obj")
bad_mesh_path = "MeshPathThatDoesntExist"


def test_GoodMesh():
    hfc = LoadOBJ(good_mesh_path, rotation=CommonRotations.Yup_to_Zup)

def test_GoodMeshOBJLoader():
    hfc = LoadOBJ(good_mesh_path, rotation=CommonRotations.Yup_to_Zup)

def test_BadMesh():
    with pytest.raises(FileNotFoundException):
        hfc = LoadOBJ(bad_mesh_path)

def test_LoadMultipleMeshes():
    meshes = LoadOBJ(good_mesh_path, OBJGroupType.BY_GROUP)

    # This should crash if the meshes weren't loaded correctly
    for mesh in meshes:
        mesh.Rotate((90,0,0))

def test_LoadFromVerticesEqualsOBJ():
    # Get mesh data from pywavefront
    scene = pywavefront.Wavefront(good_mesh_path, collect_faces=True)
    vertices = scene.vertices
    meshes = scene.mesh_list
    indices = list(itertools.chain.from_iterable([mesh.faces for mesh in meshes]))

    # Insert into MeshInfo object
    MI = MeshInfo(indices, vertices, "TestMesh", 39)

    assert(MI.id == 39)
    assert(MI.name == "TestMesh")

def test_rotation():
    mesh = LoadOBJ(good_mesh_path)
    mesh.Rotate((90,0,0))
