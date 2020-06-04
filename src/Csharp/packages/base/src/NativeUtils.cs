using HumanFactors.NativeUtils.CommonNativeArrays;
using System;
using System.Collections.Generic;
using System.Diagnostics;

using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

[assembly: CLSCompliant(true)]

namespace HumanFactors.NativeUtils
{
    /// <summary> Pointers to a c++ vector's data, the vector itself, and the size of the vector </summary>
    /// <remarks>
    /// Carries all data necessary to create an instance of <see cref="NativeArray{T}" />. Note that
    /// usage of the name "vector" here corresponds to the C++ Standard Library's std::vector
    /// object, not the 3D vector described in <see cref="Vector3D" />
    /// </remarks>
    public struct CVectorAndData
    {
        /// <summary>
        /// Pointer to vector's data in C++. This is what .array maps the span to.
        /// </summary>
        public IntPtr data;
        /// <summary>
        /// Pointer to the vector itself in unmanaged memory Used for deletion.
        /// </summary>
        public IntPtr vector;

        /// <summary>
        /// The number of elements held in unmanaged memory
        /// </summary>
        public int size;

        /// <summary>
        /// The size of the second dimension of this vector (if any). 
        /// </summary>
        public int size2;

        /// <summary> Initializes a new instance of <see cref="CVectorAndData" />. </summary>
        /// <param name="in_data"> Pointer to the vector's data. </param>
        /// <param name="in_vector"> Pointer to the vector itself. </param>
        /// <param name="size"> The number of elements within this vector. </param>
        /// <param name="size2">
        /// The number of elements held in this vector's second dimension. If set to -1, the array
        /// is considered 1 dimensional.
        /// </param>
        public CVectorAndData(IntPtr in_data, IntPtr in_vector, int size = -1, int size2 = -1)
        {
            data = in_data;
            vector = in_vector;
            this.size = size;
            this.size2 = size2;
        }

        /// <summary>
        /// Returns true if this CVectorAndData was constructed with valid pointers, and holds atleast one element.
        /// </summary>
        /// <returns>
        ///   <c>true</c> if size > 0 and both the data and vector pointers are not equal to IntPtr.Zero otherwise, <c>false</c>.
        /// </returns>
        public bool IsValid() => (size > 0 && data != IntPtr.Zero && vector != IntPtr.Zero);
    }

    /// <summary>
    /// Holds global information relevant to interop.
    /// </summary>
    public static class NativeConstants
    {
        /// <summary>
        /// Relative path to the humanfactors DLL.
        /// </summary>
        public const string DLLPath = "HumanFactors.dll";
    }

    /// <summary>
    /// Contains some functions useful for interop.
    /// </summary>
    public static class HelperFunctions
    {
        /// <summary> Copy an array of structs from unmanaged memory into managed memory </summary>
        /// <typeparam name="T"> The type of struct to copy </typeparam>
        /// <param name="unmanagedArray"> Pointer to the array in unmanaged memory. </param>
        /// <param name="length"> Number of elements in the unmanaged array. </param>
        /// <param name="mangagedArray"> An output parameter for the managed array to write to. This will be resized.</param>
        /// <returns> The new array of structs in managed memory. </returns>
        public static void MarshalUnmananagedArray2Struct<T>(IntPtr unmanagedArray, int length, out T[] mangagedArray) where T : struct
        {
            var size = Marshal.SizeOf(typeof(T));
            mangagedArray = new T[length];

            for (int i = 0; i < length; i++)
            {
                IntPtr ins = new IntPtr(unmanagedArray.ToInt64() + i * size);
                //mangagedArray[i] = Marshal.PtrToStructure(ins, T);
                mangagedArray[i] = (T)Marshal.PtrToStructure(ins, typeof(T));
                // mangagedArray[i] = n;
            }
        }

        /// <summary>
        /// Flatten a given vector array into an array of floats. Useful for preparing for Pinvoke
        /// </summary>
        /// <param name="vectors">The array of vectors to flatten</param>
        /// <returns>An array of floats 3x as big as the array of vectors. </returns>
        public static float[] FlattenVectorArray(IEnumerable<Vector3D> vectors)
        {
            float[] out_array = new float[vectors.Count() * 3];

            int i = 0;
            foreach (var vector in vectors)
            {
                int os = i * 3;
                out_array[os] = vector.x;
                out_array[os + 1] = vector.y;
                out_array[os + 2] = vector.z;
                i++;
            }
            return out_array;
        }

        /// <summary>
        /// Convert a flat array of floats into a vector of points where <paramref
        /// name="float_array" /> is true
        /// </summary>
        /// <param name="float_array">
        /// The array of floats. Every 3 values should correspond to a single Vector3D
        /// </param>
        /// <param name="result_array">
        /// Length must be equal to the length of <paramref name="float_array" /> / 3. If the <c>
        /// false </c> then the Vector3D at this index will be set to null. Otherwise, it will be
        /// converted from the float array.
        /// </param>
        /// <returns> An array of Vector3D </returns>
        /// <exception cref="ArgumentException">
        /// The number of elements in <paramref name="result_array" /> is not equal to the number of
        /// elements in <paramref name="float_array" />/3
        /// </exception>
        public static Vector3D[] FloatArrayToVectorArray(float[] float_array, bool[] result_array)
        {
            int size = float_array.Length / 3;

            if (size != result_array.Length)
                throw new ArgumentException("Size of float_array was not equal to the size of the result array!");

            Vector3D[] out_points = new Vector3D[size];
            for (int i = 0; i < size; i++)
            {
                int os = i * 3;
                if (result_array[i])
                {
                    float x = float_array[os];
                    float y = float_array[os + 1];
                    float z = float_array[os + 2];
                    out_points[i] = new Vector3D(x, y, z);
                }
                else
                    out_points[i] = new Vector3D(Single.NaN, Single.NaN, Single.NaN);
            }
            return out_points;
        }
    }
}