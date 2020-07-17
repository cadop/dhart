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
        /*!
            \brief Check if this float is finite. 
            \param s The float to check.
            \returns true if this float is finite; otherwise, false.
            \details A float is finite if it is not equal to NAN, positive infinity, or negative infinity.
        */
        public static bool IsFinite(this Single s) 
            => !(Single.IsNaN(s) || Single.IsInfinity(s) || Single.IsNegativeInfinity(s));

    }

    /*! 
        \brief A three dimensional vector with built in utility functions. 
         
        \details 
        The Vector3D is the fundamental type of HumanFactors for handling position 
        and direction values. 

        \remarks
        If using another library with similar types such as unity, 
        consider making extension methods to easily convert them
        to Vector3D.
         
    */
    public struct Vector3D {
        public readonly float x, y, z; 

        /*!
            \brief Create a new instance of Vector3D
            \param X The x coordinate.
            \param Y The y coordinate.
            \param Z The z coordinate.
        */
        public Vector3D(float X, float Y, float Z)
        {
            x = X; y = Y; z = Z;
        }

        /*! 
            \brief Get a component of this vector as if it were an array of 3 floats
            \param i The index of the coordinate to get.
            \returns The x coordinate for 0, the y coordinate for 1, the z coordinate for 2.
            
            \exception System.ArgumentOutOfRangeException i is greater than 2 or less than 0.
        */
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

        /*!
            \brief  Convert this vector3D to an array of floats
            \returns A 3 element array containing X, Y, and Z
        */
        public float[] ToArray() => new float[3] { x, y, z, };
        
        /*!
            \brief Calculate the distance between this vector and another.
            \param V2 The vector to calculate distance to.
            \returns The distance between this vector and V2.    
        */
        public float DistanceTo(Vector3D V2)
        {
            return (float)Math.Sqrt(
                    Math.Pow(this.x - V2.x, 2)
                    + Math.Pow(this.y - V2.y, 2)
                    + Math.Pow(this.z - V2.z, 2)
            );
        }

        /*!
            \brief Check if this vector is valid.
            \returns false if the point contains any components that are not finite.
            \see Single.IsFinite() for more information on how this is calculated.        
        */    
        public bool IsValid() => x.IsFinite() && y.IsFinite() && z.IsFinite();
   };
}
