using System.IO;
using System.Runtime.Hosting;

/*! 
    \brief Custom exceptions thrown by DHARTAPI Internally
    
    \remarks
    Exceptions are created here as needed, and generally existing C# excepions 
    are preferred.
*/
namespace DHARTAPI.Exceptions
{
	/*! 
     \brief Common error codes to convey the state of the program.
     \remarks Used as the return type for most native functions.
    */
	internal enum HF_STATUS
	{
		/// <summary>
		/// The equested operation executed successfully.
		/// </summary>
		OK = 1,

		// Special Codes //
		/// <summary>
		/// This functionality has not yet been implemented. This should not be seen at all in release.
		/// </summary>
		NOT_IMPLEMENTED = -54,

		// Error Codes //
		/// <summary>
		/// A generic error code often used as a catch all. Instances of this code being returned
		/// should be made more specific using another error code.
		/// </summary>
		GENERIC_ERROR = 0,

		/// <summary>
		/// The file at the given path was not found.
		/// </summary>
		NOT_FOUND = -1,

		/// <summary>
		/// The provided mesh was invalid.
		/// </summary>
		INVALID_MESH = -2,

		/// <summary>
		/// No graph was produced by the given input.
		/// </summary>
		NO_GRAPH = -3,

		/// <summary>
		/// Unused.
		/// </summary>
		INVALID_COST = -4,

		/// <summary>
		/// An external dependency required for this function to execute was not found.
		/// </summary>
		MISSING_DEPEND = -5,

		/// <summary>
		/// Ran out of memory during execution, usually because the dataset was too big.
		/// </summary>
		OUT_OF_MEMORY = -6,

		/// <summary>
		/// Unused.
		/// </summary>
		MALFORMED_DB = -7,

		/// <summary>
		/// Unused.
		/// </summary>
		DB_BUSY = -8,

		/// <summary>
		/// One or more of the given pointers was invalid.
		/// </summary>
		INVALID_PTR = -9,

		/// <summary>
		/// The requested element was out of the range of the container.
		/// </summary>
		OUT_OF_RANGE = -10,

		/// <summary>
		/// No path could be found between the start and endpoints given. Used exclusively for pathfinding.
		/// </summary>
		NO_PATH = -11,

		/*! \brief There is no cost with the given name associated with the graph */
		NO_COST = -12,

		/*! \brief The graph has not been compressed! */
		NOT_COMPRESSED = -13


	};

	/*! \brief The file passed was not an obj file, or was an invalid obj file */
	public class InvalidMeshException : System.ApplicationException
	{
		/// <summary>Initializes a new instance of the <see cref="InvalidMeshException" /> class.</summary>
		/// <param name="message">The message that describes the error.</param>
		public InvalidMeshException(string message) : base(message) { }
	}

	/*! \brief Tried to add an alternate cost to an edge without meeting the preconditions. */
	public class InvalidCostOperation : System.ApplicationException
	{
		public InvalidCostOperation(string message) : base(message) { }
	}

	/*! \brief There was some logical error with what the user was trying to perform. */
	public class LogicError : System.ApplicationException
	{
		public LogicError(string message) : base(message) { }
	}
}