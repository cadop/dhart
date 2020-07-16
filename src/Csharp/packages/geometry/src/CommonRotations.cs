using System.Numerics;

namespace HumanFactors.Geometry
{
    /*! \brief A collection of rotations that are frequently useful in HumanFactors. */
    public static class CommonRotations
    {
        /*! \brief To rotate a mesh from Z-Up to Y-Up */
        public static Vector3D Zup_To_Yup = new Vector3D(-90, 0,  0);
        /*! 
            \brief Will rotate a mesh from Y-Up to Z-Up.
            \remarks 
            Useful for rotating Y-Up meshes so the GraphGenerator
            can run on them.
        */
        public static Vector3D Yup_To_Zup = new Vector3D(90, 0,  0);
    }
}