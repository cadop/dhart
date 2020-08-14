using HumanFactors.NativeUtils;
using System;

namespace HumanFactors.Pathfinding
{
	/*! 
        \brief A ID of a Node in a Path and the cost to it's next element.

        \note
        The ID held by a pathmember corresponds to the ID of a node in the graph from the path
        it's generated from. The node corresponding to this path member, stored
        in the graph's nodes array at the index of it's ID. I.E. if this 
        pathmember's ID is 1, then it points to the node at 
        Graph.getNodes()[1].
    */
	public struct PathMember
	{
		/*! \brief The cost to next node in the path. */
		public float cost_to_next;

		/*! \brief The ID of this node */
		public int id;

		/*! 
            \brief Compare the ID and Cost to next of this PathMember to another.
            
            \param PM2 Pathmember to compare to this one.

            \returns True if the ID and cost to next of PM2 are equal to that of this pathmember.
        */
		public bool Equals(PathMember PM2) => (this.id == PM2.id && this.cost_to_next == PM2.cost_to_next);

		/*! \brief Get a string representation of this pathmember's ID and cost to it's next element. */
		public override string ToString() => String.Format("({0}, {1:0.###})", id, cost_to_next);
	}

	/*! 
        \brief A collection of PathMembers representing a path from a start point to an end point.

        \see PathMember for information about the objects this array holds.
    */
	public class Path : NativeArray<PathMember>
	{
		/*! 
            \brief Initializes a new path from a pointer to a path in unmanaged memory.
            \param ptrs Pointers to a vector of paths and its data.
        */
		internal Path(CVectorAndData ptrs) : base(ptrs) { }

		/*!
			 \brief Free the native memory managed by this class. 
			 \note the garbage collector will handle this automatically
			 \warning Do not attempt to use this class after freeing it!
			 \returns True. This is guaranteed to execute properly.  
        */
		protected override bool ReleaseHandle()
		{
			NativeMethods.C_DestroyPath(handle);
			return true;
		}
	}
}