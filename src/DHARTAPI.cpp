// This file doesn't do anything. it only exists to appease cmake
// since cmake won't let you build a library without atleast one source
#include <DHARTAPI.h>
#include <embree_raytracer.h>
#include <OBJLoader.h>
#include <string>
#include <iostream>
#include <MeshInfo.h>

int CountVertices()
{
	auto MI = HF::Geometry::LoadMeshObjects("teapot.obj");
	HF::RayTracer::EmbreeRayTracer RT(MI);
	int vert_count = MI[0].NumVerts();
	return vert_count;
}
