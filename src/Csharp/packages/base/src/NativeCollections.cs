using HumanFactors.Exceptions;
using System;
using System.Collections.Generic;
using System.Diagnostics;

using System.Diagnostics;

using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

/*! \brief A set of native array implementations that are useful throughout the entire program. */
namespace HumanFactors.NativeUtils.CommonNativeArrays
{
    /*! \brief An array of floats in unmanaged memory. */
    public class ManagedFloatArray : NativeUtils.NativeArray<float> {
        internal ManagedFloatArray(CVectorAndData ptrs) : base(ptrs) { }
        protected override bool ReleaseHandle() => (HF_STATUS.OK == NativeMethods.DestroyFloatVector(handle));
    }


    /*! \brief Native methods for interacting with the common arrays. */
    internal static class NativeMethods{
        private const string dll_path = NativeConstants.DLLPath;

        /// <summary>Destroys float vector pointed to by <paramref name="ptr"/>.</summary>
        /// <param name="ptr">The pointer to delete.</param>
        /// <returns>HF_STATUS.OK if successful</returns>
        [DllImport(dll_path)]
        internal static extern HF_STATUS DestroyFloatVector(IntPtr ptr);
    }
}