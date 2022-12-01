using DHARTAPI.NativeUtils.CommonNativeArrays;
using System;
using System.Collections.Generic;
using System.Diagnostics;

using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

[assembly: CLSCompliant(true)]

namespace DHARTAPI.NativeUtils
{
	
	/*! 
		\brief Pointers to a C++ vector's data, the vector itself, and the size of the vector
		
		\details
		Carries all data necessary to create an instance of NativeArray{T}.

		\note
		usage of the name "vector" here corresponds to the C++ Standard Library's `std::vector`
		object, not the 3D vector described in Vector3D. 
	*/
	internal struct CVectorAndData
	{
		/*! \brief Pointer to vector's data in C++. This is what .array maps the span to.*/
		public IntPtr data;

		/*! \brief Pointer to the vector itself in unmanaged memory Used for deletion. */
		public IntPtr vector;

		/*! \brief The number of elements held in unmanaged memory */
		public int size;

		/*! \brief The size of the second dimension of this vector (if any). */
		public int size2;

		/*! 
			\brief Construct a new instance of CVectorAndData.

			\param in_data Pointer to the vector's data.
			\param in_vector Pointer to the vector itself.
			\param size The number of elements within this vector.
			\param size2
			The number of elements held in this vector's second dimension. If set to -1, the array
			is considered 1 dimensional.
		*/
		public CVectorAndData(IntPtr in_data, IntPtr in_vector, int size = -1, int size2 = -1)
		{
			data = in_data;
			vector = in_vector;
			this.size = size;
			this.size2 = size2;
		}

		/*!
			\brief Determine whether or not this CVectorAndData points to a actual vector.
		
			\details
			A vector is considered invalid if any of its pointers are null, or if its size is less than or equal to zero. 
		
			\returns 
			true if size > 0 and both the data and vector pointers are not equal to IntPtr.Zero otherwise, false
		*/
		public bool IsValid() => (size > 0 && data != IntPtr.Zero && vector != IntPtr.Zero);
	}

	/*! 
        \brief Holds global information relevant to interop.
    */
	internal static class NativeConstants
	{
		/*! \brief Relative path to the DHART_API DLL. 
            
            \remarks
            This is used in the DLL imports section of every function that calls into native code in this library.
        */
		public const string DLLPath = "DHARTAPI.dll";
	}

	/*!
        \brief Contains some functions useful for converting data for interop.
        
        \remarks
        These functions are frequently called internally in multiple DHARTAPI
        namespaces. 
    */
	public sealed class HelperFunctions
	{
		/*! 
            \brief Copy an array of structs from unmanaged memory into managed memory </summary>
            
            \tparam T The type of struct to copy.
            \param unmanagedArray Pointer to the array in unmanaged memory. 
            \param length Number of elements in the unmanaged array.
            \param mangagedArray An output parameter for the managed array to write to.

            \pre `length` is equal to the length of `unmanagedArray` in native memory
            \post `managedArray` is filled with copies of the elements from `unmanagedArray`
       */
		internal static void MarshalUnmananagedArray2Struct<T>(IntPtr unmanagedArray, int length, out T[] mangagedArray) where T : struct
		{
			// Calculate the size T in bytes
			var size = Marshal.SizeOf(typeof(T));

			// Construct a new managed array of T that can fit all elements of UnmanagedArray
			mangagedArray = new T[length];

			// Iterate through every element in UnmanagedArray
			for (int i = 0; i < length; i++)
			{
				// Calculate the position of the element in unnmanagedArray at index i
				IntPtr ins = new IntPtr(unmanagedArray.ToInt64() + i * size);

				// Marshal this pointer to a struct, then insert it into the same index in
				// the managed array
				mangagedArray[i] = (T)Marshal.PtrToStructure(ins, typeof(T));
			}
		}

		/*! 
            \brief Convert a  vector array into an array of floats. 
            
            \param vectors The array of vectors to convert
            
            \returns An array of floats 3x as big as the array of vectors.
            
            \internal
                \remarks Useful for preparing for Pinvoke
            \endinternal
        */
		public static float[] FlattenVectorArray(IEnumerable<Vector3D> vectors)
		{
			// Create a new output array of ffloats 3x the size of vectors.
			float[] out_array = new float[vectors.Count() * 3];
			int i = 0;

			// Iterate through every vector in the input vectors
			foreach (var vector in vectors)
			{
				// Calculate the index of the first element in the
				// output array
				int os = i * 3;

				// fill the next 3 elements with the x,y and z parameters of this vector
				out_array[os] = vector.x;
				out_array[os + 1] = vector.y;
				out_array[os + 2] = vector.z;

				// Increment loop variable
				i++;
			}

			// Return result
			return out_array;
		}

        /*! 
            \brief Convert a flat array of floats into a vector of points where result_array is true
            
            \param float_array
            The array of floats. Every 3 values should correspond to a single Vector3D
            \param result_array
            Array of booleans where if false the Vector3D at this index will be set to null. 
            Otherwise, it will be converted from the float array.
           
            \returns An array of Vector3D where every 3 floats in float_array is a single vector3D.
            
			\pre The length of `result_array` is equal to 1/3 the length of `float_array`

            \remarks
            This function is used frequently internally to handle results of ray intersections.

            \throws ArgumentException
            The number of elements in result_array is not equal to the number of
            elements in float_array / 3
       */

        public static float[] FlattenVectorArrayUnsafe(Vector3D[] vectors)
        {
            // Create a new output array of floats 3x the size of vectors.
            float[] out_array = new float[vectors.Length * 3];

            // Use Unsafe.Add to iterate over the array 
            ref var s0 = ref vectors[0];
            for (int i = 0, j = 0; i < vectors.Length; i++, j++)
            {
                out_array[j] = Unsafe.Add(ref s0, i).x;
                out_array[j++] = Unsafe.Add(ref s0, i).y;
                out_array[j++] = Unsafe.Add(ref s0, i).z;
            }

            return out_array;
        }

        public static Vector3D[] FloatArrayToVectorArray(float[] float_array, bool[] result_array)
		{
			// If our precondition isn't met, throw
			int size = float_array.Length / 3;
			if (size != result_array.Length)
				throw new ArgumentException("Size of float_array was not equal to the size of the result array!");

			Vector3D[] out_points = new Vector3D[size];

			// Iterate through every result
			for (int i = 0; i < size; i++)
			{

				// Calculate the index of this point in float_array
				int os = i * 3;

				// If the result array indicated an intersection here, insert a new vector
				// at this point containing the x,y, and z coordinates of the new point
				if (result_array[i])
				{
					float x = float_array[os];
					float y = float_array[os + 1];
					float z = float_array[os + 2];
					out_points[i] = new Vector3D(x, y, z);
				}
				else // Return an invalid point to signify that no geometry was intersected
					out_points[i] = new Vector3D(Single.NaN, Single.NaN, Single.NaN);
			}
			return out_points;
		}
	}
}