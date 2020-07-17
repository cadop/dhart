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
    }

    /*! 
        \brief An array of Nodes directly from a graph in unmanaged memory.
    */
    public class NodeList : NativeArray<Node>
    {
        /*!
            \brief Initializes a new instance of the <see cref="NodeList"/>
        */
        /// <param name="in_vector">The vector to create this nodelist from.</param>
        internal NodeList(CVectorAndData in_vector) : base(in_vector)
        {
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