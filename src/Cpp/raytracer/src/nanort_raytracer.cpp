#define NANORT_USE_CPP11_FEATURE 
#include "nanort.h"
#include "meshinfo.h"
#include "objloader.h"
#include "ray_data.h"
#include <iostream>
#include <fstream>
#include <vector>

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

} // end namespace