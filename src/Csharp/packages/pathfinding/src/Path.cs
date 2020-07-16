using HumanFactors.NativeUtils;

namespace HumanFactors.Pathfinding
{
    /// <summary>
    /// A node in <see langword="abstract"/>path
    /// </summary>
    public struct PathMember
    {
        /// <summary>
        /// The cost to next node in the path
        /// </summary>
        public float cost_to_next;

        /// <summary>
        /// The ID of this node
        /// </summary>
        public int id;
    }

    /// <summary>
    /// A collection of node ids and costs representing a path. Holds <see cref="PathMember"/>s.
    /// </summary>
    public class Path : NativeArray<PathMember>
    {
        /// <summary>
        /// Initializes a new path from a pointer to a path in unmanaged memory.
        /// </summary>
        internal Path(CVectorAndData ptrs) : base(ptrs) { }

        /// <summary>
        /// Free the path in unmanaged memory. 
        /// </summary>
        protected override bool ReleaseHandle()
        {
            NativeMethods.C_DestroyPath(handle);
            return true;
        }
    }
}