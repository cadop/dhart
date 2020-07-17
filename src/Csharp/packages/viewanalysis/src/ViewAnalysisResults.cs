using HumanFactors.Exceptions;
using HumanFactors.NativeUtils;
using HumanFactors.NativeUtils.CommonNativeArrays;
using HumanFactors.RayTracing;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Dynamic;
using System.Runtime.InteropServices;

namespace HumanFactors.ViewAnalysis
{
    /*! \brief The results of a ray intersection. */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct ViewAnalysisResult
    {
        /// \brief The distance from the origin to the hitpoint. Equal to -1 if the ray didn't hit
        float distance;
        /// \brief The ID of the intersected mesh. -1 if the ray didn't intersect any geometry.
        int meshid;
    }
    
    /*! 
         \brief  An array of ViewAnalysisResult from ViewAnalysis.ViewAnalysisStandard
   */
    public class ResultArray : NativeArray2D<ViewAnalysisResult>
    {
        /// <summary> Initializes a new instance of the <see cref="ResultArray" /> class. </summary>
        /// <param name="ptrs"> Must have size2 set to a value greater than one. </param>
        internal ResultArray(CVectorAndData ptrs) : base(ptrs) { }

        /*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
		*/
        protected override bool ReleaseHandle() => HF_STATUS.OK == NativeMethods.C_DeleteResultVector(handle);
    }

    /*! 
        \brief An array of float from ViewAnalysis.SphericallyDistributeRays

        \details
        Every 3 floats represent a direction.
    */
    public class DirectionArray : NativeArray2D<float>
    {
        /// <summary> Initializes a new instance of the <see cref="DirectionArray" /> class. </summary>
        /// <param name="ptr"> Info needed to create the array. </param>
        internal DirectionArray(CVectorAndData ptr) : base(ptr) { }
        /*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
        protected override bool ReleaseHandle()
        {
            HumanFactors.NativeUtils.CommonNativeArrays.NativeMethods.DestroyFloatVector(handle);
            return true;
        }
    }
}