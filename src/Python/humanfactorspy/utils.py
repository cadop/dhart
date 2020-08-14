
__all__ = ['is_point']

def is_point(potential_point) -> bool:
    """ Check if a certain type can be used as a x,y,z set of coordinates

    An object is considered a point, as long as it is able to return numbers
    for [0], [1], and [2]. If it doesnt have __get__ defined, or any of these
    values aren't numbers, then it isn't a point. 

    Args:
        potential_point : object to check if it is a point

    Returns:
        true if it can be used as a point, false otherwise

    Example:

       >>> # An item with no square brackets operator
       >>> is_point(1) 
       False
       
       >>> # An item that has a  Has square brackets operator, 
       >>> # but doesn't hold a numeric type
       >>> is_point(["string", "string", "string"]) 
       False

       >>> # An item that has a square brackets operator, 
       >>> # but holds lists
       >>> is_point([[1, 2, 3], [1, 2, 3], [1, 2, 3]])
       False

       >>> # Has square brackets operator but only holds 2 elements
       >>> is_point([1, 2])
       False

       >>> # The following statments should all be 
       >>> # considered points
       >>> is_point([1, 2, 3])
       True

       >>> is_point([1.0, 2.0, 2.5])
       True

       >>> is_point((1, 2, 3))
       True

       >>> is_point((1.0, 2.0, 3.0))
       True
    """
    # Works like a C++ concept. If it fails to execute this statment
    # due to not having square brackets, the catch statment will
    # catch the exception and return false. Also must have numeric
    # type
    try:
        return (
            isinstance(potential_point[0], (int, float, complex))
            and isinstance(potential_point[1], (int, float, complex))
            and isinstance(potential_point[2], (int, float, complex))
        )
    # These should be thrown if the object doesn't have a subscript operator
    # or does not have enough elements
    except (TypeError, IndexError):
        return False  
