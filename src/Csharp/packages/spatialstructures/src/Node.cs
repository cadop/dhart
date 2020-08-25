using HumanFactors.NativeUtils;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.AccessControl;

namespace HumanFactors.SpatialStructures
{
	/*!
        \brief A point in space.
        
        \remarks
        Used internally by the graph generator.
    */
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public struct Node
	{
		/// <summary>
		/// The X coordinate.
		/// </summary>
		public float x;

		/// <summary>
		/// The Y coordinate.
		/// </summary>
		public float y;

		/// <summary>
		/// The Z coordinate
		/// </summary>
		public float z;

		/// <summary>
		/// Unused.
		/// </summary>
		public short type;

		/// <summary>
		/// The ID of this node.
		/// </summary>
		public int id;

		/*! \brief Get a string representation of this node's coordinates.
            \returns A string containing this node's X,Y, and Z coordinates. 
        */
		public override string ToString() => String.Format("({0}, {1}, {2})", x, y, z);

		/*! \brief Create a vector3D with this node's X,Y and Z coordinates. */
		public Vector3D ToVector3D() => new Vector3D(x, y, z);
	}

	/*! 
        \brief An array of Nodes directly from a graph in unmanaged memory.
    */
	public class NodeList : NativeArray<Node>
	{
		/*!
            \brief Initializes a new instance of the NodeList.
            \param in_vector The vector to create this nodelist from.
        */
		internal NodeList(CVectorAndData in_vector) : base(in_vector) { }
	

		/*! 
			\brief Convert all nodes in this array to Vector3D
			
			\returns The Nodes in this list as Vector3D
		*/
		public Vector3D[] ToVector3D()
		{
			Vector3D[] out_array = new Vector3D[this.size];

			var array = this.array;
			for (int i = 0; i < this.size; i++)
				out_array[i] = new Vector3D(array[i].x, array[i].y, array[i].z);

			return out_array;
		}
		
		/*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
		protected override bool ReleaseHandle()
		{
			NativeMethods.C_DestroyNodeVector(handle);
			return true;
		}
	}
}