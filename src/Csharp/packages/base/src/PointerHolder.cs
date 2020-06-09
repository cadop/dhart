using System;
using System.Runtime.InteropServices;
using System.Runtime;
using System.Security.Policy;

/// <summary> A set of utilities and common functions that assist in connecting to native code </summary>
namespace HumanFactors.NativeUtils
{
    /// <summary>
    /// An object that holds a pointer to unmanaged memory or some other resource that must be freed.
    /// Safehandle, but is able to report size to the garbage collector to improve performance.
    /// </summary>
    /// <remarks>All uses of pointers in this library will be wrapped by this, since it ensures that no memory is leaked
    /// under any circumstances. </remarks>
    public abstract class NativeObject : SafeHandle
    {

        /// <summary>
        /// The size of the object pointed to in unmanaged memory in bytes. Used to exert pressure on the GC. 
        /// While not mandatory, accurately setting this will improve the performance of the garbage collector.
        /// </summary>
        public int pressure;

        /// <summary> Pointer to the object in unmanaged memory. </summary>
        public IntPtr Pointer { get => handle;}

        /// <summary>
        /// Create a new Native Object to manage the lifetime of the object pointed to by <paramref name="in_native_pointer" />.
        /// </summary>
        /// <param name="in_native_pointer"> Pointer to the held object in unmanaged memory.</param>
        /// <param name="in_pressure">
        /// The size of the unmanaged object in bytes. This is used for tuning the garbage
        /// collector. If this is 0 or lower, the garbage collector will be unaffected.
        /// </param>
        public NativeObject(IntPtr in_native_pointer, int in_pressure = 0) : base(new IntPtr(0), true)
        {
            this.pressure = in_pressure;
            this.handle = in_native_pointer;
            if (in_pressure > 0)
                GC.AddMemoryPressure(pressure);
        }

        /// <summary>
        /// There is no way to invalidate this class without destroying it, so will always return
        /// <c> false </c>
        /// </summary>
        public override bool IsInvalid => false;


        /// <summary>
        /// Finalizes an instance of the <see cref="NativeObject" /> by removing the pressure
        /// applied in the constructor.
        /// </summary>
        ~NativeObject()
        {
            if (this.pressure > 0)
                GC.RemoveMemoryPressure(this.pressure);
        }
    }

    /// <summary>
    /// A NativeObject that owns a readable array in unmanaged memory accessible through the .array member.
    /// </summary>
    /// <typeparam name="T"> The type of the array pointed to in unmanaged memory. </typeparam>
    [CLSCompliant(true)]
    public abstract class NativeArray<T> : NativeObject where T :struct
    {

        /// <summary> Calculate the size of our struct so we can properly set the pressure </summary>
        /// <returns> The size of the struct in bytes </returns>
        /// <remarks>
        /// The size of <typeparamref name="T" /> is automatically calculated and multiplied by the
        /// number of elements to get the number of bytes occupied by this object in unmanaged
        /// memory. This ignores the inherent size of a C++ vector but is accurate enough for the
        /// garbage collector.
        /// </remarks>
        static int CalcTSize(CVectorAndData ptrs, int num_elements_override = -1) {

            int num_elements = num_elements_override < 1 ? ptrs.size : num_elements_override;

            return System.Runtime.InteropServices.Marshal.SizeOf(typeof(T)) * num_elements_override;
        }

        /// <summary>
        /// Read only access to this unmanaged array as a <see cref="Span{T}"/>.
        /// </summary>
        /// <returns>A <see cref="Span{T}"/> referencing the entirety of the unmanaged array.</returns>
        /// <remarks>
        /// This only returns a reference to the unmanaged array, not a copy. If you want to copy
        /// the array, use the <see cref="CopyArray" /> method.
        /// </remarks>
        unsafe virtual public Span<T> array { get => new Span<T>(ptrs.data.ToPointer(), size); }

        unsafe public override string ToString()
        {
            var arr = this.array;
            
            const int threshold = 15;
            int n = array.Length;
            
            // Truncate if the size of the array is higher than the threshold
            int truncate_after = -1;
            if (n > threshold)
                truncate_after = 5;

            string out_str = "[";
            for(int i = 0; i < arr.Length; i++)
            {
                var element = arr[i];

                /// Print dots if we hit this point
                if (i == truncate_after && truncate_after > 0)
                    out_str += ". . . ";
                else if (truncate_after < 0 || i < truncate_after || i > Math.Max(arr.Length - truncate_after, 3))
                    out_str += element.ToString() + ", ";
            }

            out_str = out_str.Remove(out_str.LastIndexOf(","));
            return out_str + "]";
        }

        /// <summary> Marshal the unmanaged array pointed to by this object into a managed array.</summary>
        /// <returns>A deep copy of the unmanaged array in managed memory. </returns>
        /// <remarks>
        /// In most cases it is recommended to use the <see cref="array" /> property, as it saves
        /// time and memory by not copying the data into managed memory.
        /// </remarks>
        public virtual T[] CopyArray()
        {
            T[] managed_array;
            HelperFunctions.MarshalUnmananagedArray2Struct<T>(this.ptrs.data, this.ptrs.size, out managed_array);
            return managed_array;
        }

        /// <summary>
        /// Data and vector pointers. Used for the construction of new spans and the freeing of
        /// resources when this object is finalized.
        /// </summary>
        protected CVectorAndData ptrs;

        /// <summary> The number of elements in the array. </summary>
        public virtual int size { get => ptrs.size; }

        /// <summary> Construct a new native array to wrap an unmanaged array in C++ </summary>
        /// <param name="ptrs"> Information about the vector to wrap </param>
        /// <param name="size">
        /// The size in bytes of this array in unmanaged memory. If set to -1, the size will
        /// automatically be calculated.
        /// </param>
        unsafe public NativeArray(CVectorAndData ptrs, int size = -1) : base(ptrs.vector, CalcTSize(ptrs, size)) {
            this.ptrs = ptrs;
        }
    }

    /// <summary> A wrapper for a two dimensional array in unmanaged memory. </summary>
    /// <typeparam name="T"> The type of object pointed to in unmanaged memory. </typeparam>
    /// <seealso cref="HumanFactors.NativeUtils.NativeArray{T}" />
    public abstract class NativeArray2D<T> : NativeArray<T> where T : unmanaged
    {
        /// <summary> Get the entire array as a flattened one dimensional array. </summary>
        unsafe override public Span<T> array { get => new Span<T>(ptrs.data.ToPointer(), ptrs.size * ptrs.size2);}

        /// <summary> Get a reference to a row of this array. </summary>
        /// <value> The <see cref="Span{T}" />. </value>
        /// <param name="i"> The row to reference. </param>
        /// <returns>
        /// A new span from the beginning of row[ <paramref name="i" />] to the end of row[
        /// <paramref name="i" />.
        /// </returns>
        unsafe public Span<T> this[int i] => new Span<T>((T*)ptrs.data.ToPointer() + ptrs.size2 * i, ptrs.size2);

        /// <summary> Initializes a new instance of the <see cref="NativeArray2D{T}" /> class. </summary>
        /// <param name="ptrs"> Must have size2 set to a value greater than one. </param>
        unsafe public NativeArray2D(CVectorAndData ptrs) : base(ptrs, ptrs.size * ptrs.size2) { }
    }

}