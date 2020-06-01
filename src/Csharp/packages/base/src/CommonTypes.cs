using System;
using System.IO;
using System.Runtime;

/// <summary>
/// Automated analysis of the built environent
/// </summary>
namespace HumanFactors
{

    /// <summary>
    /// Some extension functions that are used internally.
    /// </summary>
    public static class Extensions
    {
        /// <summary>
        /// Determines whether this float is finite.
        /// </summary>
        /// <param name="s">The float to check.</param>
        /// <returns>
        ///   <c>true</c> if the specified float is finite; otherwise, <c>false</c>.
        /// </returns>
        /// <remarks> A float is finite if it is not equal to NAN, positive infinity, or negative infinity.</remarks>
        public static bool IsFinite(this Single s) 
            => !(Single.IsNaN(s) || Single.IsInfinity(s) || Single.IsNegativeInfinity(s));

    }

    /// <summary>
    /// A three dimensional vector.
    /// </summary>
    public struct Vector3D {
        public readonly float x, y, z;

        /// <summary>
        /// Initializes a new instance <see cref="Vector3D"/>.
        /// </summary>
        /// <param name="X">The x coordinate.</param>
        /// <param name="Y">The y coordinate.</param>
        /// <param name="Z">The z coordinate.</param>
        public Vector3D(float X, float Y, float Z)
        {
            x = X; y = Y; z = Z;
        }

        /// <summary>
        /// Get a component of this vector as if it were an array of 3 floats
        /// </summary>
        /// <value>
        /// The index of the coordinate to get.
        /// </value>
        /// <param name="i">Index of the coordinate</param>
        /// <returns> The x coordinate for 0, the y coordinate for 1, the z coordinate for 2 </returns>
        /// <exception cref="System.ArgumentOutOfRangeException"> the input is greater than 2 or less than 0 </exception>
        public float this[int i]
        {
            get
            {
                switch (i)
                {
                    case 0:
                        return x;
                    case 1:
                        return y;
                    case 2:
                        return z;
                    default:
                        throw new ArgumentOutOfRangeException();
                }
            }
        }




        /// <summary>
        /// Convert this vector3D to an array of floats
        /// </summary>
        /// <returns>A 3 element array containing X,Y, and Z</returns>
        public float[] ToArray() => new float[3] { x, y, z, };

        /// <summary>
        /// Calculate the distance between this vector and <paramref name="V2"/>
        /// </summary>
        /// <param name="V2">The vector to calculate distance to </param>
        /// <returns> The distance between this vector and <paramref name="V2"/> </returns>
        public float DistanceTo(Vector3D V2)
        {
            return (float)Math.Sqrt(
                    Math.Pow(this.x - V2.x, 2)
                    + Math.Pow(this.y - V2.y, 2)
                    + Math.Pow(this.z - V2.z, 2)
            );
        }

        /// <summary>
        /// Check if this point is valid.
        /// </summary>
        /// <returns><see langword="false"/> if the point contains NaN, </returns>
        /// <remarks> A point is considered valid if all of its coordinates are finite <see cref="HumanFactors.Extensions.IsFinite(float)"/> </remarks>
        public bool IsValid() => x.IsFinite() && y.IsFinite() && z.IsFinite();
   };
}
