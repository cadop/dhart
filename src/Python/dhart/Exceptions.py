from enum import IntEnum as Enum


class HF_STATUS(Enum):
    """ Error codes to be returned from C++ """

    OK = 1
    NOT_IMPLEMENTED = -54
    GENERIC_ERROR = 0  # Not sure what happened here (If this gets thrown, either fix it or give it a status code!)
    NOT_FOUND = -1  # A file does not exist at the given path
    INVALID_OBJ = -2  # The given path did not point to a valid obj file
    NO_GRAPH = -3  # This requires a valid graph in the DB to execute successfully
    INVALID_COST = -4  # The given cost name does not exist in the database
    MISSING_DEPEND = -5  # A dependency for this object is missing.
    OUT_OF_MEMORY = -6  # Ran out of memory during the last operation
    MALFORMED_DB = -7  # The database exists, but is in some kind of error state
    DB_BUSY = -8  # The database is busy (is there some external connection?)
    EMPTY_INPUT = -9  # Nothing was passed!
    OUT_OF_RANGE = (
        -10
    )  # The requested datapoint was out of the bounds of the given structure
    NO_PATH = -11  # No path was returned from the pathfinding ooperation
    NO_COST = -12  # The given cost could not be found.
    NOT_COMPRESSED = -13  # The graph needed to be compressed and it wasn't


class HFException(Exception):
    """ Generic DHARTAPIException """
    def __init__(self, message):
        super().__init__(message)

class InvalidOBJException(HFException):
    """ The provided An invalid OBJ file was specified.

    This can be raised if the given filepath leads to a file that is not an
    OBJ file, the file is a malformed obj file.
    """
    def __init__(self, message = ""):
        super().__init__(message)


class FileNotFoundException(HFException):
    """ The file at the given path given file was not found """
    def __init__(self, message = ""):
        super().__init__(message)


class OutOfRangeException(HFException):
    """ The requested datapoint was out of the bounds of the given structure """
    def __init__(self, message = ""):
        super().__init__(message)


class MissingDependencyException(HFException):
    """ One or more of the required dependencies for this operation were unable to be found """
    def __init__(self, message = ""):
        super().__init__(message)

class LogicError(HFException):
    """ A class was used improperly. """
    def __init__(self, message = ""):
        super().__init__(message)

class InvalidCostOperation(HFException):
    """ Tried to add an edge to a cost that didn't already exist in
        the default cost set 
    """
    def __init__(self, message = ""):
        super().__init__(message)