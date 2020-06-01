using System.Numerics;

namespace HumanFactors.Geometry
{
    /// <summary>
    /// Contains several constants for frequently used rotations.
    /// </summary>
    public static class CommonRotations
    {
        /// <summary>
        /// To convert a mesh from Z-Up to Y-Up
        /// </summary>
        public static Vector3D Zup_To_Yup = new Vector3D(-90, 0,  0);
        /// <summary>
        /// To convert a mesh from Y-Up to Z-Up.
        /// </summary>
        public static Vector3D Yup_To_Zup = new Vector3D(90, 0,  0);
    }
}