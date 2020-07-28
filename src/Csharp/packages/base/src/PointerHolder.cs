using System;
using System.Runtime.InteropServices;
using System.Runtime;
using System.Security.Policy;

/*! 
    \brief A set of utilities and common functions that assist in communicating with native code.

    \remarks
    The aim of this namespace is to allow an end user to interact with objects in native memory
    without ever having to write code with the unsafe keyword, or manually manage the memory of
    an object. Interacting with these objects should be as intuitive as interacting with standard
    C# types, without the risk of an unexpected crash or memory leak. 

    \internal
        When extending this codebase, any functionality that has potential for reuse in accessing
        native code should be written here. If you are managing native memory, you sohuld be using
        or inheriting native object to manage its lifetime since the implementation of nativeobject
        gaurantees performance and safety.


        \warning
        When extending this namespace make sure to throughly test for memory leaks or crashes.

        \see NativeObject for more details on managing the lifetime of objects in native memory. 
    \endinternal
 */
namespace HumanFactors.NativeUtils
{
    /*! 
        \brief An object that holds a pointer to unmanaged memory or some other resource that must be freed.
        
        \details
        Safehandle, but is able to report size to the garbage collector to improve performance.
    
        \remarks
        All uses of pointers to native objects in this library will be wrapped by this, since it ensures that no memory
        is leaked under any circumstances (providing that the implementation of native functionality is correct).
    */
    public abstract class NativeObject : SafeHandle
    {
        /*!
            \brief the size of the object pointed to in unmanaged memory in bytes. Used to exert pressure on the GC. 
            \details While not mandatory, accurately setting this will improve the performance of the garbage collector.
        */
        public int pressure;

        /// \brief Pointer to the object in unmanaged memory.
        internal IntPtr Pointer { get => handle;}

        /*! 
            \brief Create a container to manage the lifetime of an object in native memory.
            
            \param in_native_pointer Pointer to the held object in unmanaged memory.
            \param in_pressure
            The size of the unmanaged object in bytes. This is used for tuning the garbage
            collector. If this is 0 or lower, the garbage collector will be unaffected.

        */    
    public NativeObject(IntPtr in_native_pointer, int in_pressure = 0) : base(new IntPtr(0), true)
        {
            this.pressure = in_pressure;
            this.handle = in_native_pointer;
            if (in_pressure > 0)
                GC.AddMemoryPressure(pressure);
        }

        /*! \brief There is no way to invalidate this class without destroying it, so will always return false */
        public override bool IsInvalid => false;


        /// \breif Destroy this object and free the memory of the native object it contains.
        ~NativeObject()
        {
            if (this.pressure > 0)
                GC.RemoveMemoryPressure(this.pressure);
        }
    }

    /*!
        \brief A read-only array stored in unmanaged memory.
        
        \tparam T The type of the array pointed to in unmanaged memory.

        \remarks
        This type is used throughout the codebase to access arrays in unmanaged memory. Note
        that the memory stored here is immutable. 

        \see array to access the data of this array in a readable format

        \internal
            \todo Implement a squarebrackets operator for this
        \endinternal

    */
    [CLSCompliant(true)]
    public abstract class NativeArray<T> : NativeObject where T :struct
    {

        /*! 
            \brief Calculate the size of our struct so we can properly set the pressure
            \returns The size of the struct in bytes.
        
            \details     
            The size of T is automatically calculated and multiplied by the
            number of elements to get the number of bytes occupied by this object in unmanaged
            memory. This ignores the inherent size of a C++ vector but is accurate enough to improve
            the performance of the garbage collector.

            \remarks
            Future improvements could include the size of a C++ vector in the calculation. 
        */
        static int CalcTSize(CVectorAndData ptrs, int num_elements_override = -1) {

            int num_elements = num_elements_override < 1 ? ptrs.size : num_elements_override;

            return System.Runtime.InteropServices.Marshal.SizeOf(typeof(T)) * num_elements_override;
        }

        /*! 
            \brief Read only access to this unmanaged array as a span
            \returns A <see cref="Span{T}"/> referencing the entirety of the unmanaged array.
            
            \remarks
            This only returns a reference to the unmanaged array, not a copy.

            \see CopyArray for a function that copies the contents of the array.
        */
        unsafe virtual public Span<T> array { get => new Span<T>(ptrs.data.ToPointer(), size); }


        /*!
            \brief Get a string representation of this array. 

            \todo 
            Optional toggle for truncation, and modifiable truncation threshold.
            
            \returns
            Return a string representation of all elements in this array, truncating it if necessary.

        */
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

                // Print dots if we hit this point
                if (i == truncate_after && truncate_after > 0)
                    out_str += ". . . ";
                else if (truncate_after < 0 || i < truncate_after || i > Math.Max(arr.Length - truncate_after, 3))
                    out_str += element.ToString() + ", ";
            }

            out_str = out_str.Remove(out_str.LastIndexOf(","));
            return out_str + "]";
        }

        /*!
            \brief Copy the unmanaged array pointed to by this object into a managed array.
            \returns A deep copy of the unmanaged array in managed memory.
            
            \remarks
            In most cases it is recommended to use the array property, as it saves
            time and memory by not copying the data into managed memory. If you must
            use this function to copy a large array, consider Freeing the native memory
            of this class by calling ReleaseHandle shortly after. 
        */
        public virtual T[] CopyArray()
        {
            T[] managed_array;
            HelperFunctions.MarshalUnmananagedArray2Struct<T>(this.ptrs.data, this.ptrs.size, out managed_array);
            return managed_array;
        }

        public bool Equals(NativeArray<T> arr2)
        {
            var this_span = this.array;
            var that_span = arr2.array;

            // Check that their lengths are equal
            if (this_span.Length != that_span.Length) return false;

            // check that each member is equal
            for (int i = 0; i < this_span.Length; i++)
                if (!this_span[i].Equals(that_span[i])) return false;

            return true;
        }
        /*! 
            \brief Data and vector pointers. Used for the construction of new spans and the freeing of
            resources when this object is finalized.
        */
        internal CVectorAndData ptrs;

        /// \brief The number of elements in the array.
        public virtual int size { get => ptrs.size; }

        /// <summary> Construct a new native array to wrap an unmanaged array in C++ </summary>
        /// <param name="ptrs"> Information about the vector to wrap </param>
        /// <param name="size">
        /// The size in bytes of this array in unmanaged memory. If set to -1, the size will
        /// automatically be calculated.
        /// </param>
        unsafe internal NativeArray(CVectorAndData ptrs, int size = -1) : base(ptrs.vector, CalcTSize(ptrs, size)) {
            this.ptrs = ptrs;
        }
    }

    /// \brief A wrapper for a two dimensional array in unmanaged memory
    /// \tparam T The type of object pointed to in unmanaged memory.
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
        unsafe internal NativeArray2D(CVectorAndData ptrs) : base(ptrs, ptrs.size * ptrs.size2) { }
    }

}