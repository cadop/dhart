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
        \par Internal Remarks 
        When extending this codebase, any functionality that has potential for reuse in accessing
        native code should be written here. If you are managing native memory, you should be using
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
        An implementation of Safehandle, able to report it'ssize to the garbage collector to improve performance.
    
        \remarks
        All uses of pointers to native objects in this library will be wrapped by this, since it ensures that no memory
        is leaked under any circumstances (providing that the implementation of native functionality is correct).

        \internal
            \par Garbage Collection Info
            In the constructor of this object, pressure is added to the C#
            garbage collector equal to the size in bytes of the object in
            native memory this NativeObject points to. This allows it to 
            integrate with the C# garbage collector so it can tactically choose
            when to deallocate the memory of certain objects. With this 
            implementation, this library maintains the benefits of the C# 
            garbage collector. This follows a pattern specified by the official 
            C# documentation:
            https://docs.microsoft.com/en-us/dotnet/api/system.gc.addmemorypressure?view=netcore-3.1in
            More can be read about the garbage collector
            in microsoft's official documentation for it:
            https://docs.microsoft.com/en-us/dotnet/standard/garbage-collection/fundamentals
        \endinternal

    */
	public abstract class NativeObject : SafeHandle
	{
		/*!
            \brief the size of the object pointed to in unmanaged memory in bytes. Used to exert pressure on the GC. 
            \details While not mandatory, accurately setting this will improve the performance of the garbage collector.
        */
		public int pressure;

		/// \brief Pointer to the object in unmanaged memory.
		internal IntPtr Pointer { get => handle; }

		/*! 
            \brief Create a container to manage the lifetime of an object in native memory.
            
            \param in_native_pointer Pointer to the held object in unmanaged memory.
            
            \param in_pressure
            The size of the unmanaged object in bytes. This is used for tuning the garbage
            collector. If this is 0 or lower, the garbage collector will be unaffected.

            \note
            While it is allowed for pressure to not be specified, it is highly recommended to provide
            atleast an estimation of the size of the object. Multiple objects that don't assert the proper
            pressure can lead to garbage collector trying to deallocate multiple large objects at
            the same time, resulting in a delay in execution to clients. C# Works best when it knows
            how much memory each object holds so it can tactically delete things at the optimal time.

        */
		internal NativeObject(IntPtr in_native_pointer, int in_pressure = 0) : base(new IntPtr(0), true)
		{
			this.pressure = in_pressure;
			this.handle = in_native_pointer;

			// If pressure is specified, put pressure on the garbage collector
			if (in_pressure > 0)
				GC.AddMemoryPressure(pressure);
		}

		/*! \brief There is no way to invalidate this class without destroying it, so will always return false */
		public override bool IsInvalid => false;


		/// \brief Destroy this object and free the memory of the native object it contains.
		~NativeObject()
		{
			if (this.pressure > 0)
				GC.RemoveMemoryPressure(this.pressure);
		}
	}

	/*!
        \brief A read-only array stored in unmanaged memory.
        
        \tparam T The type of the array pointed to in unmanaged memory.

        \details
        This type supports many of the facilities of standard arrays, however, doesn't 
        actually hold any memory itself. Instead it simply owns a map to the array in
        native memory which can be accessed through spans.

        \internal
            \par Benefits of Using Vectors Instead of Raw Arrays
            A C++ vector is a tried and tested type, gauranteed to free all of the memory it
            contains once it is destroyed. By carrying a vector and pointer to its data,
            any functions that interact with this object in C++ don't also need a pointer
            to the size of this object, since it's all contained in the vector.  By using
            a vector instead of a raw array, we retain all of the conveinences built into
            a C++ vector, while providing the same level of functionality in C#. 
        
            \par Subclassing this class
            An important function of this type is to manage the lifetime of a C++ vector in
            native memory. Every subclass of this object needs to have a destructor that
            calls the destructor of the vector it contains in C++. Aside from that, there
            aren't any other unique attributes required to subclass this type.  Just ensure
            that your destructor is being called, and that it doesn't leak any memory. 

        \endinternal

        \note
        The memory stored here is immutable, since this only holds a view to an array in
        C++. You can use NativeArray.CopyArray() to create a modifiable copy of this array
        in managed code. 
        
        \see NativeArray.array to access the data of this array in a readable format

        \internal
            \todo Implement a squarebrackets operator for this
        \endinternal

    */
	[CLSCompliant(true)]
	public abstract class NativeArray<T> : NativeObject where T : struct
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
		static int CalcTSize(CVectorAndData ptrs, int num_elements_override = -1)
		{
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
			// Get a view of the array that this holds
			var arr = this.array;

			const int threshold = 15;
			int n = array.Length;

			// Truncate if the size of the array is higher than the threshold
			int truncate_after = -1;
			if (n > threshold)
				truncate_after = 5;

			// Iterate through all elements and add them to the output string
			string out_str = "[";
			for (int i = 0; i < arr.Length; i++)
			{
				var element = arr[i];

				// Print dots if we hit this point
				if (i == truncate_after && truncate_after > 0)
					out_str += ". . . ";
				else if (truncate_after < 0 || i < truncate_after || i > Math.Max(arr.Length - truncate_after, 3))
					out_str += element.ToString() + ", ";
			}

			// Remove the final comma
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

		/*! \brief Get the element at location i.
            
            \param i Index of the element to get

            \returns The element at index i of the array.

            \throws ArgumentOutOfRangeException the key was beyond the bounds of this array. 
        */
		public unsafe T this[int key]
		{
			get
			{
				// Perform bounds checking so we don't crash
				if (key < 0 || key >= this.size)
					throw new ArgumentOutOfRangeException("Key " + key + " was out of range of the array's bounds!");
				else
					return this.array[key]; // there may be a better way to do this, but can't use a generic
											// type with pointers.
			}
		}

		/*!
            \brief Check equality of this array and another
            
            \param arr2 Array to check equality against

            \returns true if this array is equal to `arr2`

            \details 
            Two arrays are considered equal if they contain the same elements in the same positions, and are
            of the same length. 
        */
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

		/*!\brief The number of elements in the array. */
		public virtual int size { get => ptrs.size; }

		/*! 
            \brief Construct a new native array to wrap an unmanaged array in C++.
          
            \param ptrs Information about the vector to wrap
            \param size
            The size in bytes of this array in unmanaged memory. If set to -1, the size will
            automatically be calculated.
            
        */
		unsafe internal NativeArray(CVectorAndData ptrs, int size = -1) : base(ptrs.vector, CalcTSize(ptrs, size))
		{
			this.ptrs = ptrs;
		}
	}

	/*! 
        \brief A wrapper for a two dimensional array in unmanaged memory

        \tparam T The type of object pointed to in unmanaged memory.
    */
	public abstract class NativeArray2D<T> : NativeArray<T> where T : unmanaged
	{
		/*! 
            \brief Get a view of the entire array as a flattened one dimensional array.
            
            \return A span mapped to the unmanaged array pointed to by this object.
         
        */
		unsafe override public Span<T> array { get => new Span<T>(ptrs.data.ToPointer(), ptrs.size * ptrs.size2); }

		/*!
            \brief Get a view to a to a row of this array.
            \param i The row to reference.
            \returns A new span from the beginning of row `i` to the end of row `i`
        */
		unsafe public Span<T> this[int i] => new Span<T>((T*)ptrs.data.ToPointer() + ptrs.size2 * i, ptrs.size2);

		/*! \brief Construct a new NativeArray 2D with a pointer to a 2dimensional array and it's dimensions.
            
            \param ptrs 

            \pre `ptrs` Must have size2 set to a value greater than one. 
        */
		unsafe internal NativeArray2D(CVectorAndData ptrs) : base(ptrs, ptrs.size * ptrs.size2) { }
	}

    /*! 
		\brief A native array 2D that doesn't require destruction.

		\remarks
		Use this for arrays that don't need to be destructed, but instead are destroyed by some other means.
		An example of this is the vertex and index arrays for an instance of MeshInfo, in which both are destroyed
		when the meshinfo is destroyed. 
    */
    public class DependentNativeArray<T> : NativeArray2D<T> where T : unmanaged
    {

        internal DependentNativeArray(IntPtr data, int length, int width)
            : base(new CVectorAndData(data, IntPtr.Zero, length, width)) { }


        /*!\brief Doesn't do anything since it's managed by it's parent MeshInfo */
        protected override bool ReleaseHandle() => true;
    }

}