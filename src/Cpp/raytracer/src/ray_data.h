#pragma once
#define NANORT_USE_CPP11_FEATURE
#include "nanort.h"
#include "meshinfo.h"
#include <iostream>
#include <array>
#include <meshinfo.h>


namespace HF::RayTracer{
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
    double point[3] = { -1,-1,-1 };

    // Set initialization of class by passing a mesh to create a nanort::TriangleIntersector
    nanoRT_Data(HF::nanoGeom::Mesh mesh) : nanort::TriangleIntersector<double, nanort::TriangleIntersection<double> >(mesh.vertices, mesh.faces, sizeof(double) * 3)
    {
        // Set the mesh data
        this->mesh = mesh;

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

    // Destructor
    ~nanoRT_Data() {
        // Mesh data was constructed with new, delete here
        delete[] this->mesh.vertices;
        delete[] this->mesh.faces;
        std::cout << "Destroyed" << std::endl;
    }
};



namespace HF::nanoGeom {

    // Convenience method not used now but here for clarity
    bool nanoRT_RayCast(nanort::BVHAccel<double>& accel,
        nanort::TriangleIntersector<double, nanort::TriangleIntersection<double> >& triangle_intersector,
        nanort::Ray<double>& ray,
        nanort::TriangleIntersection<double>& isect);


    template <typename T>
    inline nanort::BVHAccel<T> nanoRT_BVH(unsigned int * indices, T * vertices, int num_vertices, int num_indices)
    {
        // Setup nanort tracer BVH options
        nanort::BVHBuildOptions<T> build_options; // Use default option
        build_options.cache_bbox = false;

        // Construct datatype using verts and indices for building BVH
        nanort::TriangleMesh<T> triangle_mesh(vertices, indices, sizeof(T) * 3);
        nanort::TriangleSAHPred<T> triangle_pred(vertices, indices, sizeof(T) * 3);

        // build BVH using NanoRT Method (Replace this assert with an exception)
        nanort::BVHAccel<T> accel;
        assert(accel.Build(num_indices, triangle_mesh, triangle_pred, build_options));

        // Return the BVH object
        return accel;
    }
    inline nanort::BVHAccel<double> nanoRT_BVH(Mesh mesh) {
        return nanoRT_BVH(mesh.faces, mesh.vertices, mesh.num_vertices, mesh.num_faces);
    }

    bool nanoRT_Intersect(Mesh& mesh, nanort::BVHAccel<double>& accel, nanoRT_Data& intersector);
}
namespace HF::RayTracer {

    class NanoRTRayTracer {

    private:
        using vertex_t = double;
        using real_t = double;
        using Intersection = nanort::TriangleIntersection<real_t>;
        using Intersector = nanort::TriangleIntersector<vertex_t, Intersection>;
        using NanoBVH = nanort::BVHAccel<vertex_t>;
        using NanoRay = nanort::Ray<real_t>;

        // Add a hit object to be referenced
        std::unique_ptr<Intersector> intersector; ///< Triangle Intersector
        NanoBVH bvh; ///< A NanoRT BVH 

        std::vector<real_t> vertices; //< Internal vertex array
        std::vector<unsigned int> indices; //< Internal index array

        template <typename N, typename dist_type = real_t>
        inline NanoRay ConstructRay(const N& origin, const N& direction, dist_type min_dist = 0.0, dist_type max_dist = std::numeric_limits<dist_type>::max()) {
            NanoRay out_ray;
            out_ray.org[0] = origin[0]; out_ray.org[1] = origin[1]; out_ray.org[2] = origin[2];
            out_ray.dir[0] = direction[0]; out_ray.dir[1] = direction[1]; out_ray.dir[2] = direction[2];
            out_ray.min_t = min_dist; out_ray.max_t = max_dist;

            return out_ray;
        }

        inline Intersection CreateHit() {
            Intersection out_hit;
            out_hit.u = -1; out_hit.v = -1; out_hit.t = -1; out_hit.prim_id = -1;
            return out_hit;
        }

        template <typename point_type, typename dist_type>
        inline void MovePoint(point_type & point, const point_type & dir, dist_type dist) {
            point[0] += (dir[0] * dist);
            point[1] += (dir[1] * dist);
            point[2] += (dir[2] * dist);
        }

    public:

        /*! \brief Construct a new raytracer with an instance of meshinfo*/
        inline NanoRTRayTracer(const HF::Geometry::MeshInfo& MI){

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
            bvh = HF::nanoGeom::nanoRT_BVH<vertex_t>(indices.data(), vertices.data(), vertices.size()/3, indices.size()/3);
            
            // Create a new intersector. Note: This can't be held as a member by value since you can't even construct this object without
            // the proper input arguments, however the input arguments cannot be created until we've copied the data from
            // the mesh and converted it to the proper types. Using a pointer allows us to construct an intersector later.
            intersector = std::unique_ptr<Intersector>(new Intersector(vertices.data(), indices.data(), sizeof(real_t) * 3));
        }

        template<typename point_type>
        inline bool PointIntersection(
            point_type & origin,
            const point_type & dir,
            float distance = -1,
            int mesh_id = -1
        ) {
            // Construct the ray and hit
            NanoRay ray = ConstructRay(origin, dir, 0);
            Intersection Hit = CreateHit();

            // Cast the ray
            bool did_intersect = bvh.Traverse<Intersector>(ray, *(this->intersector), &Hit);

            // It it intersected, move the node and return true, otherwise do nothing and return false.
            if (did_intersect) {
                MovePoint(origin, dir, Hit.t);
                return true;
            }
            else
                return false;
        }
    };
}
