using HumanFactors.Geometry;
using System;
using System.Numerics;
using System.Runtime.Remoting.Messaging;

namespace HumanFactors.RayTracing
{
    /*!
        \brief A Bounding Volume Hierarchy for the \link EmbreeRaytracer \endlink.
            
        \details
        Upon calling the constructor for this class, a Bounding Volume Hierarchy is generated 
        for the \link RayTracing.EmbreeRaytracer \endlink. Without this structure, the raytracer cannot be used.

        \remarks
        A more detailed description of Embree's BVH is available in intel's paper:
        Embree: A Kernel Framework for Efficient CPU Ray Tracing
        http://cseweb.ucsd.edu/~ravir/274/15/papers/a143-wald.pdf.

        \internal
            \note
            In C++ this is is an EmbreeRayTracer.

            \todo
            Functions to add meshes to an existing BVH
        \endinternal

        \see EmbreeRaytracer for functions to use this BVH for ray intersections. 
    */
    public class EmbreeBVH : HumanFactors.NativeUtils.NativeObject
    {
        /*!
            \brief Generate a BVH for an instance of MeshInfo. 
            
            \param MI Mesh to generate the BVH from. 

            \exception Exception Embree failed to create a BVH for the given mesh.

            \internal
                \todo Add missing dependency exception

                \par What this object represents in C++
                In C++ the Raytracer is the object that handles the construction
                and destruction of Embree's BVH. This abstraction was made for
                the sake of clarity for the interfaces, and should not have much
                of an impact on the behavior of this class, but this information
                may be useful for debugging in C++ or NativeMethods.
                
            \endinternal

            \par Example
            \snippet raytracer\test_raytracer.cs EX_BVH_CSTOR
       */
        public EmbreeBVH(MeshInfo MI) : base(NativeMethods.C_ConstructRaytracer(MI.DangerousGetHandle()), MI.pressure) { }

        /*! \brief Gets the pointer of every meshinfo in an array of meshinfo. 
           
            \param MI Input Meshinfo to get the pointers for
            
            \returns An array of pointers for every instance of meshinfo in MI.

            \remarks This is required for the constructor in which multiple meshes are taken in by the BVH. 
        */
        private static IntPtr[] getMeshInfoPtrs(MeshInfo[] MI)
        {
            int num_ptrs = MI.Length;
            IntPtr[] ptrs = new IntPtr[num_ptrs];

            for (int i = 0; i < num_ptrs; i++)
                ptrs[i] = MI[i].Pointer;

            return ptrs;
        }

        /*! \brief Construct a BVH from an array of meshes
          
            \params MI Meshes to build the BVH from. 
        */
        public EmbreeBVH(MeshInfo[] MI) : base(NativeMethods.C_ConstructRaytracer(getMeshInfoPtrs(MI)), -1) {
            int total_pressure = this.pressure;

            foreach (var mesh in MI)
                total_pressure += mesh.pressure;

            this.UpdatePressure(total_pressure);
        }


        /*! \brief Add a new mesh to the BVH.
                
            \param MI Meshinfo to add to this bvh. The ID of this MeshInfo
                      will be updated if that ID is already occupied by another
                      mesh in the BVH. 

            \internal
				\details 
					This will icnrease the BVH's pressure on the garbage collector
					equal to the combined pressure of all meshes in MI. 

            \endinternal

            \par Example
            Demonstrating that the BVH changes after the addition of new meshes.

            \snippet raytracer\test_raytracer.cs EX_AddMesh
            ```
             --- Just Plane---
			(0, 0, -1)Did not Intersect
			(0, -1, 0)Intersected
			--- After Addition---
			(0, 0, -1)Intersected
			(0, -1, 0)Intersected
            ```
        */
        public void AddMesh(MeshInfo MI)
        {
            // Add the mesh to the BVH
            NativeMethods.C_AddMesh(this.Pointer, new IntPtr[] { MI.Pointer });

            // Force this mesh to udpate it's id incase it changed
            // when inserted into the BVH
            MI.UpdateIDNameAndArrays();
           
            // Increase the pressure we're exerting on the GC
            int new_pressure = this.pressure + MI.pressure;
            this.UpdatePressure(new_pressure);

        }

        /*! \brief Multiple new meshes to the BVH.

		  \param MI Meshinfo to add to this bvh. The ID of this MeshInfo
					will be updated if that ID is already occupied by another
					mesh in the BVH. 

		  \internal
			  \details 
				  This will icnrease the BVH's pressure on the garbage collector
				  equal to the combined pressure of all meshes in MI. 

		  \endinternal

		  \par Example
		  Demonstrating that the BVH changes after the addition of new meshes.

		  \snippet raytracer\test_raytracer.cs EX_AddMesh
		  ```
		   --- Just Plane---
		  (0, 0, -1)Did not Intersect
		  (0, -1, 0)Intersected
		  --- After Addition---
		  (0, 0, -1)Intersected
		  (0, -1, 0)Intersected
		  ```
	  */
        public void AddMesh(MeshInfo[] MI)
        {
            // Add the mesh to the bvh
            NativeMethods.C_AddMesh(this.Pointer, getMeshInfoPtrs(MI));

            // Update the pressure we're exerting on the GC and 
            // force these meshes to get their IDs back from the BVH
            int total_pressure = this.pressure;
            foreach (var mesh in MI)
            {
                mesh.UpdateIDNameAndArrays();
                total_pressure += mesh.pressure;
            }
            this.UpdatePressure(total_pressure);
        }

        /*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
		*/
        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyRayTracer(this.handle);
            return true;
        }
    }
}