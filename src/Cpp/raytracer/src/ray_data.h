#pragma once
#define NANORT_USE_CPP11_FEATURE
#include "nanort.h"
#include <HitStruct.h>
#include <iostream>
#include <array>

#undef max
// Forward Declares
namespace HF::Geometry {
    template <typename T> class MeshInfo;
}
namespace HF::nanoGeom {
    struct Mesh;
}


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
    HF::nanoGeom::Mesh * mesh;
    // Add a ray object to be used for intersections
    nanort::Ray<double> ray;
    // Add a hit object to be referenced
    nanort::TriangleIntersection<double> hit;
    // Add a distance attribute to store intersection distance
    double dist = -1;
    double point[3] = { -1,-1,-1 };

    // Set initialization of class by passing a mesh to create a nanort::TriangleIntersector
    nanoRT_Data(HF::nanoGeom::Mesh * mesh);

    // Destructor
    ~nanoRT_Data();
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
        accel.Build(num_indices, triangle_mesh, triangle_pred, build_options);

        // Return the BVH object
        return accel;
    }
    nanort::BVHAccel<double> nanoRT_BVH(Mesh mesh);

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

        const real_t min_dist = 0.0;
        std::vector<real_t> vertices; //< Internal vertex array
        std::vector<unsigned int> indices; //< Internal index array

        template <typename dist_type = real_t, typename N>
        inline NanoRay ConstructRay(const N& origin, const N& direction, dist_type max_dist = std::numeric_limits<dist_type>::max()) {
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
        NanoRTRayTracer(const HF::Geometry::MeshInfo<float>& MI);
        NanoRTRayTracer(const HF::Geometry::MeshInfo<double>& MI);


        template<typename point_type, typename dist_type = real_t>
        inline HitStruct<real_t> Intersect(
            const point_type& origin,
            const point_type& dir,
            dist_type distance = -1.0,
            int mesh_id = -1) 
        {
            dist_type max_dist = (distance < 0) ? std::numeric_limits<dist_type>::max() : distance;

            NanoRay ray = ConstructRay<dist_type>(origin, dir, max_dist);
            Intersection hit = CreateHit();

            // Create a new intersector every time
            NanoRTRayTracer::Intersector temp_intersector(this->vertices.data(), this->indices.data(), sizeof(real_t)*3);
            
            bool did_intersect = bvh.Traverse<Intersector>(ray, temp_intersector, &hit);
            
            if (did_intersect)
                return HitStruct{ hit.t, 0 };
            else
                return HitStruct();

        }

        template<typename point_type>
        inline bool Occluded(
            const point_type& origin,
            const point_type& dir,
            float distance = -1,
            int mesh_id = -1)
        {
            return Intersect(origin, dir, distance, mesh_id).DidHit();
        }

        template<typename point_type>
        inline bool PointIntersection(
            point_type & origin,
            const point_type & dir,
            float distance = -1,
            int mesh_id = -1
        ) {
            auto res = Intersect(origin, dir, distance, mesh_id);

            // If it intersected, move the node and return true, otherwise do nothing and return false.
            if (res.DidHit()) {
                MovePoint(origin, dir, res.distance);
                return true;
            }
            else
                return false;
        }
    };
}
