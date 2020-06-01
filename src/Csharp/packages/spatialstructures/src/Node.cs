using HumanFactors.NativeUtils;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.AccessControl;

namespace HumanFactors.SpatialStructures
{
    /// <summary>
    /// A point in space. Used internally by the graph generator.
    /// </summary>
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

    /// <summary>
    /// A list of nodes from a graph in unmanaged memory.
    /// </summary>
    public class NodeList : NativeArray<Node>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="NodeList"/> class.
        /// </summary>
        /// <param name="in_vector">The vector to create this nodelist from.</param>
        internal NodeList(CVectorAndData in_vector) : base(in_vector)
        {
        }
        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyNodeVector(handle);
            return true;
        }
    }
}