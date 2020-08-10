#pragma once
#include "nanort.h"
#include "meshinfo.h"
#include <iostream>
#include <array>

namespace {

    // Define struct of doubles to be used for ray and high precision raycasting
    struct double3 {
        double3() {}
        double3(double xx, double yy, double zz) {
            x = xx;
            y = yy;
            z = zz;
        }
        double3(const double* p) {
            x = p[0];
            y = p[1];
            z = p[2];
        }
        double3 operator*(double f) const { return double3(x * f, y * f, z * f); }
        double3 operator-(const double3& f2) const {
            return double3(x - f2.x, y - f2.y, z - f2.z);
        }
        double3 operator*(const double3& f2) const {
            return double3(x * f2.x, y * f2.y, z * f2.z);
        }
        double3 operator+(const double3& f2) const {
            return double3(x + f2.x, y + f2.y, z + f2.z);
        }
        double3& operator+=(const double3& f2) {
            x += f2.x;
            y += f2.y;
            z += f2.z;
            return (*this);
        }
        double3 operator/(const double3& f2) const {
            return double3(x / f2.x, y / f2.y, z / f2.z);
        }
        double operator[](int i) const { return (&x)[i]; }
        double& operator[](int i) { return (&x)[i]; }

        double3 neg() { return double3(-x, -y, -z); }

        double length() { return sqrt(x * x + y * y + z * z); }

        void normalize() {
            double len = length();
            if (fabs(len) > 1.0e-6) {
                double inv_len = 1.0 / len;
                x *= inv_len;
                y *= inv_len;
                z *= inv_len;
            }
        }

        double x, y, z;
        // double pad;  // for alignment
    };

} // namespace

// Derive our own class from the triangle intersector so we can store some extra data
class nanoRT_Data :
    public nanort::TriangleIntersector<double, nanort::TriangleIntersection<double> > {

public:

    // Don't let the default empty constructor work since it needs the mesh
    nanoRT_Data() = delete;
    // Add a mesh object
    HF::nanoGeom::Mesh mesh;
    // Add a ray object to be used for intersections
    nanort::Ray<double> ray;
    // Add a hit object to be referenced
    nanort::TriangleIntersection<double> hit;
    // Add a distance attribute to store intersection distance
    double dist = -1;
    double point[3] = {-1,-1,-1};

    // Set initialization of class by passing a mesh to create a nanort::TriangleIntersector
    nanoRT_Data(HF::nanoGeom::Mesh mesh) : nanort::TriangleIntersector<double, nanort::TriangleIntersection<double> >(mesh.vertices, mesh.faces, sizeof(double) * 3)
    {
        // Set the mesh data
        nanoRT_Data::mesh = mesh;

        // Setup a no hit (for safety)
        hit.u = -1;
        hit.v = -1;
        hit.t = -1;
        hit.prim_id = -1;
        
        // Setup the ray
        // Set origin of ray
        nanoRT_Data::ray.org[0] = 0.0;
        nanoRT_Data::ray.org[1] = 0.0;
        nanoRT_Data::ray.org[2] = 0.0;

        // Define direction of ray
        // Must be normalized to work properly

        // Custom double vector implementation with overloads
        // not needed in basic test but later could be useful for node additions etc. 
        //double3 dir(0,0,0); 
        //nanoRT_Data::ray.dir[0] = dir[0];
        //nanoRT_Data::ray.dir[1] = dir[1];
        //nanoRT_Data::ray.dir[2] = dir[2];

        nanoRT_Data::ray.dir[0] = 0.0;
        nanoRT_Data::ray.dir[1] = 0.0;
        nanoRT_Data::ray.dir[2] = 0.0;

        // Set max and min location 
        nanoRT_Data::ray.min_t = 0.0f;
        nanoRT_Data::ray.max_t = 20000.0f;
    }

    // Destructor
    ~nanoRT_Data() {
        std::cout << "destroyed" << std::endl;
        // Mesh data was constructed with new, maybe it needs to be deleted here?
    }


};

namespace HF::nanoGeom {

    // Convenience method not used now but here for clarity
    bool nanoRT_RayCast(nanort::BVHAccel<double>& accel,
        nanort::TriangleIntersector<double, nanort::TriangleIntersection<double> >& triangle_intersector,
        nanort::Ray<double>& ray,
        nanort::TriangleIntersection<double>& isect);

    // Interface to nanoRT BVH
    nanort::BVHAccel<double> nanoRT_BVH(Mesh& mesh);

    bool nanoRT_Intersect(Mesh& mesh, nanort::BVHAccel<double>& accel, nanoRT_Data& intersector);
}