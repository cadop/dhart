using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using HumanFactors.RayTracing;
using HumanFactors.SpatialStructures;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Runtime.InteropServices;

namespace HumanFactors.ViewAnalysis
{
    /// <summary> A struct containing the results of a ray intersection. </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct ViewAnalysisResult
    {
        /// <summary>
        /// The distance from the origin to the hitpoint. Equal to -1 if the ray didn't hit
        /// </summary>
        float distance;
        /// <summary> The ID of the mesh hit. </summary>
        int meshid;
    }


    /// <summary>
    /// An array of results from <see cref="ViewAnalysis.ViewAnalysisStandard(EmbreeBVH,
    /// IEnumerable{Vector3D}, int, float, float, float)" />
    /// </summary>
    public class ResultArray : NativeArray2D<ViewAnalysisResult>
    {
        /// <summary> Initializes a new instance of the <see cref="ResultArray" /> class. </summary>
        /// <param name="ptrs"> Must have size2 set to a value greater than one. </param>
        internal ResultArray(CVectorAndData ptrs) : base(ptrs) { }
        protected override bool ReleaseHandle() => HF_STATUS.OK == NativeMethods.C_DeleteResultVector(handle);
    }

    /// <summary>
    /// An array of directions from <see cref="ViewAnalysis.SphericallyDistributeRays(int, float,
    /// float)" />
    /// </summary>
    public class DirectionArray : NativeArray2D<float>
    {
        /// <summary> Initializes a new instance of the <see cref="DirectionArray" /> class. </summary>
        /// <param name="ptr"> Info needed to create the array. </param>
        internal DirectionArray(CVectorAndData ptr) : base(ptr) { }

        protected override bool ReleaseHandle()
        {
            HumanFactors.NativeUtils.CommonNativeArrays.NativeMethods.DestroyFloatVector(handle);
            return true;
        }
    }
}