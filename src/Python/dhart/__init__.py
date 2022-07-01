import os
from os.path import dirname as up

__all__ = ['get_sample_model']

# This should be in a function
directory = os.path.join(os.path.dirname(os.path.realpath(__file__)) ,"bin" )
os.add_dll_directory(directory)

def get_data_dir():
    """Returns the data directory of the package.
    """
    
    return os.path.join(os.path.dirname(__file__),'Example Models')

def get_sample_model(name=""):
    """ Returns the absolute path to a given sample model in the package.

    Some available strings are "energy_blob_zup.obj", "plane.obj", "teapot.obj", "sponza.obj"

    Parameters
    ----------

    name : str
        Name of the sample model to load. 

    
    Returns
    -------

    model_path : str

    """

    model_dir = get_data_dir()
    model_path = os.path.join(model_dir, name)

    return model_path
