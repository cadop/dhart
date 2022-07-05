using DHARTAPI.Exceptions;
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
namespace DHARTAPI.NativeUtils.CommonNativeArrays
{
	/*! \brief An array of floats in unmanaged memory. */
	public class ManagedFloatArray : NativeUtils.NativeArray<float>
	{
		internal ManagedFloatArray(CVectorAndData ptrs) : base(ptrs) { }

		/*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
		protected override bool ReleaseHandle() => (HF_STATUS.OK == NativeMethods.DestroyFloatVector(handle));
	}

	/*!
        \brief  An array of pointers to char arrays in unmanaged memory.
    */
	public class ManagedCharPtrArray : NativeUtils.NativeArray<IntPtr>
	{

		/*! \brief Construct a new array of char arrays froma pointer of char arrays. */
		internal ManagedCharPtrArray(CVectorAndData ptrs) : base(ptrs) { }

		/*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
		protected override bool ReleaseHandle() => (HF_STATUS.OK == NativeMethods.DestroyScoreArray(handle));
	}

	/*! \brief A 2 dimensional array of integers in C++ */
	public class UnmanagedIntArray2D : NativeArray2D<int>
	{

		/*! \brief Construct a new UnmanagedIntArray using pointer to a C++ */
		internal UnmanagedIntArray2D(CVectorAndData ptrs) : base(ptrs) {	}

		/*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
		protected override bool ReleaseHandle()
		{
			NativeMethods.DestroyIntVector(this.Pointer);
			return true;
		}
	}
	/*! \brief A 2 dimensional array of integers in C++ */
	public class UnmanagedFloatArray2D : NativeArray2D<float>
	{

		/*! \brief Construct a new UnmanagedIntArray using pointer to a C++ */
		internal UnmanagedFloatArray2D(CVectorAndData ptrs) : base(ptrs) { }

		/*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
		protected override bool ReleaseHandle()
		{
			NativeMethods.DestroyFloatVector(this.Pointer);
			return true;
		}
	}



	/*! \brief Native methods for interacting with the objects in CommonNativeArrays. */
	internal static class NativeMethods
	{
		private const string dll_path = NativeConstants.DLLPath;

		[DllImport(dll_path)]
		internal static extern HF_STATUS DestroyFloatVector(IntPtr ptr);

		[DllImport(dll_path)]
		internal static extern HF_STATUS DestroyScoreArray(IntPtr ptr);

		[DllImport(dll_path)]
		internal static extern HF_STATUS DestroyIntVector(IntPtr ptr);
	}
}