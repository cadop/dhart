using HumanFactors.Geometry;
using System;
using System.Numerics;
using System.Runtime.Remoting.Messaging;

namespace HumanFactors.RayTracing
{
    /// <summary>
    /// Contains A BVH for use with the <see cref="EmbreeRaytracer"/>.
    /// </summary>
    public class EmbreeBVH : HumanFactors.NativeUtils.NativeObject
    {
        /// <summary>
        /// Create a new BVH for the mesh located at <paramref name="MI"/>
        /// </summary>
        /// <param name="MI">Mesh to use for BVH construction</param>
        public EmbreeBVH(MeshInfo MI) : base(NativeMethods.C_ConstructRaytracer(MI.DangerousGetHandle()), MI.pressure) { }

        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyRayTracer(this.handle);
            return true;
        }
    }
}