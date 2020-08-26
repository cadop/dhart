#define NANORT_USE_CPP11_FEATURE 
#include <ray_data.h>
#include <meshinfo.h>
#include "nanort.h"
#include <iostream>
#include <fstream>
#include <vector>


nanoRT_Data::nanoRT_Data(HF::nanoGeom::Mesh * m) : 
    nanort::TriangleIntersector<double, nanort::TriangleIntersection<double>>
    (m->vertices, m->faces, sizeof(double) * 3)
{
	// Set the mesh data
	this->mesh = m;

	// Setup a no hit (for safety)
	hit.u = -1;
	hit.v = -1;
	hit.t = -1;
	hit.prim_id = -1;

	// Setup the ray
	// Set origin of ray
	this->ray.org[0] = 0.0;
	this->ray.org[1] = 0.0;
	this->ray.org[2] = 0.0;

	this->ray.dir[0] = 0.0;
	this->ray.dir[1] = 0.0;
	this->ray.dir[2] = 0.0;

	// Set max and min location 
	this->ray.min_t = 0.0f;
	this->ray.max_t = 20000.0f;
}

nanoRT_Data::~nanoRT_Data() {
        // Mesh data was constructed with new, delete here
        delete[] this->mesh->vertices;
        delete[] this->mesh->faces;
        std::cout << "Destroyed" << std::endl;
}

namespace HF::nanoGeom {

    bool nanoRT_RayCast(nanort::BVHAccel<double>& accel,
        nanort::TriangleIntersector<double, nanort::TriangleIntersection<double> >& triangle_intersector,
        nanort::Ray<double>& ray,
        nanort::TriangleIntersection<double>& isect)
    {
        /*
        Not used
        Just a clarity function for now to show that the Raycasting really needs a list of inputs
        but the class is abstracting them.

        */


        // Performs an intersection of a ray and mesh (via BVH object)
        // Modifies the input (by reference) isect 

        // perform intersection of ray on BVH
        // &isect is modified and the u,v,t values of intersection are changed in place
        bool hit = accel.Traverse(ray, triangle_intersector, &isect);
        if (hit)
            return true;

        return false;
    }

    nanort::BVHAccel<double> nanoRT_BVH(Mesh& mesh)
    {
        bool ret = false;

        // Setup nanort tracer BVH options
        nanort::BVHBuildOptions<double> build_options; // Use default option
        build_options.cache_bbox = false;

        // Construct datatype using verts and indices for building BVH
        nanort::TriangleMesh<double> triangle_mesh(mesh.vertices, mesh.faces, sizeof(double) * 3);
        nanort::TriangleSAHPred<double> triangle_pred(mesh.vertices, mesh.faces, sizeof(double) * 3);

        // build BVH
        nanort::BVHAccel<double> accel;
        ret = accel.Build(mesh.num_faces, triangle_mesh, triangle_pred, build_options);

        // This shouldn't fail
        assert(ret);

        // Return the BVH object
        return accel;
    }
    nanort::BVHAccel<double> nanoRT_BVH(Mesh mesh) {
        return nanoRT_BVH(mesh.faces, mesh.vertices, mesh.num_vertices, mesh.num_faces);
    }

    bool nanoRT_Intersect(Mesh& mesh, nanort::BVHAccel<double>& accel, nanoRT_Data& intersector)
    {
        // Performs an intersection of a ray and mesh (via BVH object)
        // Modifies the input (by reference) isect 

        // perform intersection of ray on BVH
        // &isect is modified and the u,v,t values of intersection are changed in place
        bool hit = accel.Traverse(intersector.ray, intersector, &intersector.hit);
        if (hit) {
            // Translate the point along the direction vector 
            intersector.point[0] = intersector.ray.org[0] + (intersector.ray.dir[0] * intersector.hit.t);
            intersector.point[1] = intersector.ray.org[1] + (intersector.ray.dir[1] * intersector.hit.t);
            intersector.point[2] = intersector.ray.org[2] + (intersector.ray.dir[2] * intersector.hit.t);

            return true;
        }


        return false;
    }
}; // end namespace

namespace HF::RayTracer{
   
    NanoRTRayTracer::NanoRTRayTracer(const HF::Geometry::MeshInfo& MI) {
        // Get the index and vertex arrays of the meshinfo
        auto mi_vertices = MI.GetVertexPointer().CopyArray();
        vertices.resize(mi_vertices.size());
        for (int i = 0; i < mi_vertices.size(); i++)
            vertices[i] = static_cast<vertex_t>(mi_vertices[i]);

        // Convert indices to unsigned integer because that's what nanoRT uses
        auto mi_indices = MI.GetIndexPointer().CopyArray();
        indices.resize(mi_indices.size());
        for (int i = 0; i < mi_indices.size(); i++)
            indices[i] = static_cast<unsigned int>(mi_indices[i]);

        // Build the BVH
        bvh = HF::nanoGeom::nanoRT_BVH<vertex_t>(indices.data(), vertices.data(), vertices.size() / 3, indices.size() / 3);

        // Create a new intersector. Note: This can't be held as a member by value since you can't even construct this object without
        // the proper input arguments, however the input arguments cannot be created until we've copied the data from
        // the mesh and converted it to the proper types. Using a pointer allows us to construct an intersector later.
        intersector = std::unique_ptr<Intersector>(new Intersector(vertices.data(), indices.data(), sizeof(real_t) * 3));
    }
}